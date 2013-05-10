/*
 * phy_timing_test.c
 *
 *  Created on: 21-jan.-2013
 *      Author: Miesalex
 */

#include <msp430.h>
#include <stdint.h>
#include <phy/fec.h>
#include <hal/cc430/driverlib/5xx_6xx/timera.h>

#include "phy_tests.h"


#define TIMER_OFFSET 17

unsigned int test_fec_encoding(void)
{
	uint8_t input[15] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
	uint8_t output[32];

	TimerA_configureContinuousMode(__MSP430_BASEADDRESS_T0A5__, TIMERA_CLOCKSOURCE_SMCLK, TIMERA_CLOCKSOURCE_DIVIDER_1, TIMERA_TAIE_INTERRUPT_DISABLE, TIMERA_DO_CLEAR);

	fec_init_encode(input);
	fec_set_length(15);

	//28.707
	TimerA_clear(__MSP430_BASEADDRESS_T0A5__);
	fec_encode(&output[0]);
	fec_encode(&output[4]);
	fec_encode(&output[8]);
	fec_encode(&output[12]);
	fec_encode(&output[16]);
	fec_encode(&output[20]);
	fec_encode(&output[24]);
	fec_encode(&output[28]);
	TimerA_stop(__MSP430_BASEADDRESS_T0A5__);

	return TA0R - TIMER_OFFSET;
}

unsigned int test_fec_decoding(void)
{
	uint8_t input[15] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E};
	uint8_t output[32];

	TimerA_configureContinuousMode(__MSP430_BASEADDRESS_T0A5__, TIMERA_CLOCKSOURCE_SMCLK, TIMERA_CLOCKSOURCE_DIVIDER_1, TIMERA_TAIE_INTERRUPT_DISABLE, TIMERA_DO_CLEAR);

	fec_init_encode(input);
	fec_set_length(15);
	fec_encode(&output[0]);
	fec_encode(&output[4]);
	fec_encode(&output[8]);
	fec_encode(&output[12]);
	fec_encode(&output[16]);
	fec_encode(&output[20]);
	fec_encode(&output[24]);
	fec_encode(&output[28]);

	memset(input, 0, 15);

	fec_init_decode(input);
	fec_set_length(15);

	TimerA_clear(__MSP430_BASEADDRESS_T0A5__);
	fec_decode(&output[0]);
	fec_decode(&output[4]);
	fec_decode(&output[8]);
	fec_decode(&output[12]);
	fec_decode(&output[16]);
	fec_decode(&output[20]);
	fec_decode(&output[24]);
	fec_decode(&output[28]);
	TimerA_stop(__MSP430_BASEADDRESS_T0A5__);

	return TA0R - TIMER_OFFSET;
}
