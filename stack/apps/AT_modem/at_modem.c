/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV, CORTUS.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *  Authors:
 *  philippe.nunes@cortus.com
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "debug.h"
#include "log.h"

#include "net/netdev.h"
#include "net/netopt.h"

#include "d7ap.h"
#include "phy.h"
#include "hwradio.h"
#include "hwdebug.h"
#include "hwleds.h"

#include "fifo.h"
#include "timer.h"
#include "console.h"

#include "at_parser.h"
#include "errno.h"

#include "crc.h"
#include "pn9.h"
#include "fec.h"
#include "MODULE_D7AP_defs.h"


#if (!defined PLATFORM_EFM32GG_STK3700  && !defined PLATFORM_EZR32LG_WSTK6200A && !defined PLATFORM_CORTUS_FPGA)
    #error Mismatch between the configured platform and the actual platform.
#endif

#ifdef FRAMEWORK_LOG_ENABLED
#include "log.h"
    #define DPRINT(...)      log_print_string(__VA_ARGS__)
    #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
    #define DPRINT(...)
    #define DPRINT_DATA(...)
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))

#define MAX_AT_RESPONSE_SIZE    512
#define MAX_AT_COMMAND_SIZE     512

// modulation settings

// low rate
#define BITRATE_L 9600 // bps
#define FDEV_L    4800 // Hz
// Carson's rule: 2 x fm + 2 x fd  = 9.600 + 2 x 4.800 = 19.2 kHz
// assuming 10 ppm crystals gives max error of: 2 * 10 ppm * 868 = 17.36 kHz
// => BW > 19.2 + 17.36 kHz => > 36.5 kHZ.
#define RXBW_L    36500 //Hz

// normal rate
#define BITRATE_N 55555 // bps
#define FDEV_N    50000 // Hz
// Carson's rule: 2 x fm + 2 x fd  = 55.555 + 2 x 50 = 155.555 kHz
// assuming 10 ppm crystals gives max error of: 2 * 10 ppm * 868 = 17.36 kHz
// => BW > 155.555 + 17.36 => 172.91 KHz
#define RXBW_N   172910 //Hz

// high rate
#define BITRATE_H 166667 // bps
#define FDEV_H     41667 // Hz
// Carson's rule: 2 x fm + 2 x fd  = 166.667 + 2 x 41.667 = 250 kHz
// assuming 10 ppm crystals gives max error of: 2 * 10 ppm * 868 = 17.36 kHz
// => BW > 250 + 17.36 kHz => > 267.36 kHZ.
#define RXBW_H    267360 //Hz

#define DATA_SIZE 255

typedef struct
{
    hw_radio_packet_t hw_radio_packet;
    uint8_t __data[DATA_SIZE];
} frame_t;

static netdev_t *netdev;

static uint8_t uart_rx_buffer[MAX_AT_COMMAND_SIZE] = { 0 };
static fifo_t uart_rx_fifo;

uint8_t rx_state;
typedef enum
{
    STATE_ESCAPE,
    STATE_CURSOR,
    STATE_CMD
} rx_state_t;

#define HISTORY_MAX_SIZE 5
static uint8_t history_latest_cmd = 0;
static uint8_t history_top_index = 0;
static uint8_t history_index = 0;

static char history[HISTORY_MAX_SIZE][MAX_AT_COMMAND_SIZE];
static bool history_full = false;

typedef enum {
  STATE_IDLE,
  STATE_TX,
  STATE_RX,
//  STATE_BG_SCAN
} state_t;

static hwradio_init_args_t init_args;

static state_t state = STATE_IDLE;

static channel_id_t default_channel_id = {
  .channel_header.ch_coding = PHY_CODING_PN9,
  .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
  .channel_header.ch_freq_band = PHY_BAND_868,
  .center_freq_index = 0
};

#define EMPTY_CHANNEL_ID { .channel_header_raw = 0xFF, .center_freq_index = 0xFF };

static channel_id_t current_channel_id = EMPTY_CHANNEL_ID;

uint16_t default_syncword = 0x192F;

static frame_t tx_frame;
static frame_t rx_frame;

static uint16_t counter = 0;
static uint16_t missed_packets_counter = 0;
static uint16_t received_packets_counter = 0;

static uint64_t id;

bool refill_flag = 0;

static char cmd_tx_continuously(char *value);
static char cmd_tx(char *value);

// TODO code duplication with noise_test, refactor later
static void channel_id_to_string(channel_id_t* channel, char* str, size_t len) {
    char rate = 'N';
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

    snprintf(str, len, "%.3s%c%i", band, rate, channel->center_freq_index);
}

static uint16_t encode_packet(uint8_t *data, uint16_t nbytes)
{
    uint16_t encoded_len = nbytes;

#ifndef HAL_RADIO_USE_HW_FEC
    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        encoded_len = fec_encode(data, nbytes);
#endif

#ifndef HAL_RADIO_USE_HW_DC_FREE
    pn9_encode(data, encoded_len);
#endif

    return encoded_len;
}

static hw_radio_packet_t* alloc_new_packet(uint16_t length) {
    return &rx_frame.hw_radio_packet;
}

static void release_packet(hw_radio_packet_t* packet) {
    memset(&rx_frame.hw_radio_packet, 0, sizeof(hw_radio_packet_t));
}

