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

/*
 *  Created on: Mar 24, 2015
 *  Authors:
 *  	glenn.ergeerts@uantwerpen.be
 */


#include <string.h>
#include <stdio.h>

#include <hwleds.h>
#include <hwradio.h>
#include <log.h>
#include <hwwatchdog.h>

#ifdef HAS_LCD
#include "hwlcd.h"
#endif

// configuration options
//#define RX_MODE
#define PHY_CLASS PHY_CLASS_LO_RATE
#define PACKET_LENGTH 10



#ifdef FRAMEWORK_LOG_ENABLED
#ifdef HAS_LCD
		#define DPRINT(...) log_print_string(__VA_ARGS__); lcd_write_string(__VA_ARGS__)
	#else
		#define DPRINT(...) log_print_string(__VA_ARGS__)
	#endif

#else
	#ifdef HAS_LCD
		#define DPRINT(...) lcd_write_string(__VA_ARGS__)
	#else
		#define DPRINT(...)
	#endif
#endif



hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .channel_header.ch_coding = PHY_CODING_FEC_PN9,
        .channel_header.ch_class = PHY_CLASS,
#ifdef PLATFORM_EZR32LG_WSTK6200A
        .channel_header.ch_freq_band = PHY_BAND_868,
#else
        .channel_header.ch_freq_band = PHY_BAND_433,
#endif
        .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

hw_tx_cfg_t tx_cfg = {
    .channel_id = {
        .channel_header.ch_coding = PHY_CODING_FEC_PN9,
        .channel_header.ch_class = PHY_CLASS,
#ifdef PLATFORM_EZR32LG_WSTK6200A
        .channel_header.ch_freq_band = PHY_BAND_868,
#else
        .channel_header.ch_freq_band = PHY_BAND_433,
#endif

        .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS0,
    .eirp = 10
};

static uint8_t tx_buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
static uint8_t rx_buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
hw_radio_packet_t* tx_packet = (hw_radio_packet_t*)tx_buffer;
hw_radio_packet_t* rx_packet = (hw_radio_packet_t*)rx_buffer;
static uint8_t data[256];

hw_radio_packet_t received_packet;

static uint8_t counter = 0;

void packet_received(hw_radio_packet_t* packet);
void packet_transmitted(hw_radio_packet_t* packet);


void start_rx()
{
    DPRINT("start RX\n");
    hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
}

void transmit_packet()
{
	counter++;
    DPRINT("%d tx %d bytes\n", counter, PACKET_LENGTH);
    memcpy(&tx_packet->data, data, PACKET_LENGTH);
    tx_packet->length = PACKET_LENGTH;
    hw_radio_send_packet(tx_packet, &packet_transmitted);
}

hw_radio_packet_t* alloc_new_packet(uint8_t length)
{
    return rx_packet;
}

void release_packet(hw_radio_packet_t* packet)
{
    memset(rx_buffer, 0, sizeof(hw_radio_packet_t) + 255);
}

void packet_received(hw_radio_packet_t* packet)
{
    DPRINT("packet received @ %i , RSSI = %i\n", packet->rx_meta.timestamp, packet->rx_meta.rssi);
#ifdef HAL_RADIO_USE_HW_CRC
    int cmp = memcmp(data, packet->data, packet->length-2);
#else
		int cmp = memcmp(data, packet->data, packet->length);
#endif
    if(cmp != 0)
    {
        DPRINT("Unexpected data received! %d\n", cmp);
    }

    hw_watchdog_feed();

    memset(packet->data, 0, packet->length);
}

void packet_transmitted(hw_radio_packet_t* packet)
{
#if HW_NUM_LEDS > 0
    led_toggle(0);
#endif
    DPRINT("%d tx ok\n", counter);
    timer_post_task(&transmit_packet, 1000);

    hw_watchdog_feed();
}

void bootstrap()
{
    DPRINT("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later

    data[0] = PACKET_LENGTH-1;
    int i = 1;
    for (;i<PACKET_LENGTH;i++)
    	data[i] = i;

    hw_radio_init(&alloc_new_packet, &release_packet);

    tx_packet->tx_meta.tx_cfg = tx_cfg;

	#ifdef RX_MODE
    	sched_register_task(&start_rx);
        sched_post_task(&start_rx);
	#else
        sched_register_task(&transmit_packet);
        sched_post_task(&transmit_packet);
	#endif
}
