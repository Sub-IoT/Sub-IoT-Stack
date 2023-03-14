/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string.h>

#include "hwuart.h"
#include "hwdma.h"
#include "framework_defs.h"
#include "platform_defs.h"
#include "fifo.h"
#include "scheduler.h"
#include "serial_protocol.h"
#include "modules_defs.h"
#include "hal_defs.h"
#include "debug.h"
#include "errors.h"
#include "timer.h"

#include "hwsystem.h"
#include "hwatomic.h"
#include "crc.h"

#include "ng.h"
#include "crc.h"

#include "log.h"


#define container_of(ptr, type, member) ({                      \
        _Pragma("GCC diagnostic push");                         \
        _Pragma("GCC diagnostic ignored \"-Wcast-align\"")      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );     \
        _Pragma("GCC diagnostic pop")                           \
})

#define RX_BUFFER_SIZE 256

#define TX_FIFO_FLUSH_CHUNK_SIZE 10 // at a baudrate of 115200 this ensures completion within 1 ms
                                    // TODO baudrate dependent


#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
#ifndef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
#error FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES should be defined if FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA is used.
#endif
#endif

#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
#define SERIAL_PROTOCOL_TIMEOUT            (15 * TIMER_TICKS_PER_SEC) // Same as PING_TIMEOUT of serial protocol monitor, longer than the boot time of the other party
#define SERIAL_PROTOCOL_TIMEOUT_BLOCK_TIME (5 * TIMER_TICKS_PER_MINUTE)

#endif

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_SERIAL_PROTOCOL_LOG_ENABLED)
  #define DPRINT(...) log_print_string(__VA_ARGS__)
  #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINT_DATA(...)
#endif


#define SERIAL_FRAME_SYNC_BYTE 0xC0
#define SERIAL_FRAME_VERSION   0x00
#define SERIAL_FRAME_HEADER_SIZE 7
#define SERIAL_FRAME_SIZE 4
#define SERIAL_FRAME_COUNTER 2
#define SERIAL_FRAME_TYPE 3
#define SERIAL_FRAME_CRC1   5
#define SERIAL_FRAME_CRC2   6

#define SERIAL_PROTOCOL_TX_FIFO_SIZE 255


typedef enum {
  STATE_IDLE,
  STATE_REQ_START,
  STATE_REQ_WAIT,
  STATE_REQ_BUSY,
  STATE_RESP,
  STATE_RESP_PENDING_REQ,
  STATE_REC
} state_t;

typedef struct priv_serial_protocol_handle
{
  uint8_t tx_buffer[SERIAL_PROTOCOL_TX_FIFO_SIZE];
  uint8_t rx_buffer[RX_BUFFER_SIZE];
  uint8_t header[SERIAL_FRAME_HEADER_SIZE];
  fifo_t rx_fifo;
  fifo_t tx_fifo;
  uart_handle_t* uart;
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
  dma_handle_t* dma_rx;
  dma_handle_t* dma_tx;
#endif
  struct priv_serial_protocol_handle* next_serial_protocol;
  cmd_handler_t alp_handler;
  cmd_handler_t ping_request_handler;
  cmd_handler_t ping_response_handler;
  cmd_handler_t logging_handler;
  target_rebooted_callback_t target_rebooted_cb;
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
  timer_tick_t serial_protocol_timeout_start_time;
  state_t state;
  pin_id_t uart_state_pin;
  pin_id_t target_uart_state_pin;
#endif
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
  uint16_t tx_size;
  uint8_t dma_rx_idx;
  uint8_t dma_tx_idx;
#endif
  volatile uint8_t target_uart_state_isr_count;
  uint8_t packet_up_counter;
  uint8_t payload_len;
  uint8_t packet_down_counter;
  bool tx_request_pending:1;
  bool listen_uart_inited:1;
  bool parsed_header:1;
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
  bool use_interrupt_lines:1;
  bool serial_protocol_timeout_active:1;
#endif
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
  bool use_dma:1;
  bool tx_dma_busy:1;
#endif
} priv_serial_protocol_handle_t;

_Static_assert((sizeof(serial_protocol_handle_t) - sizeof(serial_protocol_driver_t*)) >= sizeof(priv_serial_protocol_handle_t), "allocated length for private buffer to small");

static priv_serial_protocol_handle_t* first_serial_protocol = NULL;


#define SWITCH_STATE(phandle, s) do { \
  phandle->state = s; \
  DPRINT("switch to %s\n", #s); \
} while(0)

static void process_rx_fifo(void *arg);
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
static void execute_state_machine(void *arg);
#endif

