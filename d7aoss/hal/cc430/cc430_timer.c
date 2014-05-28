/*!
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
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *     	glenn.ergeerts@uantwerpen.be
 *
 */

#include <stddef.h>
#include <stdbool.h>

#include "../../framework/log.h"
#include "../../framework/timer.h"

#include "cc430_addresses.h"
#include <msp430.h>

static volatile uint8_t benchmarking_timer_rollover = 0;


void hal_timer_setvalue(uint16_t next_event)
{
	TA1CCR0 = next_event;
	//TA1CTL |= TACLR;
}

void hal_timer_enable_interrupt()
{
	TA1CCTL0 = CCIE; // Enable interrupt for CCR0
	//TA1CTL |= MC__UP;
	//TA1CTL |= MC__CONTINUOUS;
}

void hal_timer_disable_interrupt()
{
	TA1CCTL0 &= ~CCIE; // Disable interrupt for CCR0
	//TA1CTL &= ~MC__UP;
	//TA1CTL &= ~MC__CONTINUOUS;
}

void hal_timer_init()
{
	//set timer to ticks (=1024 Hz)
	//TA1CTL = TASSEL_1 + MC__UP + ID_3 + TACLR;           // ACLK/8, up mode, clear timer
	TA1CTL = TASSEL_1 + MC__CONTINUOUS + ID_3 + TACLR;           // ACLK/8, continuous up mode, clear timer
	TA1EX0 = TAIDEX_3;							// divide /4
}

uint16_t hal_timer_getvalue()
{
    return TA1R;
}

void hal_benchmarking_timer_init()
{
	#ifdef ENABLE_BENCHMARKING_TIMER
	//set timer to microticks (= 1 MHz)
	TA0CTL = TASSEL_2  + MC__CONTINUOUS + ID_0 + TACLR;           // SMCLK, continuous up mode, clear timer
	#endif
}

uint32_t hal_benchmarking_timer_getvalue()
{
	#ifdef ENABLE_BENCHMARKING_TIMER
    return TA0R + (benchmarking_timer_rollover * 0xFFFF);
	#else
    return 0;
	#endif
}

void hal_benchmarking_timer_start()
{
	#ifdef ENABLE_BENCHMARKING_TIMER
	A0CTL |= TAIE + TACLR;
	benchmarking_timer_rollover = 0;
	#endif
}

void hal_benchmarking_timer_stop()
{
	#ifdef ENABLE_BENCHMARKING_TIMER
	TA0CTL &= ~TAIE;
	#endif
}

// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A_CCO (void)
{
    timer_completed();

//    switch( TA0IV )
//    	 {
//    	   case  2: break;                          // CCR1 not used
//    	   case  4: break;                          // CCR2 not used
//    	   case 10: benchmarking_timer_rollover++;                  // overflow
//    	            break;
//    	 }
}

#ifdef ENABLE_BENCHMARKING_TIMER
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A_TA (void)
{
    //timer_completed();

    switch( TA0IV )
    	 {
    	   case 0x02: break;                          // CCR1 not used
    	   case 0x04: break;                          // CCR2 not used
    	   case 0x06: break;                          // CCR3 not used
    	   case 0x08: break;                          // CCR4 not used
    	   case 0x0A: break;                          // CCR5 not used
    	   case 0x0C: break;                          // CCR6 not used
    	   case 0x0E: benchmarking_timer_rollover++;                  // overflow
    	            break;
    	 }
}
#endif

