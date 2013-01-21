/*
 * phy_timing_test.c
 *
 *  Created on: 21-jan.-2013
 *      Author: Miesalex
 */

#include <msp430.h>
#include <stdint.h>
#include <phy/pn9.h>
#include <phy/fec.h>
#include <hal/driverlib/5xx_6xx/timera.h>

#include "phy_tests.h"

#define TIMER_OFFSET 17
#define HWREG(x) (*((volatile unsigned int *)(x)))

unsigned int test_pn9_timing(void)
{
	uint16_t pn9;
	uint8_t buffer[16];

	TimerA_configureContinuousMode(__MSP430_BASEADDRESS_T0A5__, TIMERA_CLOCKSOURCE_SMCLK, TIMERA_CLOCKSOURCE_DIVIDER_1, TIMERA_TAIE_INTERRUPT_DISABLE, TIMERA_DO_CLEAR);

	pn9_init(&pn9);

	__disable_interrupt();
	TimerA_clear(__MSP430_BASEADDRESS_T0A5__);
	pn9_encode_decode(buffer, buffer, 16, &pn9);
	TimerA_stop(__MSP430_BASEADDRESS_T0A5__);
	__enable_interrupt();

	return TA0R - TIMER_OFFSET;
}

unsigned int test_conv_encode_timing(void)
{
	uint8_t conv_state;
	uint8_t buffer[16];
	uint8_t buffer2[32];

	TimerA_configureContinuousMode(__MSP430_BASEADDRESS_T0A5__, TIMERA_CLOCKSOURCE_SMCLK, TIMERA_CLOCKSOURCE_DIVIDER_1, TIMERA_TAIE_INTERRUPT_DISABLE, TIMERA_DO_CLEAR);
	conv_encode_init(&conv_state);

	__disable_interrupt();
	TimerA_clear(__MSP430_BASEADDRESS_T0A5__);
	conv_encode(buffer, buffer2, 16, &conv_state);
	TimerA_stop(__MSP430_BASEADDRESS_T0A5__);
	__enable_interrupt();

	return TA0R - TIMER_OFFSET;
}

unsigned int test_conv_decode_timing(void)
{
	uint8_t conv_state;
	CONVDECODESTATE conv_state2;
	uint8_t buffer[16];
	uint8_t buffer2[32];

	TimerA_configureContinuousMode(__MSP430_BASEADDRESS_T0A5__, TIMERA_CLOCKSOURCE_SMCLK, TIMERA_CLOCKSOURCE_DIVIDER_1, TIMERA_TAIE_INTERRUPT_DISABLE, TIMERA_DO_CLEAR);
	conv_encode_init(&conv_state);
	conv_encode(buffer, buffer2, 16, &conv_state);

	conv_decode_init(&conv_state2);

	__disable_interrupt();
	TimerA_clear(__MSP430_BASEADDRESS_T0A5__);
	conv_decode(buffer2, buffer, 32, &conv_state2);
	TimerA_stop(__MSP430_BASEADDRESS_T0A5__);
	__enable_interrupt();

	return TA0R - TIMER_OFFSET;
}

unsigned int test_interleaving_timing(void)
{
	uint8_t buffer[16];

	TimerA_configureContinuousMode(__MSP430_BASEADDRESS_T0A5__, TIMERA_CLOCKSOURCE_SMCLK, TIMERA_CLOCKSOURCE_DIVIDER_1, TIMERA_TAIE_INTERRUPT_DISABLE, TIMERA_DO_CLEAR);


	__disable_interrupt();
	TimerA_clear(__MSP430_BASEADDRESS_T0A5__);
	interleave_deinterleave(buffer, buffer, 16);
	TimerA_stop(__MSP430_BASEADDRESS_T0A5__);
	__enable_interrupt();

	return TA0R - TIMER_OFFSET;
}