static void packet_received(hw_radio_packet_t* hw_radio_packet)
{
    assert(state == STATE_RX);
    DPRINT("packet received @ %i , RSSI = %i", hw_radio_packet->rx_meta.timestamp, hw_radio_packet->rx_meta.rssi);

#ifndef HAL_RADIO_USE_HW_DC_FREE
    pn9_encode(hw_radio_packet->data, hw_radio_packet->length);
#endif
#ifndef HAL_RADIO_USE_HW_FEC
    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        fec_decode_packet(hw_radio_packet->data, hw_radio_packet->length, hw_radio_packet->length);
#endif

    hw_radio_packet->length = hw_radio_packet->data[0] + 1;
    DPRINT_DATA(hw_radio_packet->data, hw_radio_packet->length);

    if(hw_radio_packet->length < (sizeof(uint64_t) + sizeof(uint16_t)))
    {
        log_print_error_string("%s:%s Packet too small, %d < %d", __FILE__, __FUNCTION__, hw_radio_packet->length, (sizeof(uint64_t) + sizeof(uint16_t)));
        missed_packets_counter++;
        return;
    }

    if (hw_radio_packet->rx_meta.crc_status == HW_CRC_UNAVAILABLE)
    {
        uint16_t crc;
        crc = __builtin_bswap16(crc_calculate(hw_radio_packet->data, hw_radio_packet->length - 2));

        if(memcmp(&crc, hw_radio_packet->data + hw_radio_packet->length - 2, 2) != 0)
        {
            DPRINT("CRC invalid");
            missed_packets_counter++;
            return;
        }
    }
    else if (hw_radio_packet->rx_meta.crc_status == HW_CRC_INVALID)
    {
        DPRINT("CRC invalid");
        missed_packets_counter++;
        return;
    }

#if PLATFORM_NUM_LEDS > 0
    led_toggle(0);
#endif
    uint16_t msg_counter = 0;
    uint64_t msg_id;
    uint16_t data_len = hw_radio_packet->length - sizeof(msg_id) - sizeof(msg_counter) - 2;

    if(data_len > DATA_SIZE)
    {
        log_print_error_string("%s:%s Data too large, %d > %d", __FILE__, __FUNCTION__, data_len, DATA_SIZE);
        missed_packets_counter++;
        return;
    }

    char rx_data[DATA_SIZE + 1];
    memcpy(&msg_id, hw_radio_packet->data + 1, sizeof(msg_id));
    memcpy(&msg_counter, hw_radio_packet->data + 1 + sizeof(msg_id), sizeof(msg_counter));
    memcpy(rx_data, hw_radio_packet->data + 1 + sizeof(msg_id) + sizeof(msg_counter), data_len);
    rx_data[data_len] = '\0';
    char chan[8];
    channel_id_to_string(&default_channel_id, chan, sizeof(chan));
    DPRINT("DATA received: %s", rx_data);
    DPRINT("%7s, counter <%i>, rssi <%idBm>, msgId <%lu>, Id <%lu>, timestamp <%lu>", chan, msg_counter, hw_radio_packet->rx_meta.rssi, (unsigned long)msg_id, (unsigned long)id, hw_radio_packet->rx_meta.timestamp);

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

    double per = 0;
    if(msg_counter > 0)
        per = 100.0 - ((double)received_packets_counter / (double)msg_counter) * 100.0;

    DPRINT("RX failure rate = %i %%", (int)per);
}

static void packet_header_received(uint8_t *data, uint8_t len)
{
    uint8_t packet_len;
    DPRINT("Packet Header received %i\n", len);
    DPRINT_DATA(data, len);

    assert(len == 4);

#ifndef HAL_RADIO_USE_HW_DC_FREE
    pn9_encode(data, len);
#endif

    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
    {
#ifndef HAL_RADIO_USE_HW_FEC
        fec_decode_packet(data, len, len);
#endif
        packet_len = fec_calculated_decoded_length(data[0] + 1);
    }
    else
        packet_len = data[0] + 1 ;
    if(packet_len > DATA_SIZE)
    {
        packet_len = 0;
        log_print_error_string("%s:%s Received data too large, %d > %d", __FILE__, __FUNCTION__, packet_len, DATA_SIZE);
    }

    DPRINT("RX Packet Length: %i ", packet_len);
    // set PayloadLength to the length of the expected foreground frame
    hw_radio_set_payload_length(packet_len);
}

static void stop() {
    netopt_state_t opmode = NETOPT_STATE_STANDBY;

    netdev->driver->set(netdev, NETOPT_STATE, &opmode, sizeof(netopt_state_t));

    // make sure to cancel tasks which might me pending already
    /*if(state == STATE_TX)
    {
        timer_cancel_task(&cmd_tx_repeat);
        sched_cancel_task(&cmd_tx_repeat);
    }*/
}

static void packet_transmitted(timer_tick_t timestamp)
{
    assert(state == STATE_TX);
    DPRINT("Transmitted packet @ %i with length = %i", timestamp, tx_frame.hw_radio_packet.length);

    state = STATE_IDLE;

    /* TODO Repeat ?
    timer_tick_t delay;
    if(tx_packet_delay_s == 0)
        delay = TIMER_TICKS_PER_SEC * 5;
    else
        delay = TIMER_TICKS_PER_SEC * tx_packet_delay_s;

    timer_post_task_delay(&cmd_tx_repeat, delay);
    */
}

