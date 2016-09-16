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

/*! \file efm32gg_mcu.c
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author daniel.vandenakker@uantwerpen.be
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include "em_cmu.h"
#include "em_chip.h"
#include "platform.h"

void __efm32gg_mcu_init()
{
    /* Chip errata */
    CHIP_Init();


#ifdef HW_USE_HFXO
    // init clock with HFXO (external)
    CMU_ClockDivSet(cmuClock_HF, cmuClkDiv_2);		// 24 MHZ
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);   // Enable XTAL Osc and wait to stabilize
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO); // Select HF XTAL osc as system clock source. 48MHz XTAL, but we divided the system clock by 2, therefore our HF clock will be 24MHz
    //CMU_ClockDivSet(cmuClock_HFPER, cmuClkDiv_4); // TODO set HFPER clock divider (used for SPI) + disable gate clock when not used?
#else
    // init clock with HFRCO (internal)
    CMU_HFRCOBandSet(cmuHFRCOBand_21MHz);
    CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
#endif
}

