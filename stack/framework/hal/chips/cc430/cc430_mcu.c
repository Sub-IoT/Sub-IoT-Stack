/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file cc430_mcu.c
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#include "ucs.h"

#include "msp430.h"

#define CLOCK_FREQ_MHZ 4000000
#define REF_FREQ_KHZ 32768

void __cc430_mcu_init()
{
    // output SMCLK on P.7 for debugging
	//	PMAPKEYID = 0x2D52; // unlock
	//	P3MAP7 = PM_SMCLK;
	//	P3SEL = 0xFF;//(1 << 6);

    UCS_clockSignalInit(UCS_FLLREF,	UCS_REFOCLK_SELECT,	UCS_CLOCK_DIVIDER_1);   // set DCO FLL reference = REFO
    UCS_clockSignalInit(UCS_ACLK, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);     // Set ACLK = REFO

    UCS_initFLLSettle(CLOCK_FREQ_MHZ / 1000, (uint16_t) (CLOCK_FREQ_MHZ / REF_FREQ_KHZ)); // blocks until clock settled on requested freq

    // uint32_t freq = UCS_getMCLK(); // for verifying

    WDTCTL = WDTPW + WDTHOLD; // TODO stop WDT for now
}