static error_t serial_protocol_transfer_bytes(serial_protocol_handle_t* handle, uint8_t* bytes, uint8_t length, serial_message_type_t type);
static error_t serial_protocol_transfer(serial_protocol_handle_t* handle, char* string);
static void serial_protocol_register_handler(serial_protocol_handle_t* handle, cmd_handler_t cmd_handler, serial_message_type_t type);
static void serial_protocol_unregister_handler(serial_protocol_handle_t* handle, serial_message_type_t type);
static void serial_protocol_set_target_rebooted_callback(serial_protocol_handle_t* handle, target_rebooted_callback_t cb);
static void serial_protocol_clear_handler(serial_protocol_handle_t* handle);

static serial_protocol_driver_t serial_protocol_driver =
{
  .serial_protocol_transfer_bytes = serial_protocol_transfer_bytes,
  .serial_protocol_transfer = serial_protocol_transfer,
  .serial_protocol_register_handler = serial_protocol_register_handler,
  .serial_protocol_unregister_handler = serial_protocol_unregister_handler,
  .serial_protocol_set_target_rebooted_callback = serial_protocol_set_target_rebooted_callback,
  .serial_protocol_clear_handler = serial_protocol_clear_handler,
};

static void init_priv_handle(priv_serial_protocol_handle_t* phandle)
{
  fifo_init(&phandle->rx_fifo, phandle->rx_buffer, sizeof(phandle->rx_buffer));
  fifo_init(&phandle->tx_fifo, phandle->tx_buffer, SERIAL_PROTOCOL_TX_FIFO_SIZE);
  phandle->tx_request_pending = false;
  phandle->next_serial_protocol = NULL;
  phandle->alp_handler = NULL;
  phandle->ping_response_handler = NULL;
  phandle->logging_handler = NULL;
  phandle->target_rebooted_cb = NULL;
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
  phandle->state = STATE_IDLE;
#endif
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
  phandle->tx_size = 0;
  phandle->tx_dma_busy = false;
#endif
  phandle->target_uart_state_isr_count = 0;
  phandle->packet_up_counter = 0;
  phandle->payload_len = 0;
  phandle->packet_down_counter = 0;
  phandle->tx_request_pending = false;
  phandle->listen_uart_inited = false;
  phandle->parsed_header= false;
}

static void add_serial_protocol(priv_serial_protocol_handle_t* new_phandle)
{
  priv_serial_protocol_handle_t* phandle;
  if(first_serial_protocol == NULL)
  {
    first_serial_protocol = new_phandle;
    return;
  }
  phandle = first_serial_protocol;
  while(phandle->next_serial_protocol != NULL)
  {
    phandle = phandle->next_serial_protocol;
  }
  phandle->next_serial_protocol = new_phandle;
}

static priv_serial_protocol_handle_t* get_serial_protocol(uart_handle_t* uart)
{
  priv_serial_protocol_handle_t* phandle = first_serial_protocol;
  while(phandle != NULL)
  {
    if(phandle->uart == uart)
    {
      return phandle;
    }
    phandle = phandle->next_serial_protocol;
  }
  return NULL;
}

/** @Brief Enable UART interface and UART interrupt
 *  @return void
 */
static void serial_protocol_enable(priv_serial_protocol_handle_t* phandle) 
{
  DPRINT("uart enabled @ %i",timer_get_counter_value());
  assert(uart_enable(phandle->uart));
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
  if(phandle->use_dma)
  {
    assert(dma_channel_enable(phandle->dma_rx));
    uart_start_read_bytes_via_DMA(phandle->uart, phandle->rx_buffer, RX_BUFFER_SIZE, phandle->dma_rx_idx);
    uart_tx_interrupt_enable(phandle->uart);
  }
  else
#endif
  {
    uart_rx_interrupt_enable(phandle->uart);
  }
  phandle->listen_uart_inited = true;
}

/** @Brief disables UART interface
 *  @return void
 */
static void serial_protocol_disable(priv_serial_protocol_handle_t* phandle) 
{
  phandle->listen_uart_inited = false;
  assert(uart_disable(phandle->uart));
  uart_pull_down_rx(phandle->uart);
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
  if(phandle->use_dma)
  {
    assert(dma_channel_disable(phandle->dma_rx));
    assert(dma_channel_disable(phandle->dma_tx));
    uart_tx_interrupt_disable(phandle->uart);
  }
  else
#endif
  {
    uart_rx_interrupt_disable(phandle->uart);
  }
  DPRINT("uart disabled @ %i",timer_get_counter_value());
}

/** @brief Lets receiver know that 
 *  all the data has been transfered
 *  @return void
 */
static void release_receiver(priv_serial_protocol_handle_t* phandle)
{
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
  if(phandle->use_interrupt_lines)
  {
    DPRINT("release receiver\n");
    serial_protocol_disable(phandle);
    hw_gpio_clr(phandle->uart_state_pin);
  }
#endif
}

