/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "../types.h"
#include "addresses.h"

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

extern u8 device_id[8]; // TODO: keep this as global?
extern u8 virtual_id[2];

void PMM_SetVCore (u8 level);
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
