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

#if NUM_USERBUTTONS > 1
#include "userbutton.h"
#endif

#ifdef FRAMEWORK_LOG_ENABLED
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

uint8_t cc1101_interface_strobe(uint8_t); // prototype (to prevent warning) of internal driver function which is used here.
uint8_t cc1101_interface_write_single_reg(uint8_t, uint8_t);
uint8_t cc1101_interface_write_single_patable(uint8_t);

#define CHANNEL_COUNT_433_NORMAL_RATE 8
static uint8_t channel_indexes[CHANNEL_COUNT_433_NORMAL_RATE] = {0, 8, 16, 24, 32, 40, 48, 56};
static uint8_t current_channel_indexes_index = 0;

static hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .channel_header.ch_coding = PHY_CODING_PN9,
        .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
        .channel_header.ch_freq_band = PHY_BAND_433,
        .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

void rssi_valid_cb(int16_t rssi)
{
	// dummy, we are not staying in rx
}

void start()
{
    rx_cfg.channel_id.center_freq_index = channel_indexes[current_channel_indexes_index];
    hw_radio_set_rx(&rx_cfg, NULL, &rssi_valid_cb); // we 'misuse' hw_radio_set_rx to configure the channel (using the public API)
    hw_radio_set_idle(); // go straight back to idle

    cc1101_interface_write_single_reg(0x08, 0x22); // PKTCTRL0 random PN9 mode + disable data whitening
    //cc1101_interface_write_single_reg(0x12, 0x30); // MDMCFG2: use OOK modulation to clearly view centre freq on spectrum analyzer, comment for GFSK
    cc1101_interface_write_single_patable(0xc0); // 10dBm TX EIRP
    cc1101_interface_strobe(0x35); // strobe TX
}

#if NUM_USERBUTTONS > 1
void userbutton_callback(button_id_t button_id)
{
    // change channel and restart
    switch(button_id)
    {
        case 0:
            if(current_channel_indexes_index > 0)
                current_channel_indexes_index--;
            else
                current_channel_indexes_index = CHANNEL_COUNT_433_NORMAL_RATE - 1;
            break;
        case 1:
            if(current_channel_indexes_index < CHANNEL_COUNT_433_NORMAL_RATE - 1)
                current_channel_indexes_index++;
            else
                current_channel_indexes_index = 0;
    }

    sched_post_task(&start);
}
#endif

void bootstrap()
{
    DPRINT("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later

#if NUM_USERBUTTONS > 1
    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);
#endif

    hw_radio_init(NULL, NULL);

    sched_register_task(&start);
    sched_post_task(&start);
}
