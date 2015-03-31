/*! \file
 * Test application which puts the radio in continous TX mode transmitting random data.
 * Usefull for measuring center frequency offset on a spectrum analyzer
 * Note: works only on cc1101 since this is not using the public hw_radio API but depends on cc1101 internal functions
 *
 *  Created on: Mar 24, 2015
 *  Authors:
 *  	glenn.ergeerts@uantwerpen.be
 */

// TODO ensure using cc1101 using cmake?

#include "log.h"
#include "hwradio.h"

uint8_t cc1101_interface_strobe(uint8_t); // prototype (to prevent warning) of internal driver function which is used here.

void bootstrap()
{
    log_print_string("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later

    hw_radio_init(NULL, NULL);

    cc1101_interface_strobe(0x35); // strobe TX
}
