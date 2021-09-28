/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
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
#include "hwsystem.h"
#include "platform_defs.h"
#include "debug.h"

#if !defined (USE_CC1101) && !defined (USE_SI4460) && !defined (USE_SX127X)
    #error "This application only works with cc1101, si4460 or sx127x radios"
#endif

#include "log.h"
#include "hwradio.h"

#if PLATFORM_NUM_BUTTONS > 1
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
  #include "../../framework/hal/chips/cc1101/cc1101_constants.h"
  uint8_t cc1101_interface_strobe(uint8_t); // prototype (to prevent warning) of internal driver function which is used here.
  uint8_t cc1101_interface_write_single_reg(uint8_t, uint8_t);
  uint8_t cc1101_interface_write_single_patable(uint8_t);
  #define DEFAULT_EIRP 0xC0
  #define MAX_EIRP 0xC0 //868 MHz = +12 dBm , 433 MHz = +10 dBm

#elif defined USE_SI4460
  // include private API which is not exported by cmake for 'normal' apps
  #include "../../framework/hal/chips/si4460/ezradiodrv/inc/ezradio_cmd.h"
  #include "../../framework/hal/chips/si4460/ezradiodrv/inc/ezradio_api_lib.h"
  #include "../../framework/hal/chips/si4460/si4460_registers.h"
  #include "../../framework/hal/chips/si4460/si4460.h"
  #include "../../framework/hal/chips/si4460/si4460_interface.h"
  static ezradio_cmd_reply_t ezradioReply;
  #define DEFAULT_EIRP 0x7f
  #define MAX_EIRP 0x7f // 868 MHz = +13 dBm
#elif defined USE_SX127X
  #define DEFAULT_EIRP 0 // TODO
  #define MAX_EIRP 0 // TODO
#endif

#define HI_RATE_CHANNEL_COUNT 32
#define NORMAL_RATE_CHANNEL_COUNT 32
#define LO_RATE_CHANNEL_COUNT 280

static hw_tx_cfg_t tx_cfg;
static uint16_t current_channel_indexes_index = 13; //108
static phy_channel_band_t current_channel_band = PHY_BAND_868;
static phy_channel_class_t current_channel_class = PHY_CLASS_NORMAL_RATE; 
static uint16_t channel_indexes[LO_RATE_CHANNEL_COUNT] = { 0 }; // reallocated later depending on band/class
static uint16_t channel_count = LO_RATE_CHANNEL_COUNT;
static uint8_t current_eirp_level = DEFAULT_EIRP;
static phy_coding_t current_coding = PHY_CODING_RFU;

void stop_radio(){
#if defined USE_SI4460
    // stop sending signal
    ezradio_change_state(EZRADIO_CMD_CHANGE_STATE_ARG_NEXT_STATE1_NEW_STATE_ENUM_READY);
#elif defined USE_CC1101
    cc1101_interface_strobe(RF_SIDLE);
#endif
}

void start_radio(){
#if defined USE_SI4460
#ifdef HAS_LCD
    lcd_write_string("Start sending \n");
#endif
#elif defined USE_CC1101
    cc1101_interface_strobe(RF_SCAL);
    cc1101_interface_strobe(RF_STX);
#endif
}

void configure_radio(){
#if defined USE_SI4460 || defined USE_SX127X
    hw_radio_continuous_tx(&tx_cfg, 0); //time of 0 is send unlimited

#elif defined USE_CC1101

    hw_radio_set_rx(&tx_cfg, NULL, NULL); // we 'misuse' hw_radio_set_rx to configure the channel (using the public API)
    hw_radio_set_idle(); // go straight back to idle

    /* Configure */
    cc1101_interface_write_single_patable(current_eirp_level);
    //cc1101_interface_write_single_reg(0x08, 0x22); // PKTCTRL0 random PN9 mode + disable data whitening
    cc1101_interface_write_single_reg(0x08, 0x22); // PKTCTRL0 disable data whitening, continious preamble
    if(tx_cfg.channel_id.channel_header.ch_coding == PHY_CODING_CW) {
      cc1101_interface_write_single_reg(0x12, 0x30); // MDMCFG2
    } else {
      cc1101_interface_write_single_reg(0x12, 0x12); // MDMCFG2
    }

    cc1101_interface_strobe(0x32); // strobe calibrate
#endif
}

