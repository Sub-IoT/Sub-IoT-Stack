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

#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "platform_defs.h"

#if !defined (USE_CC1101) && !defined (USE_SI4460)
    #error "This application only works with cc1101 and SI4460"
#endif

#include "log.h"
#include "hwradio.h"

#if NUM_USERBUTTONS > 1
#include "button.h"
#endif

#ifdef HAS_LCD
#include "hwlcd.h"
#endif

#ifdef FRAMEWORK_LOG_ENABLED
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

#if defined USE_CC1101
uint8_t cc1101_interface_strobe(uint8_t); // prototype (to prevent warning) of internal driver function which is used here.
uint8_t cc1101_interface_write_single_reg(uint8_t, uint8_t);
uint8_t cc1101_interface_write_single_patable(uint8_t);
#elif defined USE_SI4460
// include private API which is not exported by cmake for 'normal' apps
#include "../../framework/hal/chips/si4460/ezradiodrv/inc/ezradio_cmd.h"
#include "../../framework/hal/chips/si4460/ezradiodrv/inc/ezradio_api_lib.h"
#include "../../framework/hal/chips/si4460/si4460_registers.h"
static ezradio_cmd_reply_t ezradioReply;
static bool radio_status = false;
static uint8_t current_eirp_level = 0x7f;
static uint8_t max_eirp_level = 0x7f;
static enum modulation{
    CW=0x18,
    FSK=0x12,
    GFSK=0x13
}Mod;
#endif

#define NORMAL_RATE_CHANNEL_COUNT 8
#define LO_RATE_CHANNEL_COUNT 69

static hw_rx_cfg_t rx_cfg;
static uint8_t current_channel_indexes_index = 0;
static phy_channel_band_t current_channel_band = PHY_BAND_868;
static phy_channel_class_t current_channel_class = PHY_CLASS_NORMAL_RATE;
static uint8_t channel_indexes[NORMAL_RATE_CHANNEL_COUNT] = { 0 }; // reallocated later depending on band/class
static uint8_t channel_count = NORMAL_RATE_CHANNEL_COUNT;


void rssi_valid_cb(int16_t rssi)
{
  // dummy, we are not staying in rx
}

void start()
{
    rx_cfg.channel_id.channel_header.ch_coding = PHY_CODING_PN9;
    rx_cfg.channel_id.channel_header.ch_class = current_channel_class;
    rx_cfg.channel_id.channel_header.ch_freq_band = current_channel_band;
    rx_cfg.channel_id.center_freq_index = channel_indexes[current_channel_indexes_index];

#ifdef HAS_LCD
    char string[10] = "";
    char rate;
    char band[3];
    switch(current_channel_class)
    {
        case PHY_CLASS_LO_RATE: rate = 'L'; break;
        case PHY_CLASS_NORMAL_RATE: rate = 'N'; break;
        case PHY_CLASS_HI_RATE: rate = 'H'; break;
    }

    switch(current_channel_band)
    {
        case PHY_BAND_433: strncpy(band, "433", sizeof(band)); break;
        case PHY_BAND_868: strncpy(band, "868", sizeof(band)); break;
        case PHY_BAND_915: strncpy(band, "915", sizeof(band)); break;
    }

    sprintf(string, "%.3s%c-%i", band, rate, rx_cfg.channel_id.center_freq_index),
    lcd_write_string(string);
#endif



#if defined USE_CC1101
    hw_radio_set_rx(&rx_cfg, NULL, &rssi_valid_cb); // we 'misuse' hw_radio_set_rx to configure the channel (using the public API)
    hw_radio_set_idle(); // go straight back to idle

    cc1101_interface_write_single_patable(0xc0); // 10dBm TX EIRP
    //cc1101_interface_write_single_reg(0x08, 0x22); // PKTCTRL0 random PN9 mode + disable data whitening
    cc1101_interface_write_single_reg(0x08, 0x22); // PKTCTRL0 disable data whitening, continious preamble
   // cc1101_interface_write_single_reg(0x12, 0x30); // MDMCFG2: use OOK modulation to clearly view centre freq on spectrum analyzer, comment for GFSK
    cc1101_interface_strobe(0x32); // strobe calibrate
    cc1101_interface_strobe(0x35); // strobe TX
#elif defined USE_SI4460
//    /* Request and check radio device state */
//    ezradio_request_device_state(&ezradioReply);
    /* Configure */
    configure_continuous_radio(GFSK);
    /* Request and check radio device state */
//    ezradio_request_device_state(&ezradioReply);
    /* Start the radio with the new configuration */
    start_radio();
    /* Request and check radio device state */
    ezradio_request_device_state(&ezradioReply);

#endif
}

#if NUM_USERBUTTONS > 1
void userbutton_callback(button_id_t button_id)
{
    switch(button_id)
    {
        case 0:
#if defined USE_SI4460
            // change ezr eirp and restart
            if(current_eirp_level < max_eirp_level+1)
                current_eirp_level -= 0x05;
            else
                current_eirp_level = max_eirp_level;
            configure_continuous_radio(GFSK);
#endif
            break;
        case 1:
            // change channel and restart
            if(current_channel_indexes_index < channel_count - 1)
                current_channel_indexes_index++;
            else
                current_channel_indexes_index = 0;
    }
    sched_post_task(&start);
}
#endif

#if defined USE_SI4460
void toggle_radio(){
    if(radio_status == true){
        stop_radio();
    }
    else{
        start_radio();
    }
}

void stop_radio(){
    // stop sending signal
    ezradio_change_state(EZRADIO_CMD_CHANGE_STATE_ARG_NEXT_STATE1_NEW_STATE_ENUM_READY);
    radio_status = false;
}

void start_radio(){
    /* Read ITs, clear pending ones */
    ezradio_get_int_status(0u, 0u, 0u, NULL);

    /* Start sending packet, channel 0, START immediately, Packet n bytes long, go READY when done */
    ezradio_start_tx(rx_cfg.channel_id.center_freq_index, 0u,  0u);
    radio_status = true;
}

void configure_continuous_radio(modulation){
    DPRINT("Change radio configuration");
//    ezradio_get_int_status(0x0, 0x0, 0x0, &ezradioReply);
//    ezradio_change_state(EZRADIO_CMD_CHANGE_STATE_ARG_NEXT_STATE1_NEW_STATE_ENUM_READY);

    /* Request and check radio device state */
    ezradio_request_device_state(&ezradioReply);

    // Si4460 Direct mode
    ezradio_set_property(0x20, 0x01, 0x00, modulation);//DIRECT PN9 + 2GFSK

    //power level EIRP
    ezradio_set_property(0x22, 0x01, 0x01, current_eirp_level);
}
#endif

void bootstrap()
{
    DPRINT("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later

#ifdef HAS_LCD
    lcd_write_string("cont tx");
#endif

    switch(current_channel_class)
    {
        // TODO only 433 for now
        case PHY_CLASS_NORMAL_RATE:
          channel_count = NORMAL_RATE_CHANNEL_COUNT;
            realloc(channel_indexes, channel_count);
            for(int i = 0; i < channel_count; i++)
                channel_indexes[i] = i * 8;

            break;
        case PHY_CLASS_LO_RATE:
          channel_count = LO_RATE_CHANNEL_COUNT;
            realloc(channel_indexes, channel_count);
            for(int i = 0; i < channel_count; i++)
                channel_indexes[i] = i;

            break;
    }

#if NUM_USERBUTTONS > 1
    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);
#endif

    hw_radio_init(NULL, NULL);

    sched_register_task(&start);
    timer_post_task_delay(&start, TIMER_TICKS_PER_SEC * 5);
}
