#include "hwuart.h"

#include "framework_defs.h"
#include "platform_defs.h"
#include "fifo.h"
#include "scheduler.h"
#include "modem_interface.h"
#include "hal_defs.h"
#include "debug.h"
#include "errors.h"
#include "platform_defs.h"

#include "hwsystem.h"
#include "hwatomic.h"
#include "crc.h"

#include "ng.h"
#include "crc.h"

#include "log.h"

#define RX_BUFFER_SIZE 256

#define TX_FIFO_FLUSH_CHUNK_SIZE 10 // at a baudrate of 115200 this ensures completion within 1 ms
                                    // TODO baudrate dependent

static uart_handle_t* uart;

static uint8_t rx_buffer[RX_BUFFER_SIZE];
static fifo_t rx_fifo;

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_MODEM_INTERFACE_LOG_ENABLED)
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

#define MODEM_INTERFACE_TX_FIFO_SIZE 255
static uint8_t modem_interface_tx_buffer[MODEM_INTERFACE_TX_FIFO_SIZE];
static fifo_t modem_interface_tx_fifo;
static bool flush_in_progress = false;

uint8_t header[SERIAL_FRAME_HEADER_SIZE];
static uint8_t payload_len = 0;
static uint8_t packet_up_counter = 0;
static uint8_t packet_down_counter = 0;
static pin_id_t uart_state_pin;
static pin_id_t target_uart_state_pin;

static bool modem_listen_uart_inited = false;
static bool parsed_header = false;
static bool waiting_for_receiver = false;
static bool transmitting = false;
static bool receiving = false;


cmd_handler_t alp_handler;
cmd_handler_t ping_response_handler;
cmd_handler_t logging_handler;

/** @Brief Enable UART interface and UART interrupt
 *  @return void
 */
static void modem_interface_enable(void) 
{
  DPRINT("uart enabled @ %i",timer_get_counter_value());
  assert(uart_enable(uart));
  uart_rx_interrupt_enable(uart);
  modem_listen_uart_inited = true;
}

/** @Brief disables UART interface
 *  @return void
 */
static void modem_interface_disable(void) 
{
  DPRINT("uart disabled @ %i",timer_get_counter_value());
  modem_listen_uart_inited = false;
  assert(uart_disable(uart));
}

/** @brief Enables UART interrupt line in 
 *  to wake up the receiver.
 *  @return void
 */
static void wakeup_receiver()
{ 
  if(!waiting_for_receiver)
  {
    hw_gpio_set(uart_state_pin);
    waiting_for_receiver=true;
  }
}

/** @brief Lets receiver know that 
 *  all the data has been transfered
 *  @return void
 */
static void release_receiver()
{
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
  modem_interface_disable();
  hw_gpio_clr(uart_state_pin);
#endif
}

/** @brief transmit data in fifo to UART
 *  @return void
 */
