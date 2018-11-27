#include "hwuart.h"

#include "framework_defs.h"
#include "platform_defs.h"
#include "fifo.h"
#include "scheduler.h"
#include "modem_interface.h"
#include "hal_defs.h"
#include "debug.h"
#include "errors.h"
//#include "platform.h"
#include "platform_defs.h"

#include "hwsystem.h"
#include "hwatomic.h"
#include "crc.h"

#include "ng.h"
#include "crc.h"

#include "log.h"

#define MODEM_INTERFACE_ENABLED 1

#ifdef MODEM_INTERFACE_ENABLED
#define PLATFORM_MODEM_INTERFACE_UART 1
#define PLATFORM_MODEM_INTERFACE_BAUD 9600
#define MODEM_HEADER_SIZE 7
#define RX_BUFFER_SIZE 256

#define TX_FIFO_FLUSH_CHUNK_SIZE 10 // at a baudrate of 115200 this ensures completion within 1 ms
                                    // TODO baudrate dependent

static uart_handle_t* uart;

static uint8_t rx_buffer[RX_BUFFER_SIZE];
static fifo_t rx_fifo;
static uint8_t idx1;
static uint32_t baudrate1;
static uint8_t pins1;


//#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_MODEM_LOG_ENABLED)
  #define DPRINT(...) log_print_string(__VA_ARGS__)
  #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
//#else
   // #define DPRINT(...)
   // #define DPRINT_DATA(...)
//#endif

#define MODEM_INTERFACE_TX_FIFO_SIZE 255
static uint8_t modem_interface_tx_buffer[MODEM_INTERFACE_TX_FIFO_SIZE];
static fifo_t modem_interface_tx_fifo;
static bool flush_in_progress = false;
static bool parsed_header = false;
static bool waiting_for_receiver = false;
uint8_t header[SERIAL_FRAME_HEADER_SIZE];
static uint8_t payload_len = 0;
static uint8_t packet_up_counter = 0;
static uint8_t packet_down_counter = 0;
static pin_id_t uart_state_pin;
static pin_id_t target_uart_state_pin;

static bool modem_listen_uart_inited = false;

static void platform_wakeup();
static void platform_release();
cmd_handler_t modem_to_app_handler;
cmd_handler_t app_to_modem_handler;

