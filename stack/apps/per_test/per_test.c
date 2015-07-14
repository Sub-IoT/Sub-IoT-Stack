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
#include <stdlib.h>
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
#include "fifo.h"


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

#define COMMAND_SIZE 4
#define UART_RX_BUFFER_SIZE 20
#define COMMAND_CHAN "CHAN"
#define COMMAND_CHAN_PARAM_SIZE 7
#define COMMAND_TRAN "TRAN"
#define COMMAND_RECV "RECV"


static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = { 0 };
static fifo_t uart_rx_fifo;

typedef enum
{
    STATE_CONFIG_DIRECTION,
    STATE_CONFIG_DATARATE,
    STATE_RUNNING
} state_t;

static channel_id_t current_channel_id = {
	.channel_header.ch_coding = PHY_CODING_PN9,
	.channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
	.channel_header.ch_freq_band = PHY_BAND_433,
	.center_freq_index = 0
};

static hw_rx_cfg_t rx_cfg = {
	// channel_id set in bootstrap
    .syncword_class = PHY_SYNCWORD_CLASS0
};

static hw_tx_cfg_t tx_cfg = {
    // channel_id set in bootstrap
    .syncword_class = PHY_SYNCWORD_CLASS0,
    .eirp = 10
};

static uint8_t tx_buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
static uint8_t rx_buffer[sizeof(hw_radio_packet_t) + 255] = { 0 };
static hw_radio_packet_t* tx_packet = (hw_radio_packet_t*)tx_buffer;
static hw_radio_packet_t* rx_packet = (hw_radio_packet_t*)rx_buffer;
static uint8_t data[PACKET_SIZE + 1] = { [0] = PACKET_SIZE, [1 ... PACKET_SIZE]  = 0 };
static uint16_t counter = 0;
static uint16_t missed_packets_counter = 0;
static uint16_t received_packets_counter = 0;
static hw_radio_packet_t received_packet;
static char lcd_msg[15];
static char record[80];
static uint64_t id;
static bool is_mode_rx = true;
static state_t current_state = STATE_CONFIG_DIRECTION;

static void packet_received(hw_radio_packet_t* packet);
static void packet_transmitted(hw_radio_packet_t* packet);
static void start();

static void start_rx()
{
    DPRINT("start RX");
    current_state = STATE_RUNNING;
    hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
}

static void transmit_packet()
{
    DPRINT("transmitting packet");
    current_state = STATE_RUNNING;
    counter++;
    memcpy(data + 1, &id, sizeof(id));
    memcpy(data + 1 + sizeof(id), &counter, sizeof(counter));
    uint16_t crc = __builtin_bswap16(crc_calculate(data, sizeof(data) - 2));
    memcpy(data + sizeof(data) - 2, &crc, 2);
    memcpy(&tx_packet->data, data, sizeof(data));
    tx_cfg.channel_id = current_channel_id;
    tx_packet->tx_meta.tx_cfg = tx_cfg;
    hw_radio_send_packet(tx_packet, &packet_transmitted);
}

static hw_radio_packet_t* alloc_new_packet(uint8_t length)
{
    return rx_packet;
}

static void release_packet(hw_radio_packet_t* packet)
{
    memset(rx_buffer, 0, sizeof(hw_radio_packet_t) + 255);
}

// TODO code duplication with noise_test, refactor later
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

static void packet_received(hw_radio_packet_t* packet)
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
        char chan[8];
        channel_id_to_string(&(packet->rx_meta.rx_cfg.channel_id), chan, sizeof(chan));
        sprintf(record, "%7s,%i,%i,%lu,%lu,%i\n", chan, msg_counter, packet->rx_meta.rssi, (unsigned long)msg_id, (unsigned long)id, packet->rx_meta.timestamp);
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

static void packet_transmitted(hw_radio_packet_t* packet)
{
#if HW_NUM_LEDS > 0
    led_toggle(0);
#endif
    DPRINT("packet transmitted");
    sprintf(lcd_msg, "TX %i", counter);
    lcd_write_string(lcd_msg);
    timer_post_task(&transmit_packet, TIMER_TICKS_PER_SEC / 5);
}