static void fill_in_fifo(uint8_t remaining_bytes_len)
{
    //Refill the fifo with the same packet
    hw_radio_send_payload(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.length);
}

static void process_uart_rx_fifo()
{
    uint8_t received_AT_cmd[MAX_AT_COMMAND_SIZE];
    uint16_t len = fifo_get_size(&uart_rx_fifo);
    char ret[MAX_AT_RESPONSE_SIZE] = { 0 };
    char result = AT_ERROR;

    fifo_pop(&uart_rx_fifo, received_AT_cmd, len);
    received_AT_cmd[len] = '\0'; // Null terminal character

    if ((len < 2) || (received_AT_cmd[0] != 'A' && received_AT_cmd[1] != 'T'))
        goto answer;

    if (len == 2)
    {
        result = AT_OK;
        goto answer;
    }

    // parse and process the AT command
    result = at_parse_line((string_t) received_AT_cmd, ret);

answer:
    if(result == AT_OK)
    {
        if(strlen((const char *)ret) > 0)
        {
            console_print(ret);
        }
        console_print("\r\nOK\r\n");
    }
    else
    {
        console_print("\r\nERROR\r\n");
    }

    fifo_clear(&uart_rx_fifo);
}

static void uart_rx_cb(uint8_t data) {

    if (data == '\b') // back key
    {
        fifo_remove_last_byte(&uart_rx_fifo);
        //console_print_byte(data);
        /* white-tape the character */
        console_print_byte('\b');
        console_print_byte(' ');
        console_print_byte('\b');
        return;
    }
    else if (data == 27) // escape sequence
    {
        fifo_clear(&uart_rx_fifo);
        rx_state = STATE_ESCAPE;
        return;
    }
    else if ((data == 91) && (rx_state ==  STATE_ESCAPE))
    {
        rx_state = STATE_CURSOR;
        return;
    }
    else if ((data == 'A' || data == 'B') && (rx_state ==  STATE_CURSOR))
    {
        //DPRINT("history_latest %d\n", history_latest_cmd);

        if (data == 'A')
        {
            if (history_index > history_latest_cmd)
            {
                if (history_index < (HISTORY_MAX_SIZE -1))
                    history_index++;
                else
                    history_index = 0;
            }
            else if (history_index < history_latest_cmd)
                history_index++;

            //DPRINT("UP history_index %d\n", history_index);
        }
        else if (data == 'B')
        {
            if (history_index <= history_latest_cmd)
            {
                if (history_index > 0)
                    history_index--;
                else if (history_full && history_index == 0)
                    history_index =  HISTORY_MAX_SIZE - 1;
            }
            else if (history_index > history_latest_cmd)
                history_index--;

            //DPRINT("DOWN history_index %d \r\n", history_index);
        }

        fifo_clear(&uart_rx_fifo);
        fifo_put(&uart_rx_fifo, (uint8_t *)&history[history_index][0], strlen(&history[history_index][0]));
        console_print_bytes("\r\n", 2);
        console_print_bytes((uint8_t *)&history[history_index][0], strlen(&history[history_index][0]));

        rx_state = STATE_CMD;
        return;
    }

    rx_state = STATE_CMD;

    // The line termination character Carriage Return <cr> tells the modem to accept and
    // process the command.

    /* Process command if Enter key is pressed or UART RX buffer is full */
    if (data == '\r') {

        strncpy(&history[history_top_index][0], (const char *)uart_rx_buffer, fifo_get_size(&uart_rx_fifo));
        //DPRINT("history_latest_cmd %d  %s \r\n", history_top_index, &history[history_top_index][0]);

        history_index = history_top_index;
        history_latest_cmd = history_top_index;
        if (history_top_index < (HISTORY_MAX_SIZE - 1))
            history_top_index++;
        else
        {
            history_top_index = 0;
            history_full = true;
        }

        //DPRINT("history_index %d \r\n", history_index);
        sched_post_task(&process_uart_rx_fifo);
    }
    else if (fifo_put(&uart_rx_fifo, &data, 1) == ESIZE)
    {
        uart_rx_buffer[fifo_get_size(&uart_rx_fifo)] = '\0';
        DPRINT("\r\nERROR invalid command %s \r\n", uart_rx_buffer);
        fifo_clear(&uart_rx_fifo);
    }

    console_print_byte(data); // echo
}

