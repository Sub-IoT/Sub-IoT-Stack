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
 *    contact@christophe.vg
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "hwleds.h"
#include "hwradio.h"
#include "hwsystem.h"
#include "hwlcd.h"
#include <hwwatchdog.h>

#include "crc.h"
#include "debug.h"

#ifdef PLATFORM_EFM32GG_STK3700
#include "platform_lcd.h"
#endif
#include "userbutton.h"
#include "fifo.h"

#include "console.h"


#if (!defined PLATFORM_EFM32GG_STK3700  && !defined PLATFORM_EZR32LG_WSTK6200A && !defined PLATFORM_CORTUS_FPGA)
	#error Mismatch between the configured platform and the actual platform.
#endif

// configuration options
#define PACKET_SIZE 255

#ifdef FRAMEWORK_LOG_ENABLED
#include "log.h"
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))

#define COMMAND_SIZE 4
#define COMMAND_CHAN "CHAN"
#define COMMAND_CHAN_PARAM_SIZE 7
#define COMMAND_TRAN "TRAN"
#define COMMAND_TRAN_PARAM_SIZE 3
#define COMMAND_RECV "RECV"
#define COMMAND_RSET "RSET"
#define COMMAND_DATA "DATA"

// Define the maximum length of the user data according the size occupied already by the parameters length, counter, id and crc
#define DATA_MAX_LEN PACKET_SIZE - 2*sizeof(uint16_t) /* word for crc + counter */  - sizeof(uint64_t) /* id */ - 1 /*byte length*/

#define UART_RX_BUFFER_SIZE COMMAND_SIZE + DATA_MAX_LEN
static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = { 0 };
static fifo_t uart_rx_fifo;
static char TX_DATA[DATA_MAX_LEN+1];


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
static uint8_t data[PACKET_SIZE] = { [0 ... PACKET_SIZE-1]  = 0 };
static uint16_t counter = 0;
static uint16_t missed_packets_counter = 0;
static uint16_t received_packets_counter = 0;
static int16_t tx_packet_delay_s = 0;
static char lcd_msg[15];
static uint64_t id;
static bool is_mode_rx = false;
static state_t current_state = STATE_CONFIG_DIRECTION;

static void packet_received(hw_radio_packet_t* packet);
static void packet_transmitted(hw_radio_packet_t* packet);
static void start();
static void increase_channel();

static void start_rx() {
    DPRINT("start RX");
    current_state = STATE_RUNNING;
    hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
}

static void transmit_packet() {
    DPRINT("transmitting packet");
    current_state = STATE_RUNNING;
    counter++;
    data[0] = sizeof(id) + sizeof(counter) + strlen(TX_DATA) + sizeof(uint16_t); /* CRC is an uint16_t */
    memcpy(data + 1, &id, sizeof(id));
    memcpy(data + 1 + sizeof(id), &counter, sizeof(counter));
    /* the CRC calculation shall include all the bytes of the frame including the byte for the length*/
    memcpy(data + 1 + sizeof(id) + sizeof(counter), TX_DATA, strlen(TX_DATA));
    uint16_t crc = __builtin_bswap16(crc_calculate(data, data[0] + 1 - 2));
    memcpy(data + 1 + sizeof(id) + sizeof(counter) + strlen(TX_DATA), &crc, 2);
    memcpy(&tx_packet->data, data, sizeof(data));

    tx_cfg.channel_id = current_channel_id;
    tx_packet->tx_meta.tx_cfg = tx_cfg;
    hw_radio_send_packet(tx_packet, &packet_transmitted);
}

static hw_radio_packet_t* alloc_new_packet(uint8_t length) {
    return rx_packet;
}

static void release_packet(hw_radio_packet_t* packet) {
    memset(rx_buffer, 0, sizeof(hw_radio_packet_t) + 255);
}

// TODO code duplication with noise_test, refactor later
static void channel_id_to_string(channel_id_t* channel, char* str, size_t len) {
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

static void packet_received(hw_radio_packet_t* packet) {
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
        uint16_t data_len = packet->length - sizeof(msg_id) - sizeof(msg_counter) - 2;

        char rx_data[DATA_MAX_LEN+1];
        memcpy(&msg_id, packet->data + 1, sizeof(msg_id));
        memcpy(&msg_counter, packet->data + 1 + sizeof(msg_id), sizeof(msg_counter));
        memcpy(rx_data, packet->data + 1 + sizeof(msg_id) + sizeof(msg_counter), data_len);
        rx_data[data_len] = '\0';
        char chan[8];
        channel_id_to_string(&(packet->rx_meta.rx_cfg.channel_id), chan, sizeof(chan));
		console_printf("DATA received: %s\n", rx_data);
		console_printf("%7s, counter <%i>, rssi <%idBm>, msgId <%lu>, Id <%lu>, timestamp <%lu>\n", chan, msg_counter, packet->rx_meta.rssi, (unsigned long)msg_id, (unsigned long)id, packet->rx_meta.timestamp);

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
	    console_printf("RX failure rate = %i %%", (int)per);

#ifdef PLATFORM_EFM32GG_STK3700
	    lcd_write_string(lcd_msg);
#elif defined HAS_LCD
	    lcd_write_line(4, lcd_msg);
#endif

    }
    hw_watchdog_feed();
}

