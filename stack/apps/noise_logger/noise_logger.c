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
#include <hwsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <userbutton.h>
#include <fifo.h>
#include <debug.h>
#include <platform_sensors.h>
#include <hwwatchdog.h>

#ifdef PLATFORM_EFM32GG_STK3700
#include "platform_lcd.h"
#endif

#include "console.h"

#define NORMAL_RATE_CHANNEL_COUNT_433 8
#define LO_RATE_CHANNEL_COUNT_433 69
#define NORMAL_RATE_CHANNEL_COUNT_868 35 // TODO not correct according to spec, spec is leaving out channels near the end of the band (not sure why)
#define LO_RATE_CHANNEL_COUNT_868 280

#define COMMAND_SIZE 4
#define COMMAND_CHAN "CHAN"
#define COMMAND_CHAN_PARAM_SIZE 7
#define COMMAND_LOOP "LOOP"
#define COMMAND_RSET "RSET"
#define UART_RX_BUFFER_SIZE 20
#define TEMPERATURE_TAG "TEMP"
#define TEMPERATURE_PERIOD TIMER_TICKS_PER_SEC * 10

#define RX_MAX_WINDOW 50
#define RSSI_SAMPLES_PER_MEASUREMENT 10

typedef struct
{
    timer_tick_t tick;
    int16_t rssi[RSSI_SAMPLES_PER_MEASUREMENT];
} timestamped_rssi_t;

static uint8_t current_channel_indexes_index = 0;
static uint16_t channel_indexes[LO_RATE_CHANNEL_COUNT_868] = { 0 }; // reallocated later depending on band/class
static uint16_t channel_count = LO_RATE_CHANNEL_COUNT_868;
static bool use_manual_channel_switching = false;
static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = { 0 };
static fifo_t uart_rx_fifo;
static uint16_t rx_measurement_counter = 0;
static timestamped_rssi_t rx_measurement_max = { 0, -200};

static hw_rx_cfg_t rx_cfg = {
    .channel_id = {
        .channel_header.ch_coding = PHY_CODING_PN9,
        .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
        .channel_header.ch_freq_band = PHY_BAND_868,
        .center_freq_index = 0
    },
    .syncword_class = PHY_SYNCWORD_CLASS0
};

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

static void prepare_channel_indexes()
{
    if(rx_cfg.channel_id.channel_header.ch_class == PHY_CLASS_NORMAL_RATE)
    {
        if(rx_cfg.channel_id.channel_header.ch_freq_band == PHY_BAND_433)
        {
            channel_count = NORMAL_RATE_CHANNEL_COUNT_433;
            realloc(channel_indexes, channel_count);
            for(int i = 0; i < channel_count; i++)
                channel_indexes[i] = i * 8;

            return;
        }
        else if(rx_cfg.channel_id.channel_header.ch_freq_band == PHY_BAND_868)
        {
            channel_count = NORMAL_RATE_CHANNEL_COUNT_868;
            realloc(channel_indexes, channel_count);
            for(int i = 0; i < channel_count; i++)
                channel_indexes[i] = i * 8; // TODO not according to spec near the end of the band

            return;
        }
    }
    else if(rx_cfg.channel_id.channel_header.ch_class == PHY_CLASS_LO_RATE)
    {
        if(rx_cfg.channel_id.channel_header.ch_freq_band == PHY_BAND_433)
            channel_count = LO_RATE_CHANNEL_COUNT_433;
        else if(rx_cfg.channel_id.channel_header.ch_freq_band == PHY_BAND_868)
            channel_count = LO_RATE_CHANNEL_COUNT_868;

        realloc(channel_indexes, channel_count);
        for(int i = 0; i < channel_count; i++)
            channel_indexes[i] = i;
    }
}

void process_command_chan()
{
    while(fifo_get_size(&uart_rx_fifo) < COMMAND_CHAN_PARAM_SIZE);

    char param[COMMAND_CHAN_PARAM_SIZE];
    fifo_pop(&uart_rx_fifo, param, COMMAND_CHAN_PARAM_SIZE);

    channel_id_t new_channel = {
		.channel_header.ch_coding = PHY_CODING_PN9
    };

    if(strncmp(param, "433", 3) == 0)
        new_channel.channel_header.ch_freq_band = PHY_BAND_433;
    else if(strncmp(param, "868", 3) == 0)
        new_channel.channel_header.ch_freq_band = PHY_BAND_868;
    else if(strncmp(param, "915", 3) == 0)
        new_channel.channel_header.ch_freq_band = PHY_BAND_915;
    else
        goto error;

    char channel_class = param[3];
    if(channel_class == 'L')
        new_channel.channel_header.ch_class = PHY_CLASS_LO_RATE;
    else if(channel_class == 'N')
        new_channel.channel_header.ch_class = PHY_CLASS_NORMAL_RATE;
    else if(channel_class == 'H')
        new_channel.channel_header.ch_class = PHY_CLASS_HI_RATE;
    else
        goto error;

    uint16_t center_freq_index = atoi((const char*)(param + 4));
    new_channel.center_freq_index = center_freq_index;

    // validate
    if(new_channel.channel_header.ch_freq_band == PHY_BAND_433)
    {
        if(new_channel.channel_header.ch_class == PHY_CLASS_NORMAL_RATE
                && (new_channel.center_freq_index % 8 != 0 || new_channel.center_freq_index > 56))
            goto error;
        else if(new_channel.channel_header.ch_class == PHY_CLASS_LO_RATE && new_channel.center_freq_index > 68)
            goto error;
        else if(new_channel.channel_header.ch_class == PHY_CLASS_HI_RATE)
            goto error;
    } // TODO validate PHY_BAND_868

    // valid band, apply ...
    rx_cfg.channel_id = new_channel;

    // change channel and restart
    prepare_channel_indexes();
    current_channel_indexes_index = 0;
    sched_post_task(&start_rx);
    return;

    error:
        console_print("Error parsing CHAN command. Expected format example: '433L001'\n");
        fifo_clear(&uart_rx_fifo);
}


