// console implementation, _only_ the TX part (for now)
// author: Christophe VG <contact@christophe.vg

#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <stdio.h>
#include <string.h>

#include "hwuart.h"

#include "platform.h"

#ifdef CONSOLE_UART

__LINK_C void console_init(void);
__LINK_C void console_enable(void);
__LINK_C void console_disable(void);

// the elementary console actions
__LINK_C void console_print_byte(uint8_t byte);
__LINK_C void console_print_bytes(uint8_t* bytes, uint8_t length);
__LINK_C void console_print(char* string);

__LINK_C void console_set_rx_interrupt_callback(uart_rx_inthandler_t handler);
__LINK_C void console_rx_interrupt_enable();

#else

// no uart config

#ifndef NO_CONSOLE
#pragma message "WARNING: no console configuration found !!!"
#endif

#define console_init()
#define console_enable()
#define console_dissable()
#define console_print_byte(...)
#define console_print_bytes(...)
#define console_print(...)
#define console_set_rx_interrupt_callback(...);
#define console_rx_interrupt_enable();

#endif

// a few utilty wrappers

#define console_printf(...) printf(__VA_ARGS__); fflush(stdout)

#define TRY_LABEL_LENGTH 25

#define TRY(msg, cmd) \
  console_print("+++ "msg"... ");                           \
  for(uint8_t i=0; i<TRY_LABEL_LENGTH-strlen(msg); i++) {   \
    console_print_byte(' ');                                \
  }                                                         \
  console_print( cmd ? "OK\r\n" : "FAIL\r\n" );

#endif
