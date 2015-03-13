/*! \file cc430_mcu.c
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
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author daniel.vandenakker@uantwerpen.be
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

