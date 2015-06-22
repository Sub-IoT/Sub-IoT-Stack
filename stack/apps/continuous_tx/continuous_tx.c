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

/*! \file
 * Test application which puts the radio in continous TX mode transmitting random data.
 * Usefull for measuring center frequency offset on a spectrum analyzer
 * Note: works only on cc1101 since this is not using the public hw_radio API but depends on cc1101 internal functions
 *
 *  Created on: Mar 24, 2015
 *  Authors:
 *  	glenn.ergeerts@uantwerpen.be
 */

#include "platform_defs.h"
#ifndef USE_CC1101
    #error "This application only works with cc1101"
#endif

#include "log.h"
#include "hwradio.h"

#ifdef FRAMEWORK_LOG_ENABLED
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

uint8_t cc1101_interface_strobe(uint8_t); // prototype (to prevent warning) of internal driver function which is used here.
uint8_t cc1101_interface_write_single_reg(uint8_t, uint8_t);
uint8_t cc1101_interface_write_single_patable(uint8_t);

void bootstrap()
{
    DPRINT("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later

    hw_radio_init(NULL, NULL);

    cc1101_interface_write_single_reg(0x08, 0x22); // PKTCTRL0 random PN9 mode + disable data whitening
    //cc1101_interface_write_single_reg(0x12, 0x30); // MDMCFG2: use OOK modulation to clearly view centre freq on spectrum analyzer, comment for GFSK
    cc1101_interface_write_single_patable(0xc0); // 10dBm TX EIRP
    cc1101_interface_strobe(0x35); // strobe TX
}
