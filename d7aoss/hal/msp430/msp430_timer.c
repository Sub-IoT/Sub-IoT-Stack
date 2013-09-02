/*
 * timer.c
 *
 *  Created on: 26-nov.-2012
 *      Author: Maarten Weyn
 */

#include <stddef.h>
#include <stdbool.h>

#include "../../framework/log.h"
#include "../../framework/timer.h"

#include "msp430_addresses.h"
#include <msp430.h>

static volatile uint8_t benchmarking_timer_rollover = 0;


void hal_timer_setvalue(uint16_t next_event)
{
	TA0CCR0 = next_event;
	//TA1CTL |= TACLR;
}

void hal_timer_enable_interrupt()
{
	TA0CCTL0 = CCIE; // Enable interrupt for CCR0
	//TA1CTL |= MC__UP;
	//TA1CTL |= MC__CONTINUOUS;
}

void hal_timer_disable_interrupt()
{
	TA0CCTL0 &= ~CCIE; // Disable interrupt for CCR0
	//TA1CTL &= ~MC__UP;
	//TA1CTL &= ~MC__CONTINUOUS;
}

void hal_timer_init()
{
	//set timer to ticks (=1024 Hz)
	//TA1CTL = TASSEL_1 + MC__UP + ID_3 + TACLR;           // ACLK/8, up mode, clear timer
	TA0CTL = TASSEL_1 + MC__CONTINUOUS + ID_3 + TACLR;           // ACLK/8, continuous up mode, clear timer
	TA0EX0 = TAIDEX_3;							// divide /4
}

uint16_t hal_timer_getvalue()
{
    return TA0R;
}

void hal_benchmarking_timer_init()
{
	//set timer to microticks (= 1 MHz)
	//TA0CTL = TASSEL_2  + MC__CONTINUOUS + ID_0 + TACLR;           // SMCLK, continuous up mode, clear timer
}

uint32_t hal_benchmarking_timer_getvalue()
{
    //return TA0R + (benchmarking_timer_rollover * 0xFFFF);
}

void hal_benchmarking_timer_start()
{
	//TA0CCTL0 = CCIE; // Enable interrupt for CCR0
	//TA0CTL |= MC__CONTINUOUS;
	//TA0CTL |= TAIE + TACLR;
	//benchmarking_timer_rollover = 0;
}

void hal_benchmarking_timer_stop()
{
	//TA0CCTL0 &= ~CCIE; // Disable interrupt for CCR0
	//TA0CTL &= ~MC__CONTINUOUS;
	//TA0CTL &= ~TAIE;
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
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

//#pragma vector=TIMER0_A1_VECTOR
//__interrupt void Timer_A_TA (void)
//{
//    //timer_completed();
//
//    switch( TA0IV )
//    	 {
//    	   case 0x02: break;                          // CCR1 not used
//    	   case 0x04: break;                          // CCR2 not used
//    	   case 0x06: break;                          // CCR3 not used
//    	   case 0x08: break;                          // CCR4 not used
//    	   case 0x0A: break;                          // CCR5 not used
//    	   case 0x0C: break;                          // CCR6 not used
//    	   case 0x0E: benchmarking_timer_rollover++;                  // overflow
//    	            break;
//    	 }
//}


