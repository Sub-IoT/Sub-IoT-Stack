// console implementation
// author: Christophe VG <contact@christophe.vg>

#include <string.h>

#include "hwuart.h"

#include "framework_defs.h"
#ifdef FRAMEWORK_CONSOLE_ENABLED

#include "platform_defs.h"
#include "fifo.h"
#include "scheduler.h"
#include "console.h"
#include "hal_defs.h"
#include "debug.h"


#define TX_FIFO_FLUSH_CHUNK_SIZE 10 // at a baudrate of 115200 this ensures completion within 1 ms
                                    // TODO baudrate dependent

static uart_handle_t* uart;

#define CONSOLE_TX_FIFO_SIZE 255
static uint8_t console_tx_buffer[CONSOLE_TX_FIFO_SIZE];
static fifo_t console_tx_fifo;
static bool flush_in_progress = false;

static void flush_console_tx_fifo(void *arg) {
  uint8_t len = fifo_get_size(&console_tx_fifo);
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
  // TODO remove this when ALP serial interface is removed from console
  if(!flush_in_progress)
    platform_app_mcu_wakeup();
#endif
  flush_in_progress = true;
#ifdef HAL_UART_USE_DMA_TX
  // when using DMA we transmit the whole FIFO at once
  uint8_t buffer[CONSOLE_TX_FIFO_SIZE];
  fifo_pop(&console_tx_fifo, buffer, len);
  uart_send_bytes(uart, buffer, len);
#else
  // only send small chunks over uart each invocation, to make sure
  // we don't interfer with critical stack timings.
  // When there is still data left in the fifo this will be rescheduled
  // with lowest prio
  uint8_t chunk[TX_FIFO_FLUSH_CHUNK_SIZE];
  if(len < 10) {
    fifo_pop(&console_tx_fifo, chunk, len);
    uart_send_bytes(uart, chunk, len);
    flush_in_progress = false;
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
    platform_app_mcu_release();
#endif
  } else {
    fifo_pop(&console_tx_fifo, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
    uart_send_bytes(uart, chunk, TX_FIFO_FLUSH_CHUNK_SIZE);
    sched_post_task_prio(&flush_console_tx_fifo, MIN_PRIORITY, NULL);
  }
#endif
}

void console_init(void) {
  fifo_init(&console_tx_fifo, console_tx_buffer, CONSOLE_TX_FIFO_SIZE);
  sched_register_task(&flush_console_tx_fifo);

  uart = uart_init(PLATFORM_CONSOLE_UART, PLATFORM_CONSOLE_BAUDRATE, PLATFORM_CONSOLE_LOCATION);
  assert(uart_enable(uart));
}

void console_enable(void) {
  uart_enable(uart);
}

void console_disable(void) {
  uart_disable(uart);
}

inline void console_print_byte(uint8_t byte) {
  fifo_put_byte(&console_tx_fifo, byte);
  sched_post_task_prio(&flush_console_tx_fifo, MIN_PRIORITY, NULL);
}

inline void console_print_bytes(uint8_t* bytes, uint8_t length) {
  fifo_put(&console_tx_fifo, bytes, length);
  sched_post_task_prio(&flush_console_tx_fifo, MIN_PRIORITY, NULL);
}

inline void console_print(char* string) {
    uint8_t len = strnlen(string, CONSOLE_TX_FIFO_SIZE);

    // If no space enough in the TX FIFO, it's time to flush.
    while(len > (console_tx_fifo.max_size - fifo_get_size(&console_tx_fifo)))
        flush_console_tx_fifo(&console_tx_fifo);

    console_print_bytes((uint8_t*) string, len);
}

inline void console_set_rx_interrupt_callback(uart_rx_inthandler_t uart_rx_cb) {
#ifdef PLATFORM_USE_USB_CDC
	cdc_set_rx_interrupt_callback(uart_rx_cb);
#else
  uart_set_rx_interrupt_callback(uart, uart_rx_cb);
#endif
}

inline void console_rx_interrupt_enable() {
  uart_rx_interrupt_enable(uart);
}

#endif