static void packet_transmitted(hw_radio_packet_t* packet) {
#if HW_NUM_LEDS > 0
    led_toggle(0);
#endif
    DPRINT("packet transmitted");
    sprintf(lcd_msg, "TX %i", counter);
#ifdef PLATFORM_EFM32GG_STK3700
	    lcd_write_string(lcd_msg);
#elif defined HAS_LCD
	    lcd_write_line(4, lcd_msg);
#endif

    timer_tick_t delay;
    if(tx_packet_delay_s == 0)
        delay = TIMER_TICKS_PER_SEC / 5;
    else
        delay = TIMER_TICKS_PER_SEC * tx_packet_delay_s;

    //increase_channel();
    timer_post_task(&transmit_packet, delay);
    hw_watchdog_feed();
}

static void stop() {
	// make sure to cancel tasks which might me pending already
	hw_radio_set_idle();

	if(is_mode_rx) {
		sched_cancel_task(&start_rx);
	} else {
		timer_cancel_task(&transmit_packet);
		sched_cancel_task(&transmit_packet);
	}
}

static void increase_channel() {
	if (current_channel_id.channel_header.ch_freq_band == PHY_BAND_433)
	{
		if (current_channel_id.channel_header.ch_class == PHY_CLASS_LO_RATE)
		{
			current_channel_id.center_freq_index = (current_channel_id.center_freq_index == 68) ? 0 : current_channel_id.center_freq_index + 1;
		}
		else
		{
			current_channel_id.center_freq_index = (current_channel_id.center_freq_index == 56) ? 0 : current_channel_id.center_freq_index + 8;
		}
	}else if (current_channel_id.channel_header.ch_freq_band == PHY_BAND_868)
	{
		if (current_channel_id.channel_header.ch_class == PHY_CLASS_LO_RATE)
		{
			current_channel_id.center_freq_index = (current_channel_id.center_freq_index == 279) ? 0 : current_channel_id.center_freq_index + 1;
		}
		else
		{
			current_channel_id.center_freq_index = (current_channel_id.center_freq_index == 272) ? 0 : current_channel_id.center_freq_index + 8;
		}
	}



	#ifdef PLATFORM_EZR32LG_WSTK6200A
		char str[20];
		channel_id_to_string(&current_channel_id, str, sizeof(str));
		console_print(str);
		lcd_write_line(6, str);
	#endif
}

static void start() {
    counter = 0;
    missed_packets_counter = 0;
    received_packets_counter = 0;
#ifdef HAS_LCD
    uint8_t lcd_line = 0;
#endif

    switch(current_state)
    {
        case STATE_CONFIG_DIRECTION:
            is_mode_rx? sprintf(lcd_msg, "PER RX") : sprintf(lcd_msg, "PER TX");
            break;
        case STATE_CONFIG_DATARATE:
#ifdef HAS_LCD
        	lcd_line = 1;
#endif
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
#ifdef HAS_LCD
        	lcd_line = 2;
#endif
            if(is_mode_rx)
            {
                sprintf(lcd_msg, "RUN RX");
                //lcd_write_string(lcd_msg);
                console_printf("%s,%s,%s,%s,%s,%s\n", "channel_id", "counter", "rssi", "tx_id", "rx_id", "timestamp");
                rx_cfg.channel_id = current_channel_id;
                timer_post_task(&start_rx, TIMER_TICKS_PER_SEC * 5);
            }
            else
            {
                hw_radio_set_idle();
                sprintf(lcd_msg, "RUN TX");
                //lcd_write_string(lcd_msg);
                timer_post_task(&transmit_packet, TIMER_TICKS_PER_SEC * 5);
            }
            break;
    }

#ifdef PLATFORM_EFM32GG_STK3700
	lcd_write_string(lcd_msg);
#elif defined HAS_LCD
	lcd_write_line(lcd_line, lcd_msg);
#endif

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

                case STATE_RUNNING:
                	increase_channel();
                    break;
            }
            break;
    }

    sched_post_task(&start);
}

// TODO code duplication with noise_test, refactor later
// Eg. 433L001
static void process_command_chan()
{
    uint8_t param[COMMAND_CHAN_PARAM_SIZE];
    fifo_pop(&uart_rx_fifo, param, COMMAND_CHAN_PARAM_SIZE);

    channel_id_t new_channel;

    if(strncmp((const char*)param, "433", 3) == 0)
        new_channel.channel_header.ch_freq_band = PHY_BAND_433;
    else if(strncmp((const char*)param, "868", 3) == 0)
        new_channel.channel_header.ch_freq_band = PHY_BAND_868;
    else if(strncmp((const char*)param, "915", 3) == 0)
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
    console_print(str);

#ifdef PLATFORM_EFM32GG_STK3700
#elif HAS_LCD
	lcd_write_line(6, str);
#endif

    // change channel and restart
    // TODOsched_post_task(&start_rx);
    return;

    error:
        console_print("Error parsing CHAN command. Expected format example: '433L001'\n");
        fifo_clear(&uart_rx_fifo);
}