static void flush_modem_interface_tx_fifo(void *arg) 
{
  uint8_t len = fifo_get_size(&modem_interface_tx_fifo);

#ifdef HAL_UART_USE_DMA_TX
  // when using DMA we transmit the whole FIFO at once
  uint8_t buffer[MODEM_INTERFACE_TX_FIFO_SIZE];
  fifo_pop(&modem_interface_tx_fifo, buffer, len);
  uart_send_bytes(uart, buffer, len);
#else
  // only send small chunks over uart each invocation, to make sure
  // we don't interfer with critical stack timings.
  // When there is still data left in the fifo this will be rescheduled
  // with lowest prio
  uint8_t chunk[TX_FIFO_FLUSH_CHUNK_SIZE];
  if(len < 10) 
  {
    fifo_pop(&modem_interface_tx_fifo, chunk, len);
    uart_send_bytes(uart, chunk, len);
    transmitting=false;
    release_receiver();
  } 
  else 
  {
    fifo_pop(&modem_interface_tx_fifo, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
    uart_send_bytes(uart, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
    sched_post_task_prio(&flush_modem_interface_tx_fifo, MIN_PRIORITY, NULL);
  }
#endif
}

/** @Brief Schedules flush tx fifo when receiver is ready
 *  @return void
 */
static void get_receiver_ready()
{
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
  if(!receiving)
  {
    transmitting=true;
    wakeup_receiver();
    DPRINT("request receiver");
    while(waiting_for_receiver){} //TODO timeout?
    modem_interface_enable();
    sched_post_task_prio(&flush_modem_interface_tx_fifo, MIN_PRIORITY, NULL);
  }
  else
  {
    sched_post_task_prio(&get_receiver_ready, MIN_PRIORITY, NULL);
  }
#endif
}

/** @Brief Check package counter and crc
 *  @return void
 */
static bool verify_payload(fifo_t* bytes, uint8_t* header)
{
  uint8_t payload[header[SERIAL_FRAME_SIZE]];
  fifo_peek(bytes, (uint8_t*) &payload, 0, header[SERIAL_FRAME_SIZE]);

  //check for missing packages
  packet_down_counter++;
  if(header[SERIAL_FRAME_COUNTER]!=packet_down_counter)
  {
    //TODO consequence? (save total missing packages?)
    log_print_string("!!! missed packages: %i",(header[SERIAL_FRAME_COUNTER]-packet_down_counter));
    packet_down_counter=header[SERIAL_FRAME_COUNTER]; //reset package counter
  }

  DPRINT("RX HEADER: ");
  DPRINT_DATA(header, SERIAL_FRAME_HEADER_SIZE);
  DPRINT("RX PAYLOAD: ");
  DPRINT_DATA(payload, header[SERIAL_FRAME_SIZE]);

  uint16_t calculated_crc = crc_calculate(payload, header[SERIAL_FRAME_SIZE]);
 
  if(header[SERIAL_FRAME_CRC1]!=((calculated_crc >> 8) & 0x00FF) || header[SERIAL_FRAME_CRC2]!=(calculated_crc & 0x00FF))
  {
    //TODO consequence? (request repeat?)
    log_print_string("CRC incorrect!");
    return false;
  }
  else
    return true;
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
  if(!parsed_header) 
  {
    if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE) 
    {
        fifo_peek(&rx_fifo, header, 0, SERIAL_FRAME_HEADER_SIZE);

        if(header[0] != SERIAL_FRAME_SYNC_BYTE || header[1] != SERIAL_FRAME_VERSION) 
        {
          fifo_skip(&rx_fifo, 1);
          DPRINT("skip");
          parsed_header = false;
          payload_len = 0;
          if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE)
            sched_post_task(&process_rx_fifo);
          return;
        }
        parsed_header = true;
        fifo_skip(&rx_fifo, SERIAL_FRAME_HEADER_SIZE);
        payload_len = header[SERIAL_FRAME_SIZE];
        DPRINT("UART RX, payload size = %i", payload_len);
        sched_post_task(&process_rx_fifo);
    }
  }
  else 
  {
    if(fifo_get_size(&rx_fifo) < payload_len) {
      return;
    }
    // payload complete, start parsing
    // rx_fifo can be bigger than the current serial packet, init a subview fifo
    // which is restricted to payload_len so we can't parse past this packet.
    fifo_t payload_fifo;
    fifo_init_subview(&payload_fifo, &rx_fifo, 0, payload_len);
  
    if(verify_payload(&payload_fifo,(uint8_t *)&header))
    {
      if(header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_ALP_DATA && alp_handler != NULL)
        alp_handler(&payload_fifo);
      else if (header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_PING_RESPONSE  && ping_response_handler != NULL)
        ping_response_handler(&payload_fifo);
      else if (header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_LOGGING && logging_handler != NULL)
        logging_handler(&payload_fifo);
      else if (header[SERIAL_FRAME_TYPE]==SERIAL_MESSAGE_TYPE_PING_REQUEST)
      {
        uint8_t ping_reply[1]={0x02};
        fifo_skip(&payload_fifo,1);
        modem_interface_transfer_bytes((uint8_t*) &ping_reply,1,SERIAL_MESSAGE_TYPE_PING_RESPONSE);
      }
      else
      {
        fifo_skip(&payload_fifo, payload_len);
        DPRINT("!!!FRAME TYPE NOT IMPLEMENTED");
      }
      fifo_skip(&rx_fifo, payload_len - fifo_get_size(&payload_fifo)); // pop parsed bytes from original fifo
    }
    else
      DPRINT("!!!PAYLOAD DATA INCORRECT");
    payload_len = 0;
    parsed_header = false;
    if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE)
      sched_post_task(&process_rx_fifo);
  }
}
/** @Brief put received UART data in fifo
 *  @return void
 */
static void uart_rx_cb(uint8_t data)
{
    error_t err;
    start_atomic();
        err = fifo_put(&rx_fifo, &data, 1); assert(err == SUCCESS);
    end_atomic();

    if(!sched_is_scheduled(&process_rx_fifo))
        sched_post_task_prio(&process_rx_fifo, MIN_PRIORITY - 1, NULL);
}
/** @Brief Keeps µC awake while receiving UART data
 *  @return void
 */
static void modem_listen(void* arg) 
{
  if(!modem_listen_uart_inited) 
  {
    modem_interface_enable();
    hw_gpio_set(uart_state_pin); //set interrupt gpio to indicate ready for data
  }

  if(receiving==true)
  {
    // prevent the MCU to go back to stop mode by scheduling ourself again until pin goes low,
    // to keep UART RX enabled
    sched_post_task_prio(&modem_listen, MIN_PRIORITY, NULL);
  }
  else 
  {
    hw_gpio_clr(uart_state_pin);
    modem_interface_disable();
  }
}
/** @Brief Processes events on UART interrupt line
 *  @return void
 */
