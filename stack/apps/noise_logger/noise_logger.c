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

hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .channel_header.ch_coding = PHY_CODING_PN9,
        .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
        .channel_header.ch_freq_band = PHY_BAND_433,
        .center_freq_index = 5
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

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
    if(rssi_measurements_index < 1000)
        timer_post_task_delay(&read_rssi, 1); // TODO delay
    else
        log_print_string("done");
}

void rssi_valid(int16_t cur_rssi)
{
    timer_post_task_delay(&read_rssi, 1); // TODO delay
}

void start_rx()
{
    log_print_string("start RX");
    hw_radio_set_rx(&rx_cfg, NULL, &rssi_valid);
}

void bootstrap()
{
    hw_radio_init(NULL, NULL);

    sched_register_task(&read_rssi);
    sched_register_task(&start_rx);
    sched_post_task(&start_rx);
}