static void process_uart_rx_fifo()
{
    if(fifo_get_size(&uart_rx_fifo) >= COMMAND_SIZE)
    {
        uint8_t received_cmd[COMMAND_SIZE];
        fifo_pop(&uart_rx_fifo, received_cmd, COMMAND_SIZE);
        if(strncmp((const char*)received_cmd, COMMAND_CHAN, COMMAND_SIZE) == 0)
        {
            if (fifo_get_size(&uart_rx_fifo) == COMMAND_CHAN_PARAM_SIZE)
                process_command_chan();
        }
        else if(strncmp((const char*)received_cmd, COMMAND_TRAN, COMMAND_SIZE) == 0)
        {
            if (fifo_get_size(&uart_rx_fifo)  == COMMAND_TRAN_PARAM_SIZE)
            {
                uint8_t param[COMMAND_TRAN_PARAM_SIZE];
                fifo_pop(&uart_rx_fifo, param, COMMAND_TRAN_PARAM_SIZE);
                tx_packet_delay_s = atoi((const char*)param);
                DPRINT("performing TRAN command with %d tx_packet_delay_s\r\n",
                        tx_packet_delay_s);

                stop();
                is_mode_rx = false;
                current_state = STATE_RUNNING;
                sched_post_task(&start);
			}
        }
        else if(strncmp((const char*)received_cmd, COMMAND_RECV, COMMAND_SIZE) == 0)
        {
            DPRINT("entering RECV mode\r\n");
            stop();
            is_mode_rx = true;
            current_state = STATE_RUNNING;
            sched_post_task(&start);
        }
        else if(strncmp((const char*)received_cmd, COMMAND_RSET, COMMAND_SIZE) == 0)
        {
            DPRINT("resetting...\r\n");
            hw_reset();
        }
        else if(strncmp((const char*)received_cmd, COMMAND_DATA, COMMAND_SIZE) == 0)
        {
            DPRINT("New DATA to be sent \r\n");
            int16_t data_len = MIN(DATA_MAX_LEN, fifo_get_size(&uart_rx_fifo));

            fifo_pop(&uart_rx_fifo, TX_DATA, data_len);
            TX_DATA[data_len] = '\0'; // null terminate the data string
            DPRINT("DATA <%s> \r\n", TX_DATA);

            stop();
            is_mode_rx = false;
            current_state = STATE_RUNNING;
            sched_post_task(&start);
        }
        else
        {
            DPRINT("ERROR invalid command %.4s\n\r", received_cmd);
        }

        fifo_clear(&uart_rx_fifo);
    }
}

static void uart_rx_cb(uint8_t data) {
    error_t err;

    /* Process command if Enter key is pressed or UART RX buffer is full */
    if ((data == '\r') || (fifo_get_size(&uart_rx_fifo) == UART_RX_BUFFER_SIZE)) {
        timer_post_task_delay(&process_uart_rx_fifo, TIMER_TICKS_PER_SEC);
    }
    else {
        err = fifo_put(&uart_rx_fifo, &data, 1); assert(err == SUCCESS);
    }
    console_print_byte(data); // echo
}

void bootstrap() {
    DPRINT("Device booted at time: %d\n", timer_get_counter_value());

#ifndef FRAMEWORK_LOG_BINARY
    console_print("\r\nPER TEST - commands:\r\n");
    console_print("  CHANfffriii  channel settings:\r\n");
    console_print("               fff frequency : 433, 868, 915\r\n");
    console_print("               r   rate      : L(ow) N(ormal) H(igh)\r\n");
    console_print("               iii center_freq_index\r\n");
    console_print("  TRANsss      transmit a packet every sss seconds.\r\n");
    console_print("  RECV         receive packets\r\n");
    console_print("  RSET         reset module\r\n");
#endif

    id = hw_get_unique_id();
    strcpy(TX_DATA, "TEST");
    hw_radio_init(&alloc_new_packet, &release_packet);

    rx_cfg.channel_id = current_channel_id;
    tx_cfg.channel_id = current_channel_id;

    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);

    fifo_init(&uart_rx_fifo, uart_rx_buffer, sizeof(uart_rx_buffer));

    console_set_rx_interrupt_callback(&uart_rx_cb);
    console_rx_interrupt_enable();

    sched_register_task(&start_rx);
    sched_register_task(&transmit_packet);
    sched_register_task(&start);
    sched_register_task(&process_uart_rx_fifo);

    current_state = STATE_CONFIG_DIRECTION;

    sched_post_task(&start);
    sched_post_task(&process_uart_rx_fifo);


#ifdef PLATFORM_EFM32GG_STK3700
#elif defined HAS_LCD
    	char str[20];
    	channel_id_to_string(&current_channel_id, str, sizeof(str));
    	lcd_write_line(6, str);
#endif

    timer_post_task(&transmit_packet, TIMER_TICKS_PER_SEC * 1);

}