static void uart_int_cb(pin_id_t pin_id, uint8_t event_mask)
{
  /*
    Delay uart init until scheduled task,
    MCU clock will only be initialzed correclty after ISR, 
    when entering scheduler again
  */
  if(event_mask & GPIO_RISING_EDGE) 
  {
    if(transmitting)
    {
      DPRINT("Target ready @ %i",timer_get_counter_value());
      waiting_for_receiver=false;
    }
    else
    {
      receiving=true;
      DPRINT("Ready to receive @ %i",timer_get_counter_value());
      sched_post_task(&modem_listen);
    }
  }
  else
    receiving=false;
}

static void modem_interface_set_rx_interrupt_callback(uart_rx_inthandler_t uart_rx_cb) {
#ifdef PLATFORM_USE_USB_CDC
	cdc_set_rx_interrupt_callback(uart_rx_cb);
#else
  uart_set_rx_interrupt_callback(uart, uart_rx_cb);
#endif
}

void modem_interface_init(uint8_t idx, uint32_t baudrate, pin_id_t uart_state_int_pin, pin_id_t target_uart_state_int_pin) 
{
  fifo_init(&modem_interface_tx_fifo, modem_interface_tx_buffer, MODEM_INTERFACE_TX_FIFO_SIZE);
  sched_register_task(&flush_modem_interface_tx_fifo);
  sched_register_task(&get_receiver_ready);
  sched_register_task(&process_rx_fifo);
  uart_state_pin=uart_state_int_pin;
  target_uart_state_pin=target_uart_state_int_pin;

  uart = uart_init(idx, baudrate,0);
  DPRINT("uart initialized");
  
  fifo_init(&rx_fifo, rx_buffer, sizeof(rx_buffer));
  modem_interface_set_rx_interrupt_callback(&uart_rx_cb);

#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
  assert(sched_register_task(&modem_listen) == SUCCESS);
  DPRINT("using interrupt lines");
  assert(hw_gpio_configure_interrupt(target_uart_state_pin, &uart_int_cb, GPIO_RISING_EDGE | GPIO_FALLING_EDGE) == SUCCESS);
  assert(hw_gpio_enable_interrupt(target_uart_state_pin) == SUCCESS);
  if(hw_gpio_get_in(target_uart_state_pin))
  {
    receiving=true;
    sched_post_task(&modem_listen);
    DPRINT("Ready to receive (boot) @ %i",timer_get_counter_value());
  }
#endif

// When not using interrupt lines we keep uart enabled so we can use RX IRQ.
// If the platform has interrupt lines the UART should be re-enabled when handling the modem interrupt
#ifndef PLATFORM_USE_MODEM_INTERRUPT_LINES
  modem_interface_enable();
#endif
}



void modem_interface_transfer_bytes(uint8_t* bytes, uint8_t length, serial_message_type_t type) 
{
  uint8_t header[SERIAL_FRAME_HEADER_SIZE];
  uint16_t crc=crc_calculate(bytes,length);

  packet_up_counter++;
  header[0] = SERIAL_FRAME_SYNC_BYTE;
  header[1] = SERIAL_FRAME_VERSION;

  header[SERIAL_FRAME_COUNTER] = packet_up_counter;
  header[SERIAL_FRAME_TYPE] = type;
  header[SERIAL_FRAME_SIZE] = length;
  header[SERIAL_FRAME_CRC1] = (crc >> 8) & 0x00FF;
  header[SERIAL_FRAME_CRC2] = crc & 0x00FF;

  DPRINT("TX HEADER:");
  DPRINT_DATA(header, SERIAL_FRAME_HEADER_SIZE);
  DPRINT("TX PAYLOAD:");
  DPRINT_DATA(bytes, length);
   
  fifo_put(&modem_interface_tx_fifo, (uint8_t*) &header, SERIAL_FRAME_HEADER_SIZE);
  fifo_put(&modem_interface_tx_fifo, bytes, length);

#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
  sched_post_task_prio(&get_receiver_ready, MIN_PRIORITY, NULL);
#else
  sched_post_task_prio(&flush_modem_interface_tx_fifo, MIN_PRIORITY, NULL);
#endif  
}

void modem_interface_transfer(char* string) {
  modem_interface_transfer_bytes((uint8_t*) string, strnlen(string, 100), SERIAL_MESSAGE_TYPE_LOGGING); 
}


void modem_interface_register_handler(cmd_handler_t cmd_handler, serial_message_type_t type)
{
  if(type == SERIAL_MESSAGE_TYPE_ALP_DATA) 
    alp_handler=cmd_handler;
  else if(type == SERIAL_MESSAGE_TYPE_PING_RESPONSE) 
    ping_response_handler=cmd_handler;
  else if(type == SERIAL_MESSAGE_TYPE_LOGGING) 
    logging_handler=cmd_handler;
  else
    DPRINT("Modem interface callback not implemented");
}