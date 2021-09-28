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

// console implementation, _only_ the TX part (for now)
// author: Christophe VG <contact@christophe.vg

#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <stdio.h>
#include <string.h>

#include "hwuart.h"

#include "framework_defs.h"

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
#define console_disable()                     ((void)0)

#define console_print_byte(...)                ((void)0)
#define console_print_bytes(...)               ((void)0)
#define console_print(...)                     ((void)0)

#define console_set_rx_interrupt_callback(...) ((void)0)
#define console_rx_interrupt_enable()          ((void)0)

#define console_printf(...)                    ((void)0)
#define TRY(msg, cmd) cmd

#endif

#endif
