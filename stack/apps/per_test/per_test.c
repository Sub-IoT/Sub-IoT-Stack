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
#include <hwsystem.h>
#include <hwuart.h>
#include <log.h>
#include <crc.h>
#include <assert.h>
#include "hwlcd.h"
#include "platform_lcd.h"
#include "userbutton.h"


#ifndef PLATFORM_EFM32GG_STK3700
    #error "assuming STK3700 for now"
#endif

// configuration options
#define PACKET_SIZE 16

#ifdef FRAMEWORK_LOG_ENABLED
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

typedef enum
{
    STATE_CONFIG_DIRECTION,
    STATE_CONFIG_DATARATE,
    STATE_RUNNING
} state_t;


hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .channel_header.ch_coding = PHY_CODING_PN9,
        .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
        .channel_header.ch_freq_band = PHY_BAND_433,
        .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

hw_tx_cfg_t tx_cfg = {
    .channel_id = {
        .channel_header.ch_coding = PHY_CODING_PN9,
        .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
        .channel_header.ch_freq_band = PHY_BAND_433,
        .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS0,
    .eirp = 10
};

static uint8_t tx_buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
static uint8_t rx_buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
hw_radio_packet_t* tx_packet = (hw_radio_packet_t*)tx_buffer;
hw_radio_packet_t* rx_packet = (hw_radio_packet_t*)rx_buffer;
static uint8_t data[PACKET_SIZE + 1] = { [0] = PACKET_SIZE, [1 ... PACKET_SIZE]  = 0 };
static uint16_t counter = 0;
static uint16_t missed_packets_counter = 0;
static uint16_t received_packets_counter = 0;
static hw_radio_packet_t received_packet;
static char lcd_msg[15];
static char record[80];
static uint64_t id;
static bool is_mode_rx = true;
static phy_channel_class_t current_channel_class = PHY_CLASS_NORMAL_RATE;
static state_t current_state = STATE_CONFIG_DIRECTION;

void packet_received(hw_radio_packet_t* packet);
void packet_transmitted(hw_radio_packet_t* packet);
void start();

void start_rx()
{
    DPRINT("start RX");
    current_state = STATE_RUNNING;
    hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
}

void transmit_packet()
{
    DPRINT("transmitting packet");
    current_state = STATE_RUNNING;
    counter++;
    memcpy(data + 1, &id, sizeof(id));
    memcpy(data + 1 + sizeof(id), &counter, sizeof(counter));
    uint16_t crc = __builtin_bswap16(crc_calculate(data, sizeof(data) - 2));
    memcpy(data + sizeof(data) - 2, &crc, 2);
    memcpy(&tx_packet->data, data, sizeof(data));
    tx_packet->tx_meta.tx_cfg = tx_cfg;
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
		#if HW_NUM_LEDS > 0
			led_toggle(0);
		#endif
		uint16_t msg_counter = 0;
        uint64_t msg_id;
        memcpy(&msg_id, packet->data + 1, sizeof(msg_id));
        memcpy(&msg_counter, packet->data + 1 + sizeof(msg_id), sizeof(msg_counter));

        sprintf(record, "%i,%i,%lu,%lu,%i\n", msg_counter, packet->rx_meta.rssi, (unsigned long)msg_id, (unsigned long)id, packet->rx_meta.timestamp);
		uart_transmit_message(record, strlen(record));

		if(counter == 0)
		{
			// just start, assume received all previous counters to reset PER to 0%
			received_packets_counter = msg_counter - 1;
			counter = msg_counter - 1;
		}

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
            sched_post_task(&start);
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

void stop()
{
    // make sure to cancel tasks which might me pending already
	if(is_mode_rx)
		sched_cancel_task(&start_rx);
	else
		sched_cancel_task(&transmit_packet);

    hw_radio_set_idle();
}

void start()
{
    counter = 0;
    missed_packets_counter = 0;
    received_packets_counter = 0;

    switch(current_state)
    {
        case STATE_CONFIG_DIRECTION:
            is_mode_rx? sprintf(lcd_msg, "PER RX") : sprintf(lcd_msg, "PER TX");
            break;
        case STATE_CONFIG_DATARATE:
            switch(current_channel_class)
            {
                case PHY_CLASS_LO_RATE:
                    sprintf(lcd_msg, "LO-RATE");
                    break;
                case PHY_CLASS_NORMAL_RATE:
                    sprintf(lcd_msg, "NOR-RATE");
                    break;
                case PHY_CLASS_HI_RATE:
                    sprintf(lcd_msg, "HI-RATE");
                    break;
            }
            break;
        case STATE_RUNNING:
            if(is_mode_rx)
            {
                sprintf(lcd_msg, "RUN RX");
                lcd_write_string(lcd_msg);
                sprintf(record, "%s,%s,%s,%s,%s\n", "counter", "rssi", "tx_id", "rx_id", "timestamp");
                uart_transmit_message(record, strlen(record));
                rx_cfg.channel_id.channel_header.ch_class = current_channel_class;
                timer_post_task(&start_rx, TIMER_TICKS_PER_SEC * 5);
            }
            else
            {
                hw_radio_set_idle();
                sprintf(lcd_msg, "RUN TX");
                lcd_write_string(lcd_msg);
                timer_post_task(&transmit_packet, TIMER_TICKS_PER_SEC * 5);
            }
            break;
    }

    lcd_write_string(lcd_msg);
}

void userbutton_callback(button_id_t button_id)
{
	stop();
    switch(button_id)
    {
        case 0:
            switch(current_state)
            {
                case STATE_CONFIG_DIRECTION:
                    current_state = STATE_CONFIG_DATARATE;
                    break;
                case STATE_CONFIG_DATARATE:
                    current_state = STATE_RUNNING;
                    break;
                case STATE_RUNNING:
                    current_state = STATE_CONFIG_DIRECTION;
                    break;
            }
            break;
        case 1:
            switch(current_state)
            {
                case STATE_CONFIG_DIRECTION:
                    is_mode_rx = !is_mode_rx;
                    break;
                case STATE_CONFIG_DATARATE:
                    if(current_channel_class == PHY_CLASS_NORMAL_RATE)
                        current_channel_class = PHY_CLASS_LO_RATE;
                    else
                        current_channel_class = PHY_CLASS_NORMAL_RATE;

                    tx_cfg.channel_id.channel_header.ch_class = current_channel_class;
                    rx_cfg.channel_id.channel_header.ch_class = current_channel_class;
                    break;
            }
            break;
    }

    sched_post_task(&start);
}

void bootstrap()
{
    DPRINT("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later
    id = hw_get_unique_id();
    hw_radio_init(&alloc_new_packet, &release_packet);

    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);

    sched_register_task(&start_rx);
    sched_register_task(&transmit_packet);
    sched_register_task(&start);

    current_state = STATE_CONFIG_DIRECTION;
    current_channel_class = PHY_CLASS_NORMAL_RATE;

    sched_post_task(&start);
}
