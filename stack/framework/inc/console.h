// console implementation, _only_ the TX part (for now)
// author: Christophe VG <contact@christophe.vg

#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <stdio.h>
#include <string.h>

#include "hwuart.h"

#include "framework_defs.h"
#include "platform.h"

#ifdef FRAMEWORK_CONSOLE_ENABLED

__LINK_C void console_init(void);
__LINK_C void console_enable(void);
__LINK_C void console_disable(void);

// the elementary console actions
__LINK_C void console_print_byte(uint8_t byte);
__LINK_C void console_print_bytes(uint8_t* bytes, uint8_t length);
__LINK_C void console_print(char* string);

__LINK_C void console_set_rx_interrupt_callback(uart_rx_inthandler_t handler);
__LINK_C void console_rx_interrupt_enable();

// a few utilty wrappers

#define console_printf(...) printf(__VA_ARGS__); fflush(stdout)

#define TRY_LABEL_LENGTH 25

#define TRY(msg, cmd) \
{\
  console_print("+++ "msg"... ");                           \
  for(uint8_t i=0; i<TRY_LABEL_LENGTH-strlen(msg); i++) {   \
    console_print_byte(' ');                                \
  } \
  bool outcome = cmd;                                       \
  console_print( outcome ? "OK\r\n" : "FAIL\r\n" );\
}
#else

// no uart config
#pragma message "WARNING: no console configuration found !!!"

#define console_init()                         ((void)0)
#define console_enable()                       ((void)0)
#define console_dissable()                     ((void)0)

#define console_print_byte(...)                ((void)0)
#define console_print_bytes(...)               ((void)0)
#define console_print(...)                     ((void)0)

#define console_set_rx_interrupt_callback(...) ((void)0)
#define console_rx_interrupt_enable()          ((void)0)

#define console_printf(...)                    ((void)0)
#define TRY(msg, cmd) cmd

#endif

#endif
