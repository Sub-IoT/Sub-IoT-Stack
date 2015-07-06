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
#include <fifo.h>
#include <assert.h>

#define NORMAL_RATE_CHANNEL_COUNT 8
#define LO_RATE_CHANNEL_COUNT 69

#define COMMAND_SIZE 4
#define COMMAND_CHAN "CHAN"
#define COMMAND_CHAN_PARAM_SIZE 7 + 1 // (including \n)

#define UART_RX_BUFFER_SIZE 20

static uint8_t current_channel_indexes_index = 0;
static phy_channel_band_t current_channel_band = PHY_BAND_433;
static phy_channel_class_t current_channel_class = PHY_CLASS_NORMAL_RATE;
static uint8_t channel_indexes[LO_RATE_CHANNEL_COUNT] = { 0 }; // reallocated later depending on band/class
static uint8_t channel_count = LO_RATE_CHANNEL_COUNT;
static bool use_manual_channel_switching = false;
static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = { 0 };
static fifo_t uart_rx_fifo;
static hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .channel_header.ch_coding = PHY_CODING_PN9,
        .channel_header.ch_class = PHY_BAND_433,
        .channel_header.ch_freq_band = PHY_CLASS_NORMAL_RATE,
        .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

typedef struct
{
    timer_tick_t tick;
    int16_t rssi;
} timestamped_rssi_t;

void start_rx();

static void switch_prev_channel()
{
    if(current_channel_indexes_index > 0)
        current_channel_indexes_index--;
    else
        current_channel_indexes_index = channel_count - 1;

    rx_cfg.channel_id.center_freq_index = channel_indexes[current_channel_indexes_index];
}

static void switch_next_channel()
{
    if(current_channel_indexes_index < channel_count - 1)
        current_channel_indexes_index++;
    else
        current_channel_indexes_index = 0;

    rx_cfg.channel_id.center_freq_index = channel_indexes[current_channel_indexes_index];
}


void process_command_chan()
{
    while(fifo_get_size(&uart_rx_fifo) < COMMAND_CHAN_PARAM_SIZE);

    char param[COMMAND_CHAN_PARAM_SIZE];
    fifo_pop(&uart_rx_fifo, param, COMMAND_CHAN_PARAM_SIZE);

    if(strncmp(param, "433", 3) == 0)
        rx_cfg.channel_id.channel_header.ch_freq_band = PHY_BAND_433;
    else if(strncmp(param, "868", 3) == 0)
        rx_cfg.channel_id.channel_header.ch_freq_band = PHY_BAND_868;
    else if(strncmp(param, "915", 3) == 0)
        rx_cfg.channel_id.channel_header.ch_freq_band = PHY_BAND_915;
    else
        goto error;

    char channel_class = param[3];
    if(channel_class == 'L')
        rx_cfg.channel_id.channel_header.ch_class = PHY_CLASS_LO_RATE;
    else if(channel_class == 'N')
        rx_cfg.channel_id.channel_header.ch_class = PHY_CLASS_NORMAL_RATE;
    else if(channel_class == 'H')
        rx_cfg.channel_id.channel_header.ch_class = PHY_CLASS_HI_RATE;
    else
        goto error;

    uint16_t center_freq_index = atoi((const char*)(param + 5));
    rx_cfg.channel_id.center_freq_index = center_freq_index;
    // TODO validate

    // change channel and restart
    sched_post_task(&start_rx);
    return;

    error:
        uart_transmit_string("Error parsing CHAN command. Expected format example: '433L001'\n");
        fifo_clear(&uart_rx_fifo);
}

void process_uart_rx_fifo()
{
    int16_t size = fifo_get_size(&uart_rx_fifo);

    if(size < COMMAND_SIZE)
        return;

    uint8_t received_cmd[COMMAND_SIZE];
    fifo_pop(&uart_rx_fifo, received_cmd, COMMAND_SIZE);
    if(memcmp(received_cmd, COMMAND_CHAN, COMMAND_SIZE) == 0)
    {
        process_command_chan();
    }
    else
    {
        assert(false);
    }
}

static void channel_id_to_string(channel_id_t* channel, char* str, size_t len)
{
    char rate;
    char band[3];
    switch(channel->channel_header.ch_class)
    {
        case PHY_CLASS_LO_RATE: rate = 'L'; break;
        case PHY_CLASS_NORMAL_RATE: rate = 'N'; break;
        case PHY_CLASS_HI_RATE: rate = 'H'; break;
    }

    switch(channel->channel_header.ch_freq_band)
    {
        case PHY_BAND_433: strncpy(band, "433", sizeof(band)); break;
        case PHY_BAND_868: strncpy(band, "868", sizeof(band)); break;
        case PHY_BAND_915: strncpy(band, "915", sizeof(band)); break;
    }

    snprintf(str, len, "%.3s%c%03i", band, rate, channel->center_freq_index);
}

void read_rssi()
{
    timestamped_rssi_t rssi_measurement;
    rssi_measurement.tick = timer_get_counter_value();
    rssi_measurement.rssi = hw_radio_get_rssi();
    char str[80];
    char channel_str[8] = "";
    channel_id_to_string(&rx_cfg.channel_id, channel_str, sizeof(channel_str));
    lcd_write_string(channel_str);
    sprintf(str, "%7s,%i,%i\n", channel_str, rssi_measurement.tick, rssi_measurement.rssi);
    uart_transmit_string(str);
    if(!use_manual_channel_switching)
    {
        switch_next_channel();
        sched_post_task(&start_rx);
    }
    else
    {
    	sched_post_task(&process_uart_rx_fifo); // check for UART commands first
        timer_post_task_delay(&read_rssi, TIMER_TICKS_PER_SEC * 2);
    }

}

void rssi_valid(int16_t cur_rssi)
{
    sched_post_task(&read_rssi);
}

void start_rx()
{
    log_print_string("start RX");

#ifdef HAS_LCD
    char channel_str[10] = "";
    channel_id_to_string(&rx_cfg.channel_id, channel_str, sizeof(channel_str));
    lcd_write_string(channel_str);
#endif
    hw_radio_set_rx(&rx_cfg, NULL, &rssi_valid);
}

#if NUM_USERBUTTONS > 1
void userbutton_callback(button_id_t button_id)
{
    // change channel and restart
    use_manual_channel_switching = true;
    switch(button_id)
    {
        case 0: switch_prev_channel(); break;
        case 1: switch_next_channel(); break;
    }

    sched_post_task(&start_rx);
}
#endif

void uart_rx_cb(char data)
{
    error_t err;
    err = fifo_put(&uart_rx_fifo, &data, 1); assert(err == SUCCESS);
    // parse command in read_rssi() task
    use_manual_channel_switching = true;
}

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

    fifo_init(&uart_rx_fifo, uart_rx_buffer, sizeof(uart_rx_buffer));

    uart_set_rx_interrupt_callback(&uart_rx_cb);
    uart_rx_interrupt_enable(true);

    sched_register_task(&read_rssi);
    sched_register_task(&start_rx);
    sched_register_task(&process_uart_rx_fifo);
    timer_post_task_delay(&start_rx, TIMER_TICKS_PER_SEC * 3);
}
