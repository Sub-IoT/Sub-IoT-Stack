/*
 * timer.c
 *
 *  Created on: 26-nov.-2012
 *      Author: Maarten Weyn
 */

#include <stddef.h>
#include <stdbool.h>

#include "../../log.h"

#include "cc430_addresses.h"


static void _timer_setvalue(u16 next_event)
{
	TA1CCR0 = next_event;
	TA1CTL |= TACLR;
}

static void _timer_enable_interrupt()
{
	TA1CCTL0 = CCIE; // Enable interrupt for CCR0
	TA1CTL |= MC__UP;
}

static void _timer_disable_interrupt()
{
	TA1CCTL0 &= ~CCIE; // Disable interrupt for CCR0
	TA1CTL &= ~MC__UP;
}

void _timer_init()
{
	//set timer to ticks (=1024 Hz)
	TA1CTL = TASSEL_1 + MC__UP + ID_3 + TACLR;           // ACLK/8, up mode, clear timer
	TA1EX0 = TAIDEX_3;							// divide /4
}

int16_t _timer_getvalue()
{
    return TA1R;
}

// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A (void)
{
    timer_completed();
}