static char cmd_read_current_rssi(char *value)
{
    uint16_t rssi;

    netdev->driver->get(netdev, NETOPT_RSSI_VALUE, &rssi, sizeof(uint16_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RSSI: %ddBm\r\n", (int16_t)rssi);
    return AT_OK;
}

static char cmd_set_preamble_size(char *value)
{
    char ret;
    uint32_t size;

    ret = at_parse_extract_number(value, &size);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_PREAMBLE_LENGTH, &size, sizeof(uint16_t));

    return ret;
}

static char cmd_get_preamble_size(char *value)
{
    uint16_t size;

    netdev->driver->get(netdev, NETOPT_PREAMBLE_LENGTH, &size, sizeof(uint16_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+PRES: %d\r\n", size);
    return AT_OK;
}

static char cmd_set_preamble_polarity(char *value)
{
    char ret;
    uint32_t polarity;

    ret = at_parse_extract_number(value, &polarity);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_PREAMBLE_POLARITY, &polarity, sizeof(uint8_t));

    return ret;
}

static char cmd_get_preamble_polarity(char *value)
{
    uint32_t polarity;

    netdev->driver->set(netdev, NETOPT_PREAMBLE_POLARITY, &polarity, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+PREP: %d\r\n", (uint8_t)polarity);
    return AT_OK;
}

static char cmd_set_center_freq(char *value)
{
    char ret;
    uint32_t channel;

    ret = at_parse_extract_number(value, &channel);
    if (ret == AT_OK)
    {
        netdev->driver->set(netdev, NETOPT_CHANNEL_FREQUENCY, &channel, sizeof(uint32_t));
        netdev->driver->get(netdev, NETOPT_CHANNEL_FREQUENCY, &channel, sizeof(uint32_t));
        snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+CFREQ: %lu\r\n", channel);
    }
    return ret;
}

static char cmd_get_center_freq(char *value)
{
    uint32_t channel;

    netdev->driver->get(netdev, NETOPT_CHANNEL_FREQUENCY, &channel, sizeof(uint32_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+CFREQ: %lu\r\n", channel);
    return AT_OK;
}

static char cmd_set_fdev(char *value)
{
    char ret;
    uint32_t freq_dev;

    ret = at_parse_extract_number(value, &freq_dev);
    if (ret == AT_OK)
    {
        netdev->driver->set(netdev, NETOPT_FDEV, &freq_dev, sizeof(uint32_t));
        netdev->driver->get(netdev, NETOPT_FDEV, &freq_dev, sizeof(uint32_t));
        snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+FDEV: %luHz\r\n", freq_dev);
    }

    return ret;
}

static char cmd_get_fdev(char *value)
{
    uint32_t freq_dev;

    netdev->driver->get(netdev, NETOPT_FDEV, &freq_dev, sizeof(uint32_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+FDEV: %luHz\r\n", freq_dev);
    return AT_OK;
}

static char cmd_set_bitrate(char *value)
{
    char ret;
    uint32_t bps;

    ret = at_parse_extract_number(value, &bps);
    if (ret == AT_OK)
    {
        netdev->driver->set(netdev, NETOPT_BITRATE, &bps, sizeof(uint32_t));
        netdev->driver->get(netdev, NETOPT_BITRATE, &bps, sizeof(uint32_t));
        snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+BR: %lubps\r\n", bps);
    }

    return ret;
}

static char cmd_get_bitrate(char *value)
{
    uint32_t bps;

    netdev->driver->get(netdev, NETOPT_BITRATE, &bps, sizeof(uint32_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+BR: %lubps\r\n", bps);
    return AT_OK;
}

static char cmd_set_bandwidth(char *value)
{
    char ret;
    uint32_t bwdth;

    ret = at_parse_extract_number(value, &bwdth);
    if (ret == AT_OK)
    {
        netdev->driver->set(netdev, NETOPT_BANDWIDTH, &bwdth, sizeof(uint32_t));
        netdev->driver->get(netdev, NETOPT_BANDWIDTH, &bwdth, sizeof(uint32_t));
        snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RXBW: %luHz\r\n", bwdth);
    }

    return ret;
}

static char cmd_get_bandwidth(char *value)
{
    uint32_t bwdth;

    netdev->driver->get(netdev, NETOPT_BANDWIDTH, &bwdth, sizeof(uint32_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RXBW: %luHz\r\n", bwdth);
    return AT_OK;
}

static char cmd_set_rssi_threshold(char *value)
{
    char ret;
    uint32_t rssi_thr;

    ret = at_parse_extract_number(value, &rssi_thr);
    if (ret == AT_OK)
    {
        netdev->driver->set(netdev, NETOPT_CCA_THRESHOLD, &rssi_thr, sizeof(uint8_t));
        netdev->driver->get(netdev, NETOPT_CCA_THRESHOLD, &rssi_thr, sizeof(uint8_t));
        snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RSSIT: %d\r\n", (uint8_t)rssi_thr);
    }

    return ret;
}

static char cmd_get_rssi_threshold(char *value)
{
    uint8_t rssi_thr;

    netdev->driver->get(netdev, NETOPT_CCA_THRESHOLD, &rssi_thr, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RSSIT: %d\r\n", rssi_thr);
    return AT_OK;
}

static char cmd_set_rssi_smoothing(char *value)
{
    char ret;
    uint32_t nb_samples;

    ret = at_parse_extract_number(value, &nb_samples);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_RSSI_SMOOTHING, &nb_samples, sizeof(uint8_t));

    return ret;
}

static char cmd_get_rssi_smoothing(char *value)
{
    uint8_t nb_samples;

    netdev->driver->get(netdev, NETOPT_RSSI_SMOOTHING, &nb_samples, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RSSIS: %d\r\n", nb_samples);
    return AT_OK;
}

static char cmd_set_sync_word_size(char *value)
{
    char ret;
    uint32_t size;

    ret = at_parse_extract_number(value, &size);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_SYNC_LENGTH, &size, sizeof(uint8_t));

    return ret;
}

static char cmd_get_sync_word_size(char *value)
{
    uint8_t size;

    netdev->driver->get(netdev, NETOPT_SYNC_LENGTH, &size, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+SYNCWS: %d\r\n", size);
    return AT_OK;
}

static char cmd_set_syncword(char *value)
{
    char ret;
    uint8_t sync_word[8];
    uint8_t len = sizeof(sync_word);

    ret = at_parse_extract_hexstring(value, sync_word, &len);
    if (ret == AT_OK)
    {
        DPRINT("sync word:");
        DPRINT_DATA(sync_word, len);
        netdev->driver->set(netdev, NETOPT_SYNC_WORD, sync_word, len);
     }

    return ret;
}

static void BinToHex (const unsigned char * buff, int length, unsigned char * output, int outLength)
{
    char binHex[] = "0123456789ABCDEF";

    if (!output || outLength < 4) return;
    *output = '\0';

    if (!buff || length <= 0 || outLength <= 2 * length)
        return;

    for (; length > 0; --length, outLength -= 2)
    {
        unsigned char byte = *buff++;

        *output++ = binHex[(byte >> 4) & 0x0F];
        *output++ = binHex[byte & 0x0F];
    }
    if (outLength-- <= 0) return;
    *output++ = '\0';
}


static char cmd_get_sync_word(char *value)
{
    uint8_t sync_word[8];
    uint8_t hex_string[17];
    uint8_t sync_size;

    sync_size = netdev->driver->get(netdev, NETOPT_SYNC_WORD, sync_word, sizeof(sync_word));

    BinToHex(sync_word, sync_size, hex_string, sizeof(hex_string));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+SYNCW: %s\r\n", hex_string);
    return AT_OK;
}

static char cmd_set_mod_shaping(char *value)
{
    char ret;
    uint32_t BT;

    ret = at_parse_extract_number(value, &BT);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_MOD_SHAPING, &BT, sizeof(uint8_t));

    return ret;
}

static char cmd_get_mod_shaping(char *value)
{
    uint8_t BT;

    netdev->driver->get(netdev, NETOPT_MOD_SHAPING, &BT, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+MODS: %d\r\n", BT);
    return AT_OK;
}

static char cmd_set_crc_on(char *value)
{
    char ret;
    uint32_t enable;

    ret = at_parse_extract_number(value, &enable);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_INTEGRITY_CHECK, &enable, sizeof(uint8_t));

    return AT_OK;
}

static char cmd_get_crc_on(char *value)
{
    uint8_t enable;

    netdev->driver->get(netdev, NETOPT_INTEGRITY_CHECK, &enable, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+CRCON: %d\r\n", enable);
    return AT_OK;
}

static char cmd_set_sync_on(char *value)
{
    char ret;
    uint32_t enable;

    ret = at_parse_extract_number(value, &enable);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_SYNC_ON, &enable, sizeof(uint8_t));

    return AT_OK;
}

static char cmd_get_sync_on(char *value)
{
    uint8_t enable;

    netdev->driver->get(netdev, NETOPT_SYNC_ON, &enable, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+SYNC: %d\r\n", enable);
    return AT_OK;
}

static char cmd_set_fec_on(char *value)
{
    char ret;
    uint32_t enable;

    ret = at_parse_extract_number(value, &enable);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_FEC_ON, &enable, sizeof(uint8_t));

    return AT_OK;
}

static char cmd_get_fec_on(char *value)
{
    uint8_t enable;

    netdev->driver->get(netdev, NETOPT_FEC_ON, &enable, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+FEC: %d\r\n", enable);
    return AT_OK;
}

static char cmd_set_preamble_detector_on(char *value)
{
    char ret;
    uint32_t enable;

    ret = at_parse_extract_number(value, &enable);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_PREAMBLE_DETECT_ON, &enable, sizeof(uint8_t));
    return AT_OK;
}

static char cmd_get_preamble_detector_on(char *value)
{
    uint8_t enable;

    netdev->driver->get(netdev, NETOPT_PREAMBLE_DETECT_ON, &enable, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+PRED: %d\r\n", enable);
    return AT_OK;
}

static char cmd_set_opmode(char *value)
{
    char ret;
    uint32_t opmode;

    ret = at_parse_extract_number(value, &opmode);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_STATE, &opmode, sizeof(netopt_state_t));
    return AT_OK;
}

static char cmd_get_opmode(char *value)
{
    uint32_t opmode = 0;

    netdev->driver->get(netdev, NETOPT_STATE, &opmode, sizeof(netopt_state_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+MODE: %d\r\n", (uint8_t)opmode);
    return AT_OK;
}


static char cmd_set_payload_length(char *value)
{
    char ret;
    uint32_t length;

    ret = at_parse_extract_number(value, &length);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_MAX_PACKET_SIZE, &length, sizeof(uint8_t));

    return ret;
}

static char cmd_set_dc_free_scheme(char *value)
{
    char ret;
    uint32_t scheme;

    ret = at_parse_extract_number(value, &scheme);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_DC_FREE_SCHEME, &scheme, sizeof(uint8_t));

    return AT_OK;
}

static char cmd_get_dc_free_scheme(char *value)
{
    uint8_t scheme;

    netdev->driver->get(netdev, NETOPT_DC_FREE_SCHEME, &scheme, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+DCFREE: %d\r\n", scheme);
    return AT_OK;
}

static char cmd_start_rx(char *value)
{
    if (state == STATE_TX)
        stop();

    // Setting refill_flag to false
    refill_flag = false;

    // Unlimited Length packet format to set the Receive packet of arbitrary length
    hw_radio_set_payload_length(0x00); // unlimited length mode

    DPRINT("START FG scan @ %i", timer_get_counter_value());

    state = STATE_RX;
    hw_radio_set_opmode(HW_STATE_RX);

    DPRINT("Listen mode set\n");

    return AT_OK;
}

static char cmd_tx(char *value)
{
    DPRINT("Sending \"%s\" payload (%d bytes)\n", value, strlen(value));

    state = STATE_TX;

    tx_frame.hw_radio_packet.length = sizeof(id) + sizeof(counter) + strlen(value);
    if(tx_frame.hw_radio_packet.length > DATA_SIZE)
    {
        DPRINT("%s:%s Cannot send: packet too large, %d > %d", __FILE__, __FUNCTION__, tx_frame.hw_radio_packet.length, DATA_SIZE)
        return AT_ERROR;
    }
    memcpy(tx_frame.hw_radio_packet.data + 1, &id, sizeof(id));
    memcpy(tx_frame.hw_radio_packet.data + 1 + sizeof(id), &counter, sizeof(counter));
    memcpy(tx_frame.hw_radio_packet.data + 1 + sizeof(id) + sizeof(counter), value, strlen(value));

#ifndef HAL_RADIO_USE_HW_CRC
    /* the CRC calculation shall include all the bytes of the frame including the byte for the length*/
    uint16_t crc = __builtin_bswap16(crc_calculate(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.length + 1 - 2));
    memcpy(tx_frame.hw_radio_packet.data + 1 + sizeof(id) + sizeof(counter) + strlen(value), &crc, 2);
    tx_frame.hw_radio_packet.length += sizeof(uint16_t); /* CRC is an uint16_t */
#endif

    tx_frame.hw_radio_packet.data[0] = tx_frame.hw_radio_packet.length;
    tx_frame.hw_radio_packet.length +=1;

    DPRINT("Frame <%i>", counter);
    DPRINT_DATA(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.length);

    // Encode the packet if not supported by xcvr
    tx_frame.hw_radio_packet.length = encode_packet(tx_frame.hw_radio_packet.data,
                                                    tx_frame.hw_radio_packet.length);

    DPRINT("Encoded frame len<%i>", tx_frame.hw_radio_packet.length);
    DPRINT_DATA(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.length);

    if(refill_flag)
        hw_radio_enable_refill(true);

    if (hw_radio_send_payload(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.length) == -ENOTSUP) {
        DPRINT("Cannot send: radio is still transmitting");
        return AT_ERROR;
    }

    return AT_OK;
}

static char cmd_tx_continuously(char *value)
{
    // If you need traces, increase the baudrate to 576000
    refill_flag = true;
    return (cmd_tx(value));
}

static uint32_t get_parameter(netopt_t opt, size_t maxlen)
{
    uint32_t value = 0x00000000;

    netdev->driver->get(netdev, opt, &value, maxlen);
    return value;
}

static char cmd_print_status(char *value)
{
    char status_item[MAX_AT_RESPONSE_SIZE];

    console_print("\r\nFSK modulation: ");
    switch (get_parameter(NETOPT_MOD_SHAPING, sizeof(uint8_t))) {
        case 1: console_print("BT1.0 "); break;
        case 2: console_print("BT0.5 "); break;
        case 3: console_print("BT0.3 "); break;
        default: console_print("No shaping"); break;
    }

    console_print("\r\n");
    snprintf(status_item, MAX_AT_RESPONSE_SIZE, "Center frequency: %lu Hz\r\n", get_parameter(NETOPT_CHANNEL_FREQUENCY, sizeof(uint32_t)));
    console_print(status_item);
    snprintf(status_item, MAX_AT_RESPONSE_SIZE, "Bitrate: %lu bps\r\n", get_parameter(NETOPT_BITRATE, sizeof(uint32_t)));
    console_print(status_item);
    snprintf(status_item, MAX_AT_RESPONSE_SIZE, "Fdev:%lu Hz\r\n", get_parameter(NETOPT_FDEV, sizeof(uint32_t)));
    console_print(status_item);
    snprintf(status_item, MAX_AT_RESPONSE_SIZE, "Rxbw:%lu Hz\r\n", get_parameter(NETOPT_BANDWIDTH, sizeof(uint32_t)));
    console_print(status_item);
    snprintf(status_item, MAX_AT_RESPONSE_SIZE, "Rssi threshold: %ddB smoothing: %d samples\r\n",
             (uint8_t)get_parameter(NETOPT_CCA_THRESHOLD, sizeof(uint8_t)),
             (uint8_t)get_parameter(NETOPT_RSSI_SMOOTHING, sizeof(uint8_t)));
    console_print(status_item);

    if (get_parameter(NETOPT_INTEGRITY_CHECK, sizeof(uint8_t))) {
        console_print("Crc On\r\n");
    } else
        console_print("Crc Off\r\n");

    if (get_parameter(NETOPT_FEC_ON, sizeof(uint8_t))) {
        console_print("FEC On\r\n");
    } else
        console_print("FEC Off\r\n");

    console_print("dcFree:");
    switch (get_parameter(NETOPT_DC_FREE_SCHEME, sizeof(uint8_t))) {
        case 0: console_print("none "); break;
        case 1: console_print("Manchester "); break;
        case 2: console_print("Whitening "); break;
        case 3: console_print("reserved "); break;
    }

    console_print("\r\nPreambleDetect: ");
    if (get_parameter(NETOPT_PREAMBLE_DETECT_ON, sizeof(uint8_t))) {
        snprintf(status_item, MAX_AT_RESPONSE_SIZE, "size %u, polarity %u ",
                 (uint8_t)get_parameter(NETOPT_PREAMBLE_LENGTH, sizeof(uint16_t)),
                 (uint8_t)get_parameter(NETOPT_PREAMBLE_POLARITY, sizeof(uint8_t)));
        console_print(status_item);
    } else
        console_print("Off ");

    if (get_parameter(NETOPT_SYNC_ON, sizeof(uint8_t))) {
        uint8_t sync_word[8];
        uint8_t hex_string[17];
        uint8_t sync_size;

        sync_size = netdev->driver->get(netdev, NETOPT_SYNC_WORD, sync_word, sizeof(sync_word));

        BinToHex(sync_word, sync_size, hex_string, sizeof(hex_string));

        snprintf(status_item, MAX_AT_RESPONSE_SIZE, "\r\nsyncsize:%d ", sync_size);
        console_print(status_item);
        snprintf(status_item, MAX_AT_RESPONSE_SIZE, "\r\nsyncword: %s (LSB first)\r\n", hex_string);
        console_print(status_item);
    } else
        console_print("\r\nSync Off");

    switch (get_parameter(NETOPT_STATE, sizeof(uint8_t))) {
        case NETOPT_STATE_IDLE:
                console_print("IDLE mode"); break;
                break;
        case NETOPT_STATE_STANDBY:
            console_print("STANDBY mode"); break;
            break;
        case NETOPT_STATE_RX:
            console_print("RX mode"); break;
            break;
        case NETOPT_STATE_TX:
            console_print("TX mode"); break;
            break;
#ifdef CIOT25
        case NETOPT_STATE_BYPASS_TX_MANUAL_MODULATION:
            console_print("STANDBY mode"); break;
            break;
        case NETOPT_STATE_BYPASS_RX_MANUAL_DEMODULATION:
            console_print("BYPASS RX MANUAL MODULATION mode"); break;
            break;
        case NETOPT_STATE_RX_DIRECT:
            console_print("RX direct mode"); break;
            break;
        case NETOPT_STATE_TX_FIFO:
            console_print("TX FIFO mode"); break;
            break;
        case NETOPT_STATE_RX_FIFO:
            console_print("RX FIFO mode"); break;
            break;
#endif // CIOT25
        default: console_print("unknown mode"); break;
    }

    console_print("\r\n");
    return AT_OK;
}

static char cmd_print_help(char *value);

/*const*/ AT_COMMAND cmd_items[AT_COMMANDS_NUM] =
{
//    { "h", cmd_hw_reset, "","hardware reset"},
//    { "i", cmd_init, "","initialize radio driver"},
//    { "r", cmd_radio_reg_read, "[Register address]","read single radio register"},
//    { "w", cmd_radio_reg_write, "[Register address], [register value]","write single radio register"},

    { "+RSSI", NULL, cmd_read_current_rssi, "<-dBm>", "read instantaneous RSSI"},
    { "+RSSIT", cmd_set_rssi_threshold, cmd_get_rssi_threshold, "<-dBm>","get/set rssi threshold"},
    { "+RSSIS", cmd_set_rssi_smoothing, cmd_get_rssi_smoothing, "<%d>","get/set rssi smoothing"},

    { "+PREP", cmd_set_preamble_polarity, cmd_get_preamble_polarity, "", "toggle Preamble polarity"},
    { "+PRES", cmd_set_preamble_size, cmd_get_preamble_size, "<%d>", "get/set PreambleSize"},
    { "+PRED", cmd_set_preamble_detector_on, cmd_get_preamble_detector_on, "<%d>", "toggle Preamble detector On"},
    { "+CRCON", cmd_set_crc_on, cmd_get_crc_on, "","toggle crcOn"},
    { "+SYNCWS", cmd_set_sync_word_size, cmd_get_sync_word_size, "<%d>", "get/set Sync word size"},
    { "+SYNCW", cmd_set_syncword, cmd_get_sync_word, "<hex bytes>", "get/set syncword"},
    { "+SYNC", cmd_set_sync_on, cmd_get_sync_on, "", "toggle SyncOn"},

    { "+CHANNEL",  cmd_set_center_freq, cmd_get_center_freq, "<Hz>","get/set RF center frequency"},
    { "+RXBW", cmd_set_bandwidth, cmd_get_bandwidth, "<Hz>","get/set bandwith"},
    { "+FDEV", cmd_set_fdev, cmd_get_fdev, "<Hz>","get/set fdev"},
    { "+BR", cmd_set_bitrate, cmd_get_bitrate, "<bps>","get/set bitrate"},

    { "+BT", cmd_set_mod_shaping, cmd_get_mod_shaping, "", "get/set modulation shaping"},
    { "+DCFREE", cmd_set_dc_free_scheme, cmd_get_dc_free_scheme, "%d", "get/set the DC free coding scheme"},
    { "+FEC", cmd_set_fec_on, cmd_get_fec_on, "%d", "get/set the FEC encoder"},

    { "+MODE", cmd_set_opmode, cmd_get_opmode, "<%d>", "get/set operation mode"},
    { "+PAYLEN", cmd_set_payload_length, NULL, "<%d>", "set payload length"},
    { "+TX", cmd_tx, NULL, "<%d or string>","transmit packet over phy"},
    { "+TXC", cmd_tx_continuously, NULL, "<%d or string>", "transmit continuously same packet over phy"},
    { "+RX", cmd_start_rx, NULL, "","start RX"},

    { "+STATUS", cmd_print_status, cmd_print_status, "","print status"},
    { "+HELP", cmd_print_help, cmd_print_help, "","this list of commands"},
    { NULL, NULL, NULL, NULL, NULL},
};

static char cmd_print_help(char *value)
{
    int i;
    char help_item[MAX_AT_RESPONSE_SIZE];

    snprintf(help_item, MAX_AT_RESPONSE_SIZE, "%-10s %-14s %s\r\n", "Command", "Arg", "Description");
    console_print(help_item);
    console_print("------------------------------------------------------\r\n");

    for (i = 0; cmd_items[i].cmd != NULL ; i++) {
        snprintf(help_item, MAX_AT_RESPONSE_SIZE, "%-10s %-14s %s\r\n",
                 cmd_items[i].cmd, cmd_items[i].arg_descr, cmd_items[i].description);
        console_print(help_item);
    }

    return AT_OK;
}

static void configure_channel(const channel_id_t* channel) {
    if(phy_radio_channel_ids_equal(&current_channel_id, channel)) {
        return;
    }

    // configure modulation settings
    if(channel->channel_header.ch_class == PHY_CLASS_LO_RATE)
    {
        hw_radio_set_bitrate(BITRATE_L);
        hw_radio_set_tx_fdev(FDEV_L);
        hw_radio_set_rx_bw_hz(RXBW_L);
        hw_radio_set_preamble_size(PREAMBLE_LOW_RATE_CLASS * 8);
    }
    else if(channel->channel_header.ch_class == PHY_CLASS_NORMAL_RATE)
    {
        hw_radio_set_bitrate(BITRATE_N);
        hw_radio_set_tx_fdev(FDEV_N);
        hw_radio_set_rx_bw_hz(RXBW_N);
        hw_radio_set_preamble_size(PREAMBLE_NORMAL_RATE_CLASS * 8);
    }
    else if(channel->channel_header.ch_class == PHY_CLASS_HI_RATE)
    {
        hw_radio_set_bitrate(BITRATE_H);
        hw_radio_set_tx_fdev(FDEV_H);
        hw_radio_set_rx_bw_hz(RXBW_H);
        hw_radio_set_preamble_size(PREAMBLE_HI_RATE_CLASS * 8);
    }

    // TODO regopmode for LF?

    uint32_t center_freq = 433.06e6;
    if(channel->channel_header.ch_freq_band == PHY_BAND_868)
        center_freq = 863e6;
    else if(channel->channel_header.ch_freq_band == PHY_BAND_915)
        center_freq = 902e6;

    uint32_t channel_spacing_half = 100e3;
    if(channel->channel_header.ch_class == PHY_CLASS_LO_RATE)
        channel_spacing_half = 12500;

    center_freq += 25000 * channel->center_freq_index + channel_spacing_half;
    hw_radio_set_center_freq(center_freq);

    current_channel_id = *channel;
    DPRINT("set channel_header %i, channel_band %i, center_freq_index %i\n",
           current_channel_id.channel_header_raw,
           current_channel_id.channel_header.ch_freq_band,
           current_channel_id.center_freq_index);
}


void bootstrap() {
    DPRINT("Device booted at time: %d\n", timer_get_counter_value());

    init_args.alloc_packet_cb = alloc_new_packet;
    init_args.release_packet_cb = release_packet;
    init_args.rx_packet_cb = packet_received;
    init_args.tx_packet_cb = packet_transmitted;
    init_args.rx_packet_header_cb = packet_header_received;
    init_args.tx_refill_cb = fill_in_fifo;

    hw_radio_init(&init_args);

#ifdef HAL_RADIO_USE_HW_CRC
    hw_radio_set_crc_on(true);
#else
    hw_radio_set_crc_on(false);
#endif

#ifdef HAL_RADIO_USE_HW_DC_FREE
    hw_radio_set_dc_free(HW_DC_FREE_WHITENING);
#else
    hw_radio_set_dc_free(HW_DC_FREE_NONE);
#endif

    hw_radio_set_sync_word((uint8_t *)&default_syncword, sizeof(uint16_t));

    configure_channel(&default_channel_id);
    hw_radio_set_tx_power(10);

    netdev = (netdev_t*) &xcvr;

    id = hw_get_unique_id();

    fifo_init(&uart_rx_fifo, uart_rx_buffer, sizeof(uart_rx_buffer));
    console_set_rx_interrupt_callback(&uart_rx_cb);

    sched_register_task(&process_uart_rx_fifo);

    // put in RX mode by default
    cmd_start_rx(NULL);

    /* start the AT interface */
    console_print("\r\nAT modem - commands:\r\n");
    cmd_print_help(NULL);
    console_rx_interrupt_enable();
}