void process_command_loop()
{
    use_manual_channel_switching = false;
    sched_post_task(&start_rx);
}

void process_uart_rx_fifo()
{
    if(fifo_get_size(&uart_rx_fifo) >= COMMAND_SIZE)
    {
        uint8_t received_cmd[COMMAND_SIZE];
        fifo_pop(&uart_rx_fifo, received_cmd, COMMAND_SIZE);
        if(strncmp(received_cmd, COMMAND_CHAN, COMMAND_SIZE) == 0)
        {
            process_command_chan();
        }
        else if(strncmp(received_cmd, COMMAND_LOOP, COMMAND_SIZE) == 0)
        {
            process_command_loop();
        }
        else if(strncmp(received_cmd, COMMAND_RSET, COMMAND_SIZE) == 0)
        {
            hw_reset();
        }
        else
        {
            char err[40];
            snprintf(err, sizeof(err), "ERROR invalid command %.4s\n", received_cmd);
            console_print(err);
        }

        fifo_clear(&uart_rx_fifo);
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

    char rssi_samples_str[5 * RSSI_SAMPLES_PER_MEASUREMENT] = "";
    int16_t max_rssi_sample = -200;
    for(int i = 0; i < RSSI_SAMPLES_PER_MEASUREMENT; i++)
    {
        rssi_measurement.rssi[i] = hw_radio_get_rssi();
        if(rssi_measurement.rssi[i] > max_rssi_sample)
            max_rssi_sample = rssi_measurement.rssi[i];

        sprintf(rssi_samples_str + (i * 5), ",%04i", rssi_measurement.rssi[i]);
        // TODO delay?
    }

    char str[80];
    char channel_str[8] = "";


    channel_id_to_string(&rx_cfg.channel_id, channel_str, sizeof(channel_str));
    lcd_write_string(channel_str);
    sprintf(str, "%7s,%i%s\n", channel_str, rssi_measurement.tick, rssi_samples_str);
    console_print(str);

#ifdef PLATFORM_EFM32GG_STK3700
    //lcd_all_on();
    lcd_write_number(max_rssi_sample);
#elif defined HAS_LCD
    sprintf(str, "%7s,%d\n", channel_str, max_rssi_sample);
    lcd_write_string(str);
#endif

    if(!use_manual_channel_switching)
    {
        switch_next_channel();
        sched_post_task(&start_rx);
    }
    else
    {
    	sched_post_task(&process_uart_rx_fifo); // check for UART commands first
        uint16_t delay = rand() % 5000;
        timer_post_task_delay(&read_rssi, delay);
    }

    hw_watchdog_feed();
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

void uart_rx_cb(uint8_t data)
{
    error_t err;
    err = fifo_put(&uart_rx_fifo, &data, 1); assert(err == SUCCESS);
    // parse command in read_rssi() task
    use_manual_channel_switching = true;
}

void measureTemperature()
{
	float temp = hw_get_internal_temperature();

	int temperature = (int)(temp * 10);

#ifdef PLATFORM_EFM32GG_STK3700

	int ring_segments = temperature/100;
	ring_segments = ring_segments > 8 ? 8 : ring_segments;
	lcd_show_ring(ring_segments);

	lcd_write_temperature(temperature*10, 1);
#endif

	timer_tick_t tick = timer_get_counter_value();

	char str[80];
	sprintf(str, "%7s,%i,%2d.%2d\n", TEMPERATURE_TAG, tick, (temperature/10), abs(temperature%10));
	console_print(str);
}

void execute_sensor_measurement()
{
	timer_post_task_delay(&execute_sensor_measurement, TEMPERATURE_PERIOD);
	measureTemperature();
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

    prepare_channel_indexes();

    hw_radio_init(NULL, NULL);

    fifo_init(&uart_rx_fifo, uart_rx_buffer, sizeof(uart_rx_buffer));

    console_set_rx_interrupt_callback(&uart_rx_cb);
    console_rx_interrupt_enable(true);

    sched_register_task(&read_rssi);
    sched_register_task(&start_rx);
    sched_register_task(&process_uart_rx_fifo);
    timer_post_task_delay(&start_rx, TIMER_TICKS_PER_SEC * 3);

    sched_register_task((&execute_sensor_measurement));
    timer_post_task_delay(&execute_sensor_measurement, TEMPERATURE_PERIOD);

    measureTemperature();
}


