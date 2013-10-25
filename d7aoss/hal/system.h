/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *     	glenn.ergeerts@uantwerpen.be
 *
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "../types.h"

#define ENTER_CRITICAL_SECTION(x)  	__disable_interrupt(); //st( x = __read_status_register(); __disable_interrupt(); )
#define EXIT_CRITICAL_SECTION(x)    __enable_interrupt(); //__write_status_register(x)

#define SWITCH_BYTES(x) (x << 8 | x >> 8)
#define SPLITUINT16(x) (uint8_t)((x) >> 8), (uint8_t)((x) & 0x00FF)

#if defined(__MSPGCC__)
#define __ISR(a,b)   void __attribute__((interrupt (a))) b (void)
#define __even_in_range(a,b) (a)
#else
#define __ISR(a,b)   __interrupt void b (void)
#endif

extern uint8_t device_id[8]; // TODO: keep this as global?
extern uint8_t virtual_id[2];

extern uint32_t clock_speed;

extern uint8_t vCore_level;
extern uint16_t target_clock_speed_kHz;
extern uint8_t init_IO;

void PMM_SetVCore (uint8_t level);
void system_init();

void system_watchdog_timer_stop();
void system_watchdog_timer_start();
void system_watchdog_timer_reset();
void system_watchdog_timer_enable_interrupt();
void system_watchdog_timer_init(unsigned char clockSelect, unsigned char clockDivider);
void system_watchdog_init(unsigned char clockSelect, unsigned char clockDivider);

void system_lowpower_mode(unsigned char mode, unsigned char enableInterrupts);

void system_get_unique_id(unsigned char *tagId);

#endif // __SYSTEM_H__
