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

//#define RX_MODE

hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .ch_coding = PHY_CODING_PN9,
        .ch_class = PHY_CLASS_NORMAL_RATE,
        .ch_freq_band = PHY_BAND_433,
        .center_freq_index = 5
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

hw_tx_cfg_t tx_cfg = {
    .channel_id = {
        .ch_coding = PHY_CODING_PN9,
        .ch_class = PHY_CLASS_NORMAL_RATE,
        .ch_freq_band = PHY_BAND_433,
        .center_freq_index = 5
    },
    .syncword_class = PHY_SYNCWORD_CLASS0,
    .eirp = 10
};

static uint8_t tx_buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
static uint8_t rx_buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
hw_radio_packet_t* tx_packet = (hw_radio_packet_t*)tx_buffer;
hw_radio_packet_t* rx_packet = (hw_radio_packet_t*)rx_buffer;
static uint8_t data[] = {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};


hw_radio_packet_t received_packet;

void packet_received(hw_radio_packet_t* packet);
void packet_transmitted(hw_radio_packet_t* packet);

void start_rx()
{
    log_print_string("start RX");
    hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
}

void transmit_packet()
{
    log_print_string("transmitting packet");
    memcpy(&tx_packet->data, data, sizeof(data));
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
    log_print_string("packet received @ %i , RSSI = %i", packet->rx_meta.timestamp, packet->rx_meta.rssi);
    if(memcmp(data, packet->data, sizeof(data)) != 0)
        log_print_string("Unexpected data received!");
}

void packet_transmitted(hw_radio_packet_t* packet)
{
    led_toggle(0);
    log_print_string("packet transmitted");
    timer_post_task(&transmit_packet, 100);
}

void bootstrap()
{
    log_print_string("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later

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