static void stop()
{
    // make sure to cancel tasks which might me pending already
	if(is_mode_rx)
		sched_cancel_task(&start_rx);
	else
		sched_cancel_task(&transmit_packet);

    hw_radio_set_idle();
}

static void start()
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
            switch(current_channel_id.channel_header.ch_class)
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
                sprintf(record, "%s,%s,%s,%s,%s,%s\n", "channel_id", "counter", "rssi", "tx_id", "rx_id", "timestamp");
                uart_transmit_message(record, strlen(record));
                rx_cfg.channel_id = current_channel_id;
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

static void userbutton_callback(button_id_t button_id)
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
                    if(current_channel_id.channel_header.ch_class == PHY_CLASS_NORMAL_RATE)
                        current_channel_id.channel_header.ch_class = PHY_CLASS_LO_RATE;
                    else
                        current_channel_id.channel_header.ch_class = PHY_CLASS_NORMAL_RATE;

                    break;
            }
            break;
    }

    sched_post_task(&start);
}

// TODO code duplication with noise_test, refactor later
static void process_command_chan()
{
    while(fifo_get_size(&uart_rx_fifo) < COMMAND_CHAN_PARAM_SIZE);

    char param[COMMAND_CHAN_PARAM_SIZE];
    fifo_pop(&uart_rx_fifo, param, COMMAND_CHAN_PARAM_SIZE);

    channel_id_t new_channel;

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

    uint16_t center_freq_index = atoi((const char*)(param + 5));
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
    current_channel_id = new_channel;

    char str[20];
    channel_id_to_string(&current_channel_id, str, sizeof(str));
    uart_transmit_string(str);
    // change channel and restart
    // TODOsched_post_task(&start_rx);
    return;

    error:
        uart_transmit_string("Error parsing CHAN command. Expected format example: '433L001'\n");
        fifo_clear(&uart_rx_fifo);
}

static void process_uart_rx_fifo()
{
    if(fifo_get_size(&uart_rx_fifo) >= COMMAND_SIZE)
    {
        uint8_t received_cmd[COMMAND_SIZE];
        fifo_pop(&uart_rx_fifo, received_cmd, COMMAND_SIZE);
        if(strncmp(received_cmd, COMMAND_CHAN, COMMAND_SIZE) == 0)
        {
            process_command_chan();
        }
        else if(strncmp(received_cmd, COMMAND_TRAN, COMMAND_SIZE) == 0)
        {
            is_mode_rx = false;
            current_state = STATE_RUNNING;
            sched_post_task(&start);
        }
        else if(strncmp(received_cmd, COMMAND_RECV, COMMAND_SIZE) == 0)
        {
            is_mode_rx = true;
            current_state = STATE_RUNNING;
            sched_post_task(&start);
        }
        else
        {
            char err[40];
            snprintf(err, sizeof(err), "ERROR invalid command %.4s\n", received_cmd);
            uart_transmit_string(err);
        }

        fifo_clear(&uart_rx_fifo);
    }

    timer_post_task_delay(&process_uart_rx_fifo, TIMER_TICKS_PER_SEC);
}

static void uart_rx_cb(char data)
{
    error_t err;
    err = fifo_put(&uart_rx_fifo, &data, 1); assert(err == SUCCESS);
    // fifo will be parsed periodically by process_uart_rx_fifo() task
}

void bootstrap()
{
    DPRINT("Device booted at time: %d\n", timer_get_counter_value()); // TODO not printed for some reason, debug later
    id = hw_get_unique_id();
    hw_radio_init(&alloc_new_packet, &release_packet);

    rx_cfg.channel_id = current_channel_id;
    tx_cfg.channel_id = current_channel_id;

    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);

    fifo_init(&uart_rx_fifo, uart_rx_buffer, sizeof(uart_rx_buffer));

    uart_set_rx_interrupt_callback(&uart_rx_cb);
    uart_rx_interrupt_enable(true);

    sched_register_task(&start_rx);
    sched_register_task(&transmit_packet);
    sched_register_task(&start);
    sched_register_task(&process_uart_rx_fifo);

    current_state = STATE_CONFIG_DIRECTION;

    sched_post_task(&start);
    sched_post_task(&process_uart_rx_fifo);
}
