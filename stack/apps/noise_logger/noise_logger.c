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
 *  Created on: Apr 7, 2015
 *  Authors:
 *  	glenn.ergeerts@uantwerpen.be
 */

#include <hwradio.h>
#include <log.h>
#include <timer.h>
#include <hwlcd.h>
#include <hwuart.h>
#include <stdio.h>
#include <stdlib.h>
#include <userbutton.h>

#define NORMAL_RATE_CHANNEL_COUNT 8
#define LO_RATE_CHANNEL_COUNT 69

static uint8_t current_channel_indexes_index = 0;
static phy_channel_band_t current_channel_band = PHY_BAND_433;
static phy_channel_class_t current_channel_class = PHY_CLASS_LO_RATE;
static uint8_t channel_indexes[LO_RATE_CHANNEL_COUNT] = { 0 }; // reallocated later depending on band/class
static uint8_t channel_count = LO_RATE_CHANNEL_COUNT;

typedef struct
{
    timer_tick_t tick;
    int16_t rssi;
} timestamped_rssi_t;

timestamped_rssi_t rssi_measurements[4096] = { 0 }; // TODO tmp
int16_t rssi_measurements_index = 0;

void read_rssi()
{
    timestamped_rssi_t rssi_measurement;
    rssi_measurement.tick = timer_get_counter_value();
    rssi_measurement.rssi = hw_radio_get_rssi();
    rssi_measurements[rssi_measurements_index] = rssi_measurement;
    rssi_measurements_index++;
    char str[80];
    sprintf(str, "%i,%i,%i\n", rssi_measurements_index, rssi_measurement.tick, rssi_measurement.rssi); // TODO channel
    uart_transmit_message(str, strlen(str));
    if(rssi_measurements_index < 1000)
        //timer_post_task_delay(&read_rssi, 5); // TODO delay
    	sched_post_task(&read_rssi); // TODO delay
    else
        log_print_string("done");
}

void rssi_valid(int16_t cur_rssi)
{
    sched_post_task(&read_rssi);
}

void start_rx()
{
    log_print_string("start RX");

    hw_rx_cfg_t rx_cfg;
    rx_cfg.channel_id.channel_header.ch_coding = PHY_CODING_PN9;
    rx_cfg.channel_id.channel_header.ch_class = current_channel_class;
    rx_cfg.channel_id.channel_header.ch_freq_band = PHY_BAND_433;
    rx_cfg.channel_id.center_freq_index = channel_indexes[current_channel_indexes_index];
    rx_cfg.syncword_class = PHY_SYNCWORD_CLASS0;

#ifdef HAS_LCD
    char string[10] = "";
    char rate;
    char band[3];
    switch(rx_cfg.channel_id.channel_header.ch_class)
    {
        case PHY_CLASS_LO_RATE: rate = 'L'; break;
        case PHY_CLASS_NORMAL_RATE: rate = 'N'; break;
        case PHY_CLASS_HI_RATE: rate = 'H'; break;
    }

    switch(rx_cfg.channel_id.channel_header.ch_freq_band)
    {
        case PHY_BAND_433: strncpy(band, "433", sizeof(band)); break;
        case PHY_BAND_868: strncpy(band, "868", sizeof(band)); break;
        case PHY_BAND_915: strncpy(band, "915", sizeof(band)); break;
    }

    sprintf(string, "%.3s%c-%i", band, rate, rx_cfg.channel_id.center_freq_index),
    lcd_write_string(string);
#endif
    hw_radio_set_rx(&rx_cfg, NULL, &rssi_valid);
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
                current_channel_indexes_index = channel_count - 1;
            break;
        case 1:
            if(current_channel_indexes_index < channel_count - 1)
                current_channel_indexes_index++;
            else
                current_channel_indexes_index = 0;
    }

    sched_post_task(&start_rx);
}
#endif

void bootstrap()
{
#ifdef HAS_LCD
    lcd_write_string("NOISE");
#endif

#if NUM_USERBUTTONS > 1
    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);
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

    hw_radio_init(NULL, NULL);

    sched_register_task(&read_rssi);
    sched_register_task(&start_rx);
    timer_post_task_delay(&start_rx, TIMER_TICKS_PER_SEC * 3);
}