void change_eirp(){
#if defined USE_SI4460
    ezradio_set_property(0x22, 0x01, 0x01, current_eirp_level);
#elif defined USE_CC1101
    cc1101_interface_write_single_patable(current_eirp_level);
#endif
#ifdef HAS_LCD
    char string[10] = "";
    sprintf(string, "ptx %3x", current_eirp_level);
    lcd_write_string(string);
#endif
}

void start()
{
    tx_cfg.channel_id.channel_header.ch_coding = current_coding;
    tx_cfg.channel_id.channel_header.ch_class = current_channel_class;
    tx_cfg.channel_id.channel_header.ch_freq_band = current_channel_band;
    tx_cfg.channel_id.center_freq_index = channel_indexes[current_channel_indexes_index];
    tx_cfg.eirp = 10;

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

    sprintf(string, "%.3s%c-%i\n", band, rate, tx_cfg.channel_id.center_freq_index),
    lcd_write_string(string);
#endif

    /* Configure */
    DPRINT("configure_radio\n");
    configure_radio();

    /* start the radio */
    DPRINT("start_radio\n");
    start_radio();

}

#if PLATFORM_NUM_BUTTONS > 1
void userbutton_callback(button_id_t button_id)
{
    switch(button_id)
    {
        case 0:
            // TODO switch to values in dBm and use API to change instead of directly changing reg values
            // change ezr eirp and restart
            if(current_eirp_level < MAX_EIRP+1)
                current_eirp_level -= 0x05;
            else
                current_eirp_level = MAX_EIRP;
            //change eirp level
            change_eirp();
            break;
        case 1:
            // change channel and restart
            if(current_channel_indexes_index < channel_count - 1)
                current_channel_indexes_index++;
            else
                current_channel_indexes_index = 0;
            sched_post_task(&start);
    }
}
#endif


// packet callbacks only here to make hwradio_init() happy, not used
hw_radio_packet_t* alloc_packet_callback(uint8_t length) {
  assert(false);
}

void release_packet_callback(hw_radio_packet_t* p) {
  assert(false);
}

void bootstrap()
{
    DPRINT("Device booted at time: %d\n", timer_get_counter_value()); 

#ifdef HAS_LCD
    lcd_write_string("cont TX \n");
#endif

    DPRINT("Initializing channels\n");

    int16_t i = 0;
    switch(current_channel_class)
    {
        case PHY_CLASS_LO_RATE:
          channel_count = LO_RATE_CHANNEL_COUNT;
            realloc(channel_indexes, channel_count);
            
            for(; i < channel_count; i++)
                channel_indexes[i] = i;

            break;
        case PHY_CLASS_NORMAL_RATE:
          channel_count = NORMAL_RATE_CHANNEL_COUNT;
            realloc(channel_indexes, channel_count);
            
            for(; i < channel_count-4; i++)
                channel_indexes[i] = i*8;
            channel_indexes[i++]=229;
            channel_indexes[i++]=239;
            channel_indexes[i++]=257;
            channel_indexes[i++]=270;

            break;
        case PHY_CLASS_HI_RATE:
          channel_count = HI_RATE_CHANNEL_COUNT;
            realloc(channel_indexes, channel_count);

            for(; i < channel_count-4; i++)
                channel_indexes[i] = i*8;
            channel_indexes[i++]=229;
            channel_indexes[i++]=239;
            channel_indexes[i++]=257;
            channel_indexes[i++]=270; 
    }

#if PLATFORM_NUM_BUTTONS > 1
    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);
#endif

    DPRINT("Init radio\n");
    hw_radio_init(&alloc_packet_callback, &release_packet_callback);

    DPRINT("start\n");
    sched_register_task(&start);
    timer_post_task_delay(&start, 500);
}