static void flush_modem_interface_tx_fifo(void *arg) 
{
  uint8_t len = fifo_get_size(&modem_interface_tx_fifo);

  if(!flush_in_progress)
    platform_wakeup();
  if(waiting_for_receiver)
  {
    sched_post_task_prio(&flush_modem_interface_tx_fifo, MIN_PRIORITY, NULL);
    return;
  }
    
  flush_in_progress = true;

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
    flush_in_progress = false;
    waiting_for_receiver = false;
    platform_release();
  } 
  else 
  {
    fifo_pop(&modem_interface_tx_fifo, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
    uart_send_bytes(uart, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
    sched_post_task_prio(&flush_modem_interface_tx_fifo, MIN_PRIORITY, NULL);
  }
#endif
}

void modem_interface_register_handler(cmd_handler_t cmd_handler, serial_message_type_t type)
{
  if(type == APP_TO_MODEM) //TODO put in enum
    modem_to_app_handler=cmd_handler;
  else if(type == MODEM_TO_APP )
    app_to_modem_handler=cmd_handler;
  else
    DPRINT("Modem interface callback not implemented");
}

static uint8_t *convertFrom16To8(uint16_t dataAll) {
    static uint8_t arrayData[2] = { 0x00, 0x00 };

    *(arrayData) = (dataAll >> 8) & 0x00FF;
    arrayData[1] = dataAll & 0x00FF;
    return arrayData;
}

static bool verify_payload(fifo_t* bytes, uint8_t* header)
{
  uint8_t payload[header[SERIAL_FRAME_SIZE]];
  fifo_peek(bytes, (uint8_t*) &payload, 0, header[SERIAL_FRAME_SIZE]);

  //check for missing packages
  packet_down_counter++;
  if(packet_down_counter==200)
    packet_down_counter=0; //reset package count
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
  uint8_t *dataMix = convertFrom16To8(calculated_crc);
 
  if(dataMix[0]!=header[SERIAL_FRAME_CRC1] || dataMix[1]!=header[SERIAL_FRAME_CRC2]) //TODO inline
  {
    //TODO consequence? (request repeat?)
    log_print_string("CRC incorrect! Calculated crc: ");
    DPRINT_DATA(dataMix, 2);
    return false;
  }
  else
    return true;
}
/*
---------------HEADER(bytes)---------------------
|sync|sync|counter|message type|length|crc1|crc2|
-------------------------------------------------
1) Search for sync bytes (always)
2) get header size and parse header
3) Wait for correct # of bytes (length present in header)
4) Execute crc check and check message counter
5) send to corresponding service (alp, ping service, log service)
*/
static void process_rx_fifo(void *arg) 
{
  if(!parsed_header) 
  {
    if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE) 
    {
        fifo_peek(&rx_fifo, header, 0, SERIAL_FRAME_HEADER_SIZE);
        DPRINT_DATA(header, SERIAL_FRAME_HEADER_SIZE); // TODO tmp

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
      if(header[SERIAL_FRAME_TYPE]==APP_TO_MODEM)
        app_to_modem_handler(&payload_fifo);
      else if(header[SERIAL_FRAME_TYPE]==MODEM_TO_APP)
        modem_to_app_handler(&payload_fifo);
      else if (header[SERIAL_FRAME_TYPE]==PING)
      {
        uint8_t bytes[1];
        bytes[0]=1;
        modem_interface_print_bytes((uint8_t*) &bytes,1,PING);
      }
      fifo_skip(&rx_fifo, payload_len - fifo_get_size(&payload_fifo)); // pop parsed bytes from original fifo
    }
    else
    {
      DPRINT("!!!PAYLOAD DATA INCORRECT");
    }
    payload_len = 0;
    parsed_header = false;
     if(fifo_get_size(&rx_fifo) > SERIAL_FRAME_HEADER_SIZE)
        sched_post_task(&process_rx_fifo);
  }
}

static void uart_rx_cb(uint8_t data)
{
    error_t err;

    start_atomic();
        err = fifo_put(&rx_fifo, &data, 1); assert(err == SUCCESS);
    end_atomic();

    if(!sched_is_scheduled(&process_rx_fifo))
        sched_post_task_prio(&process_rx_fifo, MIN_PRIORITY - 1, NULL);
}
static void modem_listen(void* arg) 
{
  if(!modem_listen_uart_inited) 
  {
    modem_listen_uart_inited = true;
    modem_interface_enable();
    //set interrupt gpio to indicate ready for data
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
    hw_gpio_set(uart_state_pin);
#endif   
  }

  if(hw_gpio_get_in(target_uart_state_pin)) 
  {
    // prevent the MCU to go back to stop mode by scheduling ourself again until pin goes low,
    // to keep UART RX enabled
    sched_post_task_prio(&modem_listen, MIN_PRIORITY, NULL);
  }
  else 
  {
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
    hw_gpio_clr(uart_state_pin);
#endif  
    DPRINT("UART released\n");
    modem_interface_disable();
  }
}

static void uart_int_cb(pin_id_t pin_id, uint8_t event_mask)
{
  if(event_mask & GPIO_RISING_EDGE) 
  {
    DPRINT("UART wakeup requested");
    /*
      Delay uart init until scheduled task,
      MCU clock will only be initialzed correclty after ISR, 
      when entering scheduler again
    */
    modem_listen_uart_inited = false;
    sched_post_task(&modem_listen);
  }
}

