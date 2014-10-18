/*! \file system.h
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * \author maarten.weyn@uantwerpen.be
 * \author glenn.ergeerts@uantwerpen.be
 *
 *  \brief High Level API for hardware
 *
 *  The high level API to initialize the hardware
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "../types.h"
#include "../framework/queue.h"

//#define ENTER_CRITICAL_SECTION(x)  	__disable_interrupt(); //st( x = __read_status_register(); __disable_interrupt(); )
//#define EXIT_CRITICAL_SECTION(x)    __enable_interrupt(); //__write_status_register(x)
#define ENTER_CRITICAL_SECTION(x)  	x = __get_interrupt_state(); __disable_interrupt()
#define EXIT_CRITICAL_SECTION(x)    __set_interrupt_state(x)

#define SWITCH_BYTES(x) (x << 8 | x >> 8)
#define SPLITUINT16(x) (uint8_t)((x) >> 8), (uint8_t)((x) & 0x00FF)
#define MERGEUINT16(x,y) (uint16_t)((x) << 8) | (uint16_t)(y)

#if defined(__MSPGCC__)
#define __ISR(a,b)   void __attribute__((interrupt (a))) b (void)
#define __even_in_range(a,b) (a)
#else
#define __ISR(a,b)   __interrupt void b (void)
#endif

#define AUX_CLOCK	32768

//extern uint8_t device_id[8]; // TODO: keep this as global?
//extern uint8_t virtual_id[2];
uint8_t* device_id;
uint8_t* virtual_id;

extern uint32_t clock_speed;

extern uint8_t vCore_level;
extern uint16_t target_clock_speed_kHz;
extern uint8_t init_IO;

queue_t tx_queue;
queue_t rx_queue;

void PMM_SetVCore (uint8_t level);
void system_init(uint8_t* tx_buffer, uint16_t tx_buffer_size, uint8_t* rx_buffer, uint16_t rx_buffer_size);

void system_watchdog_timer_stop();
void system_watchdog_timer_start();
void system_watchdog_timer_reset();
void system_watchdog_timer_enable_interrupt();
void system_watchdog_timer_init(unsigned char clockSelect, unsigned char clockDivider); // TODO refactor (params?)
void system_watchdog_init(unsigned char clockSelect, unsigned char clockDivider); // TODO refactor (params?)

void system_lowpower_mode(unsigned char mode, unsigned char enableInterrupts);

void system_check_set_unique_id();

#endif // __SYSTEM_H__
