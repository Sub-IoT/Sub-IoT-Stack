/*! \file efm32gg_system.c
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
 *
 */

#include "system.h"
#include "uart.h"

#include "em_cmu.h"
#include "em_chip.h"

void system_init()
{
    /* Chip errata */
    CHIP_Init();

    if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

    // init clock
    CMU_ClockDivSet(cmuClock_HF, cmuClkDiv_2);       // Set HF clock divider to /2 to keep core frequency < 32MHz
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);   // Enable XTAL Osc and wait to stabilize
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO); // Select HF XTAL osc as system clock source. 48MHz XTAL, but we divided the system clock by 2, therefore our HF clock will be 24MHz

    uart_init();
}
