// console implementation, _only_ the TX part (for now)
// author: Christophe VG <contact@christophe.vg

#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <stdio.h>

#include "hwuart.h"

#include "platform.h"

#ifdef CONSOLE_UART

void console_init(void);

// the elementary console actions

void console_print_byte(uint8_t byte);
void console_print_bytes(uint8_t* bytes, uint8_t length);
void console_print(char* string);

void console_set_rx_interrupt_callback(uart_rx_inthandler_t handler);
void console_rx_interrupt_enable();

#else

// no uart config

#pragma message "WARNING: no console configuration found !!!"

#define console_init()
#define console_print_byte(...)
#define console_print_bytes(...)
#define console_print(...)
#define console_set_rx_interrupt_callback(...);
#define console_rx_interrupt_enable();

#endif

// a few utilty wrappers

#define console_printf(...) printf(__VA_ARGS__); fflush(stdout)

#define TRY(msg, cmd) \
  console_print("+++ "msg"... "); console_print( cmd ? "OK\r\n" : "FAIL\r\n" );

#endif