void modem_interface_init(uint8_t idx, uint32_t baudrate, uint8_t pins, pin_id_t uart_state_int_pin, pin_id_t target_uart_state_int_pin) 
{
  fifo_init(&modem_interface_tx_fifo, modem_interface_tx_buffer, MODEM_INTERFACE_TX_FIFO_SIZE);
  sched_register_task(&flush_modem_interface_tx_fifo);
  idx1=idx;
  baudrate1=baudrate;
  pins1=pins;
  uart_state_pin=uart_state_int_pin;
  target_uart_state_pin=target_uart_state_int_pin;

  uart = uart_init(idx1, baudrate1,pins1);
  DPRINT("uart initialized");
  
  fifo_init(&rx_fifo, rx_buffer, sizeof(rx_buffer));
  assert(sched_register_task(&modem_listen) == SUCCESS);

#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
  assert(hw_gpio_configure_interrupt(target_uart_state_pin, &uart_int_cb, GPIO_RISING_EDGE) == SUCCESS);
  assert(hw_gpio_enable_interrupt(target_uart_state_pin) == SUCCESS);
#endif

modem_interface_set_rx_interrupt_callback(&uart_rx_cb);

// When not using interrupt lines we keep uart enabled so we can use RX IRQ.
// If the platform has interrupt lines the UART should be re-enabled when handling the modem interrupt
#ifndef PLATFORM_USE_MODEM_INTERRUPT_LINES
  assert(uart_enable(uart_handle));
  assert(uart_rx_interrupt_enable(uart_handle) == SUCCESS);
#endif

  sched_register_task(&process_rx_fifo);
}

void modem_interface_enable(void) {
  //DPRINT("uart enabled");
  uart_enable(uart);
  uart_rx_interrupt_enable(uart);
}

void modem_interface_disable(void) {
  //DPRINT("uart disable");
  uart_disable(uart);
}

/*
---------------HEADER(bytes)---------------------
|sync|sync|counter|message type|length|crc1|crc2|
-------------------------------------------------
1) Calculate crc
2) Create frame
3) Add frame
*/
void modem_interface_print_bytes(uint8_t* bytes, uint8_t length, serial_message_type_t type) 
{
  uint8_t header[SERIAL_FRAME_HEADER_SIZE];
  uint16_t crc=crc_calculate(bytes,length);
  uint8_t* crc8= convertFrom16To8(crc);
  packet_up_counter++;
  if(packet_up_counter==200)
    packet_up_counter=0;
  header[0] = SERIAL_FRAME_SYNC_BYTE;
  header[1] = SERIAL_FRAME_VERSION;

  header[SERIAL_FRAME_COUNTER] = packet_up_counter;
  header[SERIAL_FRAME_TYPE] = type;
  header[SERIAL_FRAME_SIZE] = length;
  header[SERIAL_FRAME_CRC1] = crc8[0];
  header[SERIAL_FRAME_CRC2] = crc8[1];

  DPRINT("TX HEADER:");
  DPRINT_DATA(header, SERIAL_FRAME_HEADER_SIZE);
  DPRINT("TX PAYLOAD:");
  DPRINT_DATA(bytes, length);
   
  fifo_put(&modem_interface_tx_fifo, (uint8_t*) &header, SERIAL_FRAME_HEADER_SIZE);
  fifo_put(&modem_interface_tx_fifo, bytes, length);
  sched_post_task_prio(&flush_modem_interface_tx_fifo, MIN_PRIORITY, NULL);
}

void modem_interface_print(char* string) {
  //modem_interface_print_bytes((uint8_t*) string, strnlen(string, 100), LOGGING); //TODO
}

void modem_interface_set_rx_interrupt_callback(uart_rx_inthandler_t uart_rx_cb) {
#ifdef PLATFORM_USE_USB_CDC
	cdc_set_rx_interrupt_callback(uart_rx_cb);
#else
  uart_set_rx_interrupt_callback(uart, uart_rx_cb);
#endif
}

static void uart_off_if_released() {
  if(hw_gpio_get_out(uart_state_pin)==0 && hw_gpio_get_in(target_uart_state_pin)==0)
  {
      DPRINT("turning off modem");
      modem_interface_disable();
  }
  else
      DPRINT("Could not turn uart off"); //TODO rescheduele?
}

static void platform_wakeup()
{ 
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
  if(!waiting_for_receiver)
    hw_gpio_set(uart_state_pin);
  if(hw_gpio_get_in(target_uart_state_pin))
  {
    waiting_for_receiver=false;
    modem_interface_enable();
  }
  else
    waiting_for_receiver=true;
#endif
}

static void platform_release()
{
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
  hw_gpio_clr(uart_state_pin);
  uart_off_if_released();
#endif
}

#endif