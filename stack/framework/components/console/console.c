// console implementation
// author: Christophe VG <contact@christophe.vg>

#include "hwuart.h"

#include "framework_defs.h"
#include "platform.h"

#include "console.h"

#ifdef FRAMEWORK_CONSOLE_ENABLED

static uart_handle_t* uart;

void console_init(void) {
  uart = uart_init(CONSOLE_UART, CONSOLE_BAUDRATE, CONSOLE_LOCATION);
  uart_enable(uart);
}

void console_enable(void) {
  uart_enable(uart);
}

void console_disable(void) {
  uart_disable(uart);
}

inline void console_print_byte(uint8_t byte) {
  uart_send_byte(uart, byte);
}

inline void console_print_bytes(uint8_t* bytes, uint8_t length) {
  uart_send_bytes(uart, bytes, length);
}

inline void console_print(char* string) {
  uart_send_string(uart, string);
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