/** @brief transmit data in fifo to UART
 *  @return void
 */
static void flush_serial_protocol_tx_fifo(void *arg) 
{
  assert(arg!=NULL);
  priv_serial_protocol_handle_t* phandle = (priv_serial_protocol_handle_t*)arg;
  uint8_t len = fifo_get_size(&phandle->tx_fifo);

#ifdef HAL_UART_USE_DMA_TX
  // when using DMA we transmit the whole FIFO at once
  uint8_t buffer[SERIAL_PROTOCOL_TX_FIFO_SIZE];
  fifo_pop(&serial_protocol_tx_fifo, buffer, len);
  uart_send_bytes(uart, buffer, len);
#else
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
  //Execute atomic, otherwise there is a chance that the DMA complete callback is called during execution of this code.
  // If that would happen a DMA transfer with length 0 is started which will not trigger a complete callback
  // This means that we have to fetch the data size inside the fifo inside the atomic part again
  if(phandle->use_dma)
  {
    start_atomic();
    len = fifo_get_size(&phandle->tx_fifo);
    if(len > 0)
    {
      //log_print_string("flush %d", tx_dma_busy);
      if(!phandle->tx_dma_busy)
      {
        assert(dma_channel_enable(phandle->dma_tx));
        dma_channel_interrupt_enable(phandle->dma_tx);
        uint8_t* buffer;
        fifo_get_continuos_raw_data(&phandle->tx_fifo, &buffer, &phandle->tx_size);
        uart_send_bytes_via_DMA(phandle->uart, buffer, phandle->tx_size, phandle->dma_tx_idx);
        phandle->tx_dma_busy = true;
      }
      //To keep awake
      sched_post_task_prio(&flush_serial_protocol_tx_fifo, MIN_PRIORITY, phandle);
    }
    else
    {
      phandle->tx_request_pending = false;
      dma_channel_interrupt_disable(phandle->dma_tx);
      release_receiver(phandle);
      sched_post_task_prio(&execute_state_machine, DEFAULT_PRIORITY, phandle);
    }
    end_atomic();
  }
  else
#endif // FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
  {
    // only send small chunks over uart each invocation, to make sure
    // we don't interfere with critical stack timings.
    // When there is still data left in the fifo this will be rescheduled
    // with lowest prio
    uint8_t chunk[TX_FIFO_FLUSH_CHUNK_SIZE];
    if(len <= TX_FIFO_FLUSH_CHUNK_SIZE)
    {
      fifo_pop(&phandle->tx_fifo, chunk, len);
      uart_send_bytes(phandle->uart, chunk, len);
      phandle->tx_request_pending = false;
      release_receiver(phandle);
  #ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
      if(phandle->use_interrupt_lines)
      {
        sched_post_task_prio(&execute_state_machine, DEFAULT_PRIORITY, phandle);
      }
  #endif
    } 
    else 
    {
      fifo_pop(&phandle->tx_fifo, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
      uart_send_bytes(phandle->uart, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
      sched_post_task_prio(&flush_serial_protocol_tx_fifo, MIN_PRIORITY, phandle);
    }
  }
#endif // HAL_UART_USE_DMA_TX
}

/** @Brief Keeps µC awake while receiving UART data
 *  @return void
 */
static void listen(void* arg)
{
  priv_serial_protocol_handle_t* phandle = (priv_serial_protocol_handle_t*)arg;
  if(!phandle->listen_uart_inited)
  {
    serial_protocol_enable(phandle);
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
    hw_gpio_set(phandle->uart_state_pin); //set interrupt gpio to indicate ready for data
#endif
  }
}

#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
static bool serial_protocol_timeout(priv_serial_protocol_handle_t* phandle)
{
  if(phandle->serial_protocol_timeout_active)
  {
    timer_tick_t timer_value = timer_get_counter_value();
    timer_tick_t diff = timer_calculate_difference(phandle->serial_protocol_timeout_start_time, timer_value);
    if(diff > SERIAL_PROTOCOL_TIMEOUT)
    {
        serial_protocol_handle_t* handle = container_of((uint32_t(*)[])(void*)phandle, serial_protocol_handle_t, priv_data);
#ifdef FRAMEWORK_DEBUG_ASSERT_REBOOT
      serial_protocol_clear_handler(handle);
      log_print_error_string("serial_protocol timeout occurred in state %d", phandle->state);
      return true;
#else
      log_print_string("serial_protocol timeout occurred in state %d", phandle->state);
      if(timer_value < SERIAL_PROTOCOL_TIMEOUT_BLOCK_TIME)
      {
        serial_protocol_clear_handler(handle);
        return true;
      }
      assert(false);
#endif
    }
  }
  else
  {
    phandle->serial_protocol_timeout_start_time = timer_get_counter_value();
    phandle->serial_protocol_timeout_active = true;
  }
  return false;
}

static inline void clear_serial_protocol_timeout(priv_serial_protocol_handle_t* phandle)
{
  phandle->serial_protocol_timeout_active = false;
}

/** @Brief Handles the different states of the interrupt line handshake mechanism
 *  @return void
 */

static void execute_state_machine(void* arg)
{
    assert(arg!=NULL);
    priv_serial_protocol_handle_t* phandle = (priv_serial_protocol_handle_t*) arg;
    switch (phandle->state) {
    case STATE_REC:
    {
      if(hw_gpio_get_in(phandle->target_uart_state_pin))
      {
        if(!serial_protocol_timeout(phandle))
        {
          sched_post_task_prio(&execute_state_machine, MIN_PRIORITY, phandle);
        }
        else
        {
          return;
        }
        break;
      }
      clear_serial_protocol_timeout(phandle);
      phandle->target_uart_state_isr_count = 0;
      SWITCH_STATE(phandle, STATE_RESP);
      __attribute__ ((fallthrough));
      //Intentional fall through
    }
    case STATE_RESP:
    {
      // response period completed, process the request
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
      if(phandle->use_dma)
      {
        size_t received_bytes = uart_stop_read_bytes_via_DMA(phandle->uart);
        fifo_init_filled(&phandle->rx_fifo, phandle->rx_buffer, received_bytes, RX_BUFFER_SIZE);
      }
#endif
      sched_post_task_prio(&process_rx_fifo, DEFAULT_PRIORITY, phandle);
      if(phandle->tx_request_pending) {
        SWITCH_STATE(phandle, STATE_RESP_PENDING_REQ);
        sched_post_task_prio(&execute_state_machine, DEFAULT_PRIORITY, phandle);
      } else {
        SWITCH_STATE(phandle, STATE_IDLE);
        hw_gpio_clr(phandle->uart_state_pin);
        sched_cancel_task(&listen);
        serial_protocol_disable(phandle);
      }
      break;
    }
    case STATE_IDLE:
      if(hw_gpio_get_in(phandle->target_uart_state_pin)) {
        // wake-up requested
        DPRINT("wake-up requested");
        phandle->target_uart_state_isr_count = 0;
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
        if(!phandle->use_dma || fifo_get_size(&phandle->rx_fifo) == 0)
        {
          if(phandle->use_dma)
          {
            clear_serial_protocol_timeout(phandle);
          }
#endif
          SWITCH_STATE(phandle, STATE_REC);
          listen(phandle);
          sched_post_task_prio(&execute_state_machine, MIN_PRIORITY, phandle);
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
        }
        else
        {
          // We are still processing previous data, wait until fifo is empty
          if(!serial_protocol_timeout(phandle))
          {
            sched_post_task_prio(&execute_state_machine, MIN_PRIORITY, phandle);
          }
        }
#endif
        break;
      } else if(phandle->tx_request_pending) { //check if we are really requesting a start
        SWITCH_STATE(phandle, STATE_REQ_START);
        // fall-through to STATE_REQ_START!
        __attribute__ ((fallthrough));
      } else
      {
        break;
      }
    case STATE_REQ_START:
      sched_cancel_task(&listen);
      SWITCH_STATE(phandle, STATE_REQ_WAIT);
      hw_gpio_set(phandle->uart_state_pin); // wake-up receiver
      DPRINT("wake-up receiver\n");
      sched_post_task_prio(&execute_state_machine, MIN_PRIORITY, phandle); // reschedule again to prevent sleeoping
      // in principle we could go to sleep but this will cause pin to float, this can be improved later
      break;
    case STATE_REQ_WAIT:
      if(hw_gpio_get_in(phandle->target_uart_state_pin)) {
        phandle->target_uart_state_isr_count = 0;
        clear_serial_protocol_timeout(phandle);
        // receiver active
        SWITCH_STATE(phandle, STATE_REQ_BUSY);
        // fall-through to STATE_REQ_BUSY!
        __attribute__ ((fallthrough));
      } else {
        if(!serial_protocol_timeout(phandle))
        {
          sched_post_task_prio(&execute_state_machine, MIN_PRIORITY, phandle);
        }
        // in principle we could go to sleep but this will cause pin to float, this can be improved later
        break;
      }
    case STATE_REQ_BUSY:
      if(phandle->tx_request_pending) {
        //if receiver reacts fast this code is executed twice (once due to the fall through of the previous state and once scheduled by the uart_int_cb)
        //To be sure that serial_protocol_enable is not called the second time when the transfer is already busy it is guaranteed that the code is only executed once.
        if(!sched_is_scheduled(&flush_serial_protocol_tx_fifo) 
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
        || (phandle->use_dma && !phandle->tx_dma_busy)
#endif
        )
        {
          clear_serial_protocol_timeout(phandle);
          serial_protocol_enable(phandle);
          sched_post_task_prio(&flush_serial_protocol_tx_fifo, MIN_PRIORITY, phandle);
        }
      } else if ((!hw_gpio_get_in(phandle->target_uart_state_pin) || !uart_get_rx_port_state(phandle->uart))
                 || (hw_gpio_get_in(phandle->target_uart_state_pin) && phandle->target_uart_state_isr_count > 1)) // AL-2405: We have missed going down of target_uart_state_pin
      {
          clear_serial_protocol_timeout(phandle);
        phandle->target_uart_state_isr_count = 0;
        SWITCH_STATE(phandle, STATE_IDLE);
        sched_post_task_prio(&execute_state_machine, DEFAULT_PRIORITY, phandle);
      } else{
        if(!serial_protocol_timeout(phandle))
        {
          sched_post_task_prio(&execute_state_machine, MIN_PRIORITY, phandle);
        }
        //keep active until target reacts
      }
      break;
    case STATE_RESP_PENDING_REQ:
      assert(phandle->tx_request_pending);
      // response period completed, initiate pending request by switching to REQ_START
      assert(!hw_gpio_get_in(phandle->target_uart_state_pin));
      hw_gpio_clr(phandle->uart_state_pin);
      SWITCH_STATE(phandle, STATE_REQ_START);
      sched_post_task_prio(&execute_state_machine, DEFAULT_PRIORITY, phandle);
      break;
    default:
      DPRINT("unexpected state %i\n", phandle->state);
      assert(false);
  }
}
#endif

/** @Brief Check package counter and crc
 *  @return void
 */
static bool verify_payload(priv_serial_protocol_handle_t* phandle, fifo_t* bytes, uint8_t* frame_header)
{
  static uint8_t payload[RX_BUFFER_SIZE - SERIAL_FRAME_HEADER_SIZE]; // statically allocated so this does not end up on stack
  fifo_peek(bytes, payload, 0, frame_header[SERIAL_FRAME_SIZE]);

  //check for missing packages
  phandle->packet_down_counter++;
  if(frame_header[SERIAL_FRAME_COUNTER]!=phandle->packet_down_counter)
  {
    //TODO consequence? (save total missing packages?)
    log_print_string("!!! missed packages: %i",(frame_header[SERIAL_FRAME_COUNTER]-phandle->packet_down_counter));
    phandle->packet_down_counter=frame_header[SERIAL_FRAME_COUNTER]; //reset package counter
  }

  DPRINT("RX HEADER: ");
  DPRINT_DATA(frame_header, SERIAL_FRAME_HEADER_SIZE);
  DPRINT("RX PAYLOAD: ");
  DPRINT_DATA(payload, frame_header[SERIAL_FRAME_SIZE]);

  uint16_t calculated_crc = crc_calculate(payload, frame_header[SERIAL_FRAME_SIZE]);
 
  if(frame_header[SERIAL_FRAME_CRC1]!=((calculated_crc >> 8) & 0x00FF) || frame_header[SERIAL_FRAME_CRC2]!=(calculated_crc & 0x00FF))
  {
    //TODO consequence? (request repeat?)
    log_print_string("CRC incorrect!");
    return false;
  }
  else
    return true;
}

/**
 * @brief reset serial protocol because of error or reset of the other party
 * 
 */
static void serial_protocol_clear_handler(serial_protocol_handle_t* handle) {
    priv_serial_protocol_handle_t* phandle = (priv_serial_protocol_handle_t*)handle->priv_data;
    serial_protocol_disable(phandle);

    //clear RX
    phandle->parsed_header = false;
    phandle->payload_len = 0;
    fifo_clear(&phandle->rx_fifo);

    //clear TX
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
    phandle->tx_dma_busy = false;
    phandle->tx_size = 0;
#endif
    phandle->target_uart_state_isr_count = 0;
    phandle->tx_request_pending = false;
    fifo_clear(&phandle->tx_fifo);
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
    if(phandle->use_interrupt_lines)
    {
        //AL-2305 be sure to clear request pin
        hw_gpio_clr(phandle->uart_state_pin);
    }
    phandle->serial_protocol_timeout_active = false;
    SWITCH_STATE(phandle, STATE_IDLE);
#endif
}


static void uart_error_callback(uart_handle_t* uart, uart_error_t error) {
    priv_serial_protocol_handle_t* phandle = get_serial_protocol(uart);
    log_print_string("UART ERROR %i", error);
    if(error == UART_OVERRUN_ERROR) {
      phandle->parsed_header = false;
      phandle->payload_len = 0;
      fifo_clear(&phandle->rx_fifo);
    }
}

/** @Brief Processes received uart data
 * 1) Search for sync bytes (always)
 * 2) get header size and parse header
 * 3) Wait for correct # of bytes (length present in header)
 * 4) Execute crc check and check message counter
 * 5) send to corresponding service (alp, ping service, log service)
 *  @return void
 */
static void process_rx_fifo(void *arg) 
{
  assert(arg!=NULL);
  priv_serial_protocol_handle_t* phandle = (priv_serial_protocol_handle_t*)arg;
  if(!phandle->parsed_header) 
  {
    if(fifo_get_size(&phandle->rx_fifo) > SERIAL_FRAME_HEADER_SIZE) 
    {
        fifo_peek(&phandle->rx_fifo, phandle->header, 0, SERIAL_FRAME_HEADER_SIZE);

        if(phandle->header[0] != SERIAL_FRAME_SYNC_BYTE || phandle->header[1] != SERIAL_FRAME_VERSION) 
        {
          fifo_skip(&phandle->rx_fifo, 1);
          DPRINT("skip");
          phandle->parsed_header = false;
          phandle->payload_len = 0;
          if(fifo_get_size(&phandle->rx_fifo) > SERIAL_FRAME_HEADER_SIZE)
            sched_post_task_prio(&process_rx_fifo, DEFAULT_PRIORITY, arg);
          return;
        }
        phandle->parsed_header = true;
        fifo_skip(&phandle->rx_fifo, SERIAL_FRAME_HEADER_SIZE);
        phandle->payload_len = phandle->header[SERIAL_FRAME_SIZE];
        DPRINT("UART RX, payload size = %i", phandle->payload_len);
        sched_post_task_prio(&process_rx_fifo, DEFAULT_PRIORITY, arg);
    }
  }
  else 
  {
    if(fifo_get_size(&phandle->rx_fifo) < phandle->payload_len) {
      return;
    }
    // payload complete, start parsing
    // rx_fifo can be bigger than the current serial packet, init a subview fifo
    // which is restricted to payload_len so we can't parse past this packet.
    fifo_t payload_fifo;
    fifo_init_subview(&payload_fifo, &phandle->rx_fifo, 0, phandle->payload_len);
  
    if(verify_payload(phandle, &payload_fifo,phandle->header))
    {
      serial_protocol_handle_t* sp_handle = container_of((uint32_t(*)[])(void*)phandle, serial_protocol_handle_t, priv_data);
      if(phandle->header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_ALP_DATA && phandle->alp_handler != NULL)
        phandle->alp_handler(sp_handle,SERIAL_MESSAGE_TYPE_ALP_DATA, &payload_fifo);
      else if (phandle->header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_PING_RESPONSE  && phandle->ping_response_handler != NULL)
        phandle->ping_response_handler(sp_handle, SERIAL_MESSAGE_TYPE_PING_RESPONSE, &payload_fifo);
      else if (phandle->header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_LOGGING && phandle->logging_handler != NULL)
        phandle->logging_handler(sp_handle, SERIAL_MESSAGE_TYPE_LOGGING, &payload_fifo);
      else if (phandle->header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_PING_REQUEST)
      {
        if(phandle->ping_request_handler != NULL)
        {
          phandle->ping_request_handler(sp_handle, SERIAL_MESSAGE_TYPE_PING_REQUEST, &payload_fifo);
        }
        uint8_t ping_reply[1]={0x02};
        fifo_skip(&payload_fifo,1);
        serial_protocol_handle_t* handle = container_of((uint32_t(*)[])(void*)phandle, serial_protocol_handle_t, priv_data);
        handle->driver->serial_protocol_transfer_bytes(handle, ping_reply,1,SERIAL_MESSAGE_TYPE_PING_RESPONSE);
      }
      else if(phandle->header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_REBOOTED)
      {
        uint8_t reboot_reason;
        fifo_pop(&payload_fifo, &reboot_reason, 1);
        DPRINT("target rebooted, reason=%i\n", reboot_reason);
        if(phandle->target_rebooted_cb)
          phandle->target_rebooted_cb(sp_handle, reboot_reason);
      }
      else
      {
        fifo_skip(&payload_fifo, phandle->payload_len);
        DPRINT("!!!FRAME TYPE NOT IMPLEMENTED");
      }
      fifo_skip(&phandle->rx_fifo, phandle->payload_len - fifo_get_size(&payload_fifo)); // pop parsed bytes from original fifo
    }
    else 
    {
        DPRINT("!!!PAYLOAD DATA INCORRECT");
    }
      
    phandle->payload_len = 0;
    phandle->parsed_header = false;
    if(fifo_get_size(&phandle->rx_fifo) > SERIAL_FRAME_HEADER_SIZE)
      sched_post_task_prio(&process_rx_fifo, DEFAULT_PRIORITY, arg);
  }
}

#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
static void uart_tx_cb(uart_handle_t* uart)
{
  priv_serial_protocol_handle_t* phandle = get_serial_protocol(uart);
  phandle->tx_dma_busy = false;
  if(phandle != NULL)
  {
    fifo_skip(&phandle->tx_fifo, phandle->tx_size);
  }
  else
  {
    DPRINT("uart_tx_cb not handled!");
  }
}
#endif

/** @Brief put received UART data in fifo
 *  @return void
 */
static void uart_rx_callback(uart_handle_t* uart, uint8_t data)
{
  priv_serial_protocol_handle_t* phandle = get_serial_protocol(uart);
  if(phandle != NULL)
  {
    if(fifo_put(&phandle->rx_fifo, &data, 1) != SUCCESS)
    {
      DPRINT("RX fifo overflow!");
    }
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
    if(!phandle->use_interrupt_lines)
#endif
    {
      sched_post_task_prio(&process_rx_fifo, DEFAULT_PRIORITY, phandle);
    }
  }
  else
  {
    DPRINT("uart_rx_callback not handled!");
  }
}


#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
/** @Brief Processes events on UART interrupt line
 *  @return void
 */
static void uart_int_cb(void *arg)
{
  priv_serial_protocol_handle_t* phandle = (priv_serial_protocol_handle_t*)arg;
  phandle->target_uart_state_isr_count++;
  // do not read GPIO level here in interrupt context (GPIO clock might not be enabled yet), execute state machine instead
  sched_post_task_prio(&execute_state_machine, DEFAULT_PRIORITY, arg);
}
#endif

static void serial_protocol_set_rx_interrupt_callback(uart_handle_t* uart, uart_rx_inthandler_t uart_rx_cb) {
#ifdef PLATFORM_USE_USB_CDC
	cdc_set_rx_interrupt_callback(uart, uart_rx_cb);
#else
  uart_set_rx_interrupt_callback(uart, uart_rx_cb);
#endif
}

static void serial_protocol_set_error_callback(uart_handle_t* uart, uart_error_handler_t uart_error_cb) {
    uart_set_error_callback(uart, uart_error_cb);
}

void serial_protocol_init(serial_protocol_handle_t* handle, uint8_t idx, uint32_t baudrate,
        bool use_interrupt_lines, pin_id_t uart_state_pin_id, pin_id_t target_uart_state_pin_id,
        bool use_dma, uint8_t dma_rx_channel, uint8_t dma_tx_channel)
{
  assert(handle != NULL);
#ifndef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
  assert(!use_interrupt_lines);
#endif
#ifndef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
  assert(!use_dma);
#endif
  if(use_dma) { assert(use_interrupt_lines); }

  assert(sched_register_task_allow_multiple(&flush_serial_protocol_tx_fifo, true) == SUCCESS);
  assert(sched_register_task_allow_multiple(&process_rx_fifo, true) == SUCCESS);
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
  if(use_interrupt_lines)
  {
    assert(sched_register_task_allow_multiple(&execute_state_machine, true) == SUCCESS);
    assert(sched_register_task_allow_multiple(&listen, true) == SUCCESS);
  }
#endif

  handle->driver = &serial_protocol_driver;
  priv_serial_protocol_handle_t* phandle = (priv_serial_protocol_handle_t*)handle->priv_data;
  init_priv_handle(phandle);
  add_serial_protocol(phandle);
  
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
  phandle->use_interrupt_lines = use_interrupt_lines;
  phandle->uart_state_pin=uart_state_pin_id;
  phandle->target_uart_state_pin=target_uart_state_pin_id;
#endif
  phandle->uart = uart_init(idx, baudrate,0);
  DPRINT("uart initialized");
  
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
  phandle->use_dma = use_dma;
  if(use_dma)
  {
    phandle->dma_rx_idx = dma_rx_channel;
    phandle->dma_rx = dma_channel_init(dma_rx_channel);
    phandle->dma_tx_idx = dma_tx_channel;
    phandle->dma_tx = dma_channel_init(dma_tx_channel);
    uart_set_tx_interrupt_callback(phandle->uart, &uart_tx_cb);
  }
  else
#endif
  {
    serial_protocol_set_rx_interrupt_callback(phandle->uart, &uart_rx_callback);
  }
  serial_protocol_set_error_callback(phandle->uart, uart_error_callback);

#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
  if(use_interrupt_lines)
  {
    DPRINT("using interrupt lines");
    assert(hw_gpio_configure_interrupt(target_uart_state_pin_id, GPIO_RISING_EDGE | GPIO_FALLING_EDGE, &uart_int_cb, phandle) == SUCCESS);
    assert(hw_gpio_enable_interrupt(target_uart_state_pin_id) == SUCCESS);
    if(hw_gpio_get_in(target_uart_state_pin_id))
    {
      sched_post_task_prio(&listen, DEFAULT_PRIORITY, phandle);
      DPRINT("Ready to receive (boot) @ %i",timer_get_counter_value());
    }
  }

// When not using interrupt lines we keep uart enabled so we can use RX IRQ.
// If the platform has interrupt lines the UART should be re-enabled when handling the interrupt
  else
#endif
  {
    serial_protocol_enable(phandle);
  }

  uint8_t reboot_reason = (uint8_t)hw_system_reboot_reason();
  log_print_error_string("rebooted with reason %i", reboot_reason);
#ifndef PLATFORM_NO_REBOOT_REASON_TRANSMIT
  serial_protocol_transfer_bytes(handle, &reboot_reason, 1, SERIAL_MESSAGE_TYPE_REBOOTED);
#endif
}

static error_t serial_protocol_transfer_bytes(serial_protocol_handle_t* handle, uint8_t* bytes, uint8_t length, serial_message_type_t type) 
{
  priv_serial_protocol_handle_t* phandle = (priv_serial_protocol_handle_t*)handle->priv_data;
  error_t result;
  uint8_t frame_header[SERIAL_FRAME_HEADER_SIZE];
  uint16_t crc=crc_calculate(bytes,length);

  phandle->packet_up_counter++;
  frame_header[0] = SERIAL_FRAME_SYNC_BYTE;
  frame_header[1] = SERIAL_FRAME_VERSION;

  frame_header[SERIAL_FRAME_COUNTER] = phandle->packet_up_counter;
  frame_header[SERIAL_FRAME_TYPE] = type;
  frame_header[SERIAL_FRAME_SIZE] = length;
  frame_header[SERIAL_FRAME_CRC1] = (crc >> 8) & 0x00FF;
  frame_header[SERIAL_FRAME_CRC2] = crc & 0x00FF;

  DPRINT("TX HEADER:");
  DPRINT_DATA(frame_header, SERIAL_FRAME_HEADER_SIZE);
  DPRINT("TX PAYLOAD:");
  DPRINT_DATA(bytes, length);
   
  start_atomic();
  if((sizeof(phandle->tx_buffer) - fifo_get_size(&phandle->tx_fifo)) >= (uint8_t) (SERIAL_FRAME_HEADER_SIZE + length))
  {
    phandle->tx_request_pending = true;
    fifo_put(&phandle->tx_fifo, (uint8_t*) &frame_header, SERIAL_FRAME_HEADER_SIZE);
    fifo_put(&phandle->tx_fifo, bytes, length);
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES
    if(phandle->use_interrupt_lines)
    {
      sched_post_task_prio(&execute_state_machine, MIN_PRIORITY, phandle);
    }
    else
#endif
    {
      sched_post_task_prio(&flush_serial_protocol_tx_fifo, MIN_PRIORITY, phandle); // state machine is not used when not using interrupt lines
    }

    result = SUCCESS;
  }
  else
  {
    result = -ENOMEM;
  }
  end_atomic();
  return result;
}

static error_t serial_protocol_transfer(serial_protocol_handle_t* handle, char* string)
{
  return serial_protocol_transfer_bytes(handle, (uint8_t*) string, strnlen(string, 100), SERIAL_MESSAGE_TYPE_LOGGING); 
}

static void serial_protocol_register_handler(serial_protocol_handle_t* handle, cmd_handler_t cmd_handler, serial_message_type_t type)
{
  priv_serial_protocol_handle_t* phandle = (priv_serial_protocol_handle_t*)handle->priv_data;
  switch(type) {
    case SERIAL_MESSAGE_TYPE_ALP_DATA:
      phandle->alp_handler = cmd_handler;
      break;
    case SERIAL_MESSAGE_TYPE_PING_REQUEST:
      phandle->ping_request_handler = cmd_handler;
      break;
    case SERIAL_MESSAGE_TYPE_PING_RESPONSE:
      phandle->ping_response_handler = cmd_handler;
      break;
    case SERIAL_MESSAGE_TYPE_LOGGING:
      phandle->logging_handler = cmd_handler;
      break;
    default:
      DPRINT("Serial protocol callback not implemented for type %d", type);
  }
}

static void serial_protocol_unregister_handler(serial_protocol_handle_t* handle, serial_message_type_t type)
{
  serial_protocol_register_handler(handle, NULL, type);
}

static void serial_protocol_set_target_rebooted_callback(serial_protocol_handle_t* handle, target_rebooted_callback_t cb)
{
  priv_serial_protocol_handle_t* phandle = (priv_serial_protocol_handle_t*)handle->priv_data;
  phandle->target_rebooted_cb = cb;
}
