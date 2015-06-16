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
 *  Created on: Jun 16, 2015
 *  Authors:
 *  	glenn.ergeerts@uantwerpen.be
 */


#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <hwleds.h>
#include <hwradio.h>
#include <log.h>
#include <crc.h>
#include <assert.h>
#include "hwlcd.h"
#include "platform_lcd.h"


// configuration options
#define RX_MODE
#define PHY_CLASS PHY_CLASS_LO_RATE


#ifdef FRAMEWORK_LOG_ENABLED
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif



hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .channel_header.ch_coding = PHY_CODING_PN9,
        .channel_header.ch_class = PHY_CLASS,
        .channel_header.ch_freq_band = PHY_BAND_433,
        .center_freq_index = 5
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

hw_tx_cfg_t tx_cfg = {
    .channel_id = {
        .channel_header.ch_coding = PHY_CODING_PN9,
        .channel_header.ch_class = PHY_CLASS,
        .channel_header.ch_freq_band = PHY_BAND_433,
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
static uint16_t counter = 0;
static uint16_t missed_packets_counter = 0;
static uint16_t received_packets_counter = 0;
hw_radio_packet_t received_packet;
char lcd_msg[15];
char record[15];

void packet_received(hw_radio_packet_t* packet);
void packet_transmitted(hw_radio_packet_t* packet);

void start_rx()
{
    DPRINT("start RX");
    hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
}

void transmit_packet()
{
    DPRINT("transmitting packet");
    counter++;
    memcpy(data + 1, &counter, sizeof(counter));
    uint16_t crc = __builtin_bswap16(crc_calculate(data, sizeof(data) - 2));
    memcpy(data + sizeof(data) - 2, &crc, 2);
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
    uint16_t crc = __builtin_bswap16(crc_calculate(packet->data, packet->length + 1 - 2));
    if(memcmp(&crc, packet->data + packet->length + 1 - 2, 2) != 0)
    {
        DPRINT("CRC error");
        missed_packets_counter++;
    }
    else
    {
		uint16_t msg_counter = 0;
		memcpy(&msg_counter, packet->data + 1, sizeof(msg_counter));

		sprintf(record, "%i,%i,%i\n", msg_counter, packet->rx_meta.rssi, packet->rx_meta.timestamp);
		uart_transmit_message(record, strlen(record));

		uint16_t expected_counter = counter + 1;
		if(msg_counter == expected_counter)
		{
			received_packets_counter++;
			counter++;
		}
		else if(msg_counter > expected_counter)
		{
			missed_packets_counter += msg_counter - expected_counter;
			counter = msg_counter;
		}
		else
		{
			lcd_write_string("restart");
			while(1);
		}

	    double per = 0;
	    if(msg_counter > 0)
	        per = 100.0 - ((double)received_packets_counter / (double)msg_counter) * 100.0;

	    sprintf(lcd_msg, "%i %i", (int)per, packet->rx_meta.rssi);
	    lcd_write_string(lcd_msg);
    }
}

void packet_transmitted(hw_radio_packet_t* packet)
{
#if HW_NUM_LEDS > 0
    led_toggle(0);
#endif
    DPRINT("packet transmitted");
    sprintf(lcd_msg, "TX %i", counter);
    lcd_write_string(lcd_msg);
    timer_post_task(&transmit_packet, TIMER_TICKS_PER_SEC / 5);
}

void bootstrap()
{
    DPRINT("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later

    hw_radio_init(&alloc_new_packet, &release_packet);

    tx_packet->tx_meta.tx_cfg = tx_cfg;

	#ifdef RX_MODE
        sprintf(lcd_msg, "PER RX");
        lcd_write_string(lcd_msg);
    	sched_register_task(&start_rx);
        sched_post_task(&start_rx);
	#else
        sprintf(lcd_msg, "PER TX");
        lcd_write_string(lcd_msg);
        sched_register_task(&transmit_packet);
        timer_post_task(&transmit_packet, TIMER_TICKS_PER_SEC * 10);
	#endif
}
