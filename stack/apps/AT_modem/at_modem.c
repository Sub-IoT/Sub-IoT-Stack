/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 CORTUS
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
 *  Authors:
 *  philippe.nunes@cortus.com
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "net/netdev.h"
#include "d7ap_stack.h"
#include "phy.h"

#include "hwsystem.h"
#include "hwradio.h"
#include "debug.h"

#include "fifo.h"
#include "timer.h"
#include "console.h"

#include "at_parser.h"
#include "errno.h"

#include "crc.h"
#include "pn9.h"
#include "fec.h"


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

typedef struct
{
    hw_radio_packet_t hw_radio_packet;
    uint8_t __data[255];
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

static uint8_t history[HISTORY_MAX_SIZE][MAX_AT_COMMAND_SIZE];
static bool history_full = false;

typedef enum
{
    STATE_CONFIG,
    STATE_RX_RUNNING,
    STATE_TX_RUNNING,
    STATE_D7ATX_RUNNING
} state_t;

static channel_id_t current_channel_id = {
    .channel_header.ch_coding = PHY_CODING_PN9,
    .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
    .channel_header.ch_freq_band = PHY_BAND_868,
    .center_freq_index = 0
};

static hw_rx_cfg_t rx_cfg = {
    // channel_id set in bootstrap
    .syncword_class = PHY_SYNCWORD_CLASS1
};

static hw_tx_cfg_t tx_cfg = {
    // channel_id set in bootstrap
    .syncword_class = PHY_SYNCWORD_CLASS1,
    .eirp = 10
};

static frame_t tx_frame;
static frame_t rx_frame;
static uint8_t payload[120]; // max size allowed to apply FEC

static uint16_t counter = 0;
static uint16_t missed_packets_counter = 0;
static uint16_t received_packets_counter = 0;
static int16_t tx_packet_delay_s = 5;

static uint64_t id;
static state_t current_state = STATE_CONFIG;

static void packet_received(hw_radio_packet_t* packet);
static void packet_transmitted(hw_radio_packet_t* packet);
static void repeat_transmit();
static char transmit_packet(unsigned char *value);

static uint8_t encode_packet(hw_radio_packet_t* packet, uint8_t* encoded_packet)
{
    uint8_t encoded_len = packet->length + 1;
    memcpy(encoded_packet, packet->data, packet->length + 1);

#ifndef HAL_RADIO_USE_HW_FEC
    if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
        encoded_len = fec_encode(encoded_packet, packet->length + 1);
#endif

#ifndef HAL_RADIO_USE_HW_DC_FREE
    pn9_encode(encoded_packet, encoded_len);
#endif

    return encoded_len;
}

static void stop() {
    netopt_state_t opmode = NETOPT_STATE_STANDBY;

    netdev->driver->set(netdev, NETOPT_STATE, &opmode, sizeof(netopt_state_t));

    // make sure to cancel tasks which might me pending already
    if(current_state == STATE_TX_RUNNING)
    {
        timer_cancel_task(&repeat_transmit);
        sched_cancel_task(&repeat_transmit);
    }
    else if (current_state == STATE_D7ATX_RUNNING)
    {
        timer_cancel_task(&transmit_packet);
        sched_cancel_task(&transmit_packet);
    }
}

static void start_rx() {
    if (current_state == STATE_TX_RUNNING || current_state == STATE_D7ATX_RUNNING)
        stop();

    DPRINT("start RX");
    current_state = STATE_RX_RUNNING;

    rx_cfg.channel_id = current_channel_id;
    hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
}

static void send_packet()
{
    counter++;

    tx_frame.hw_radio_packet.data[0] = sizeof(id) + sizeof(counter) + strlen(payload) + sizeof(uint16_t);
    memcpy(tx_frame.hw_radio_packet.data + 1, &id, sizeof(id));
    memcpy(tx_frame.hw_radio_packet.data + 1 + sizeof(id), &counter, sizeof(counter));

    memcpy(tx_frame.hw_radio_packet.data + 1 + sizeof(id) + sizeof(counter), payload, strlen(payload));
    uint16_t crc = __builtin_bswap16(crc_calculate(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.data[0] + 1 - 2));
    memcpy(tx_frame.hw_radio_packet.data + 1 + sizeof(id) + sizeof(counter) + strlen(payload), &crc, 2);

    DPRINT("FRAME %i", counter);
    DPRINT_DATA(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.length + 1);

    // Encoding is done by xcvr or phy layer

    hw_radio_send_packet(&tx_frame.hw_radio_packet, &packet_transmitted, 0, NULL);
}


static char transmit_packet(unsigned char *value)
{
    DPRINT("transmitting packet");
    current_state = STATE_D7ATX_RUNNING;

    counter = 0;

    if (strlen((const char*)value))
    {
        DPRINT("sending \"%s\" payload (%d bytes)\n", value, strlen(value));
        strncpy(payload, value, sizeof(payload));
    }

    tx_cfg.channel_id = current_channel_id;
    tx_frame.hw_radio_packet.tx_meta.tx_cfg = tx_cfg;
    send_packet();

    return AT_OK;
}

static void repeat_transmit()
{
    uint8_t encoded_packet[PACKET_MAX_SIZE + 1];

    DPRINT("Repeat transmitting packet");

    counter++;

    memcpy(tx_frame.hw_radio_packet.data + 1 + sizeof(id), &counter, sizeof(counter));
    /* the CRC calculation shall include all the bytes of the frame including the byte for the length*/
    uint16_t crc = __builtin_bswap16(crc_calculate(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.length + 1 - 2));
    memcpy(tx_frame.hw_radio_packet.data + tx_frame.hw_radio_packet.length + 1 - 2, &crc, 2);

    DPRINT("Frame <%i>", counter);
    DPRINT_DATA(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.length + 1);

    // Encode the packet if not supported by xcvr
    uint8_t encoded_length = encode_packet(&tx_frame.hw_radio_packet, encoded_packet);

    DPRINT("Encoded frame len <%i>", encoded_length);
    DPRINT_DATA(encoded_packet, encoded_length);

    struct iovec vec[1];
    vec[0].iov_base = encoded_packet;
    vec[0].iov_len = encoded_length;
    if (netdev->driver->send(netdev, vec, 1) == -ENOTSUP) {
        DPRINT("Cannot send: radio is still transmitting");
    }
}


static hw_radio_packet_t* alloc_new_packet(uint8_t length) {
    return &rx_frame.hw_radio_packet;
}

static void release_packet(hw_radio_packet_t* packet) {
    memset(&rx_frame.hw_radio_packet, 0, sizeof(hw_radio_packet_t));
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
#if PLATFORM_NUM_LEDS > 0
        led_toggle(0);
#endif
        uint16_t msg_counter = 0;
        uint64_t msg_id;
        uint16_t data_len = packet->length - sizeof(msg_id) - sizeof(msg_counter) - 2;

        char rx_data[PACKET_MAX_SIZE + 1];
        memcpy(&msg_id, packet->data + 1, sizeof(msg_id));
        memcpy(&msg_counter, packet->data + 1 + sizeof(msg_id), sizeof(msg_counter));
        memcpy(rx_data, packet->data + 1 + sizeof(msg_id) + sizeof(msg_counter), data_len);
        rx_data[data_len] = '\0';
        char chan[8];
        channel_id_to_string(&(packet->rx_meta.rx_cfg.channel_id), chan, sizeof(chan));
        console_printf("\r\nDATA received: %s", rx_data);
        console_printf("\r\n%7s, counter <%i>, rssi <%idBm>, msgId <%lu>, Id <%lu>, timestamp <%lu>\n", chan, msg_counter, packet->rx_meta.rssi, (unsigned long)msg_id, (unsigned long)id, packet->rx_meta.timestamp);

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

        console_printf("\r\nRX failure rate = %i %%\r\n", (int)per);
    }
}

static void packet_transmitted(hw_radio_packet_t* packet) {

    DPRINT("packet transmitted");

    timer_tick_t delay;
    if(tx_packet_delay_s == 0)
        delay = TIMER_TICKS_PER_SEC / 5;
    else
        delay = TIMER_TICKS_PER_SEC * tx_packet_delay_s;

    timer_post_task_delay(&send_packet, delay);
}

static void process_uart_rx_fifo()
{
    uint8_t received_AT_cmd[MAX_AT_COMMAND_SIZE];
    uint16_t len = fifo_get_size(&uart_rx_fifo);
    unsigned char *ret[MAX_AT_RESPONSE_SIZE] = { 0 };
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
        if(strlen(ret) > 0)
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
        fifo_put(&uart_rx_fifo, &history[history_index][0], strlen(&history[history_index][0]));
        console_print_bytes("\r\n", 2);
        console_print_bytes(&history[history_index][0], strlen(&history[history_index][0]));

        rx_state = STATE_CMD;
        return;
    }

    rx_state = STATE_CMD;

    // The line termination character Carriage Return <cr> tells the modem to accept and
    // process the command.

    /* Process command if Enter key is pressed or UART RX buffer is full */
    if (data == '\r') {

        strncpy(&history[history_top_index][0], uart_rx_buffer, fifo_get_size(&uart_rx_fifo));
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
        uart_rx_buffer[fifo_get_size(&uart_rx_fifo)] = '/0';
        DPRINT("\r\nERROR invalid command %s \r\n", uart_rx_buffer);
        fifo_clear(&uart_rx_fifo);
    }

    console_print_byte(data); // echo
}

static char cmd_read_current_rssi(uint8_t *args_at)
{
    uint16_t rssi;

    netdev->driver->get(netdev, NETOPT_RSSI_VALUE, &rssi, sizeof(uint16_t));
    snprintf(args_at, MAX_AT_RESPONSE_SIZE, "\r\n+RSSI: %ddBm\r\n", (int16_t)rssi);
    return AT_OK;
}

static char cmd_set_preamble_size(uint8_t *args_at)
{
    char ret;
    uint32_t size;

    ret = at_parse_extract_number(args_at, &size);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_PREAMBLE_LENGTH, &size, sizeof(uint16_t));

    return ret;
}

static char cmd_get_preamble_size(unsigned char *value)
{
    uint16_t size;

    netdev->driver->get(netdev, NETOPT_PREAMBLE_LENGTH, &size, sizeof(uint16_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+PRES: %d\r\n", size);
    return AT_OK;
}

static char cmd_set_preamble_polarity(unsigned char *value)
{
    char ret;
    uint32_t polarity;

    ret = at_parse_extract_number(value, &polarity);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_PREAMBLE_POLARITY, &polarity, sizeof(uint8_t));

    return ret;
}

static char cmd_get_preamble_polarity(unsigned char *value)
{
    uint32_t polarity;

    netdev->driver->set(netdev, NETOPT_PREAMBLE_POLARITY, &polarity, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+PREP: %d\r\n", polarity);
    return AT_OK;
}

static char cmd_set_center_freq(unsigned char *value)
{
    char ret;
    uint32_t channel;

    ret = at_parse_extract_number(value, &channel);
    if (ret == AT_OK)
    {
        netdev->driver->set(netdev, NETOPT_CHANNEL, &channel, sizeof(uint32_t));
        netdev->driver->get(netdev, NETOPT_CHANNEL, &channel, sizeof(uint32_t));
        snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+CFREQ: %d\r\n", channel);
    }
    return ret;
}

static char cmd_get_center_freq(unsigned char *value)
{
    uint32_t channel;

    netdev->driver->get(netdev, NETOPT_CHANNEL, &channel, sizeof(uint32_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+CFREQ: %d\r\n", channel);
    return AT_OK;
}

static char cmd_set_fdev(unsigned char *value)
{
    char ret;
    uint32_t freq_dev;

    ret = at_parse_extract_number(value, &freq_dev);
    if (ret == AT_OK)
    {
        netdev->driver->set(netdev, NETOPT_FDEV, &freq_dev, sizeof(uint32_t));
        netdev->driver->get(netdev, NETOPT_FDEV, &freq_dev, sizeof(uint32_t));
        snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+FDEV: %dHz\r\n", freq_dev);
    }

    return ret;
}

static char cmd_get_fdev(unsigned char *value)
{
    uint32_t freq_dev;

    netdev->driver->get(netdev, NETOPT_FDEV, &freq_dev, sizeof(uint32_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+FDEV: %dHz\r\n", freq_dev);
    return AT_OK;
}

static char cmd_set_bitrate(unsigned char *value)
{
    char ret;
    uint32_t bps;

    ret = at_parse_extract_number(value, &bps);
    if (ret == AT_OK)
    {
        netdev->driver->set(netdev, NETOPT_BITRATE, &bps, sizeof(uint32_t));
        netdev->driver->get(netdev, NETOPT_BITRATE, &bps, sizeof(uint32_t));
        snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+BR: %dbps\r\n", bps);
    }

    return ret;
}

static char cmd_get_bitrate(unsigned char *value)
{
    uint32_t bps;

    netdev->driver->get(netdev, NETOPT_BITRATE, &bps, sizeof(uint32_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+BR: %dbps\r\n", bps);
    return AT_OK;
}

static char cmd_set_bandwidth(unsigned char *value)
{
    char ret;
    uint32_t bwdth;

    ret = at_parse_extract_number(value, &bwdth);
    if (ret == AT_OK)
    {
        netdev->driver->set(netdev, NETOPT_BANDWIDTH, &bwdth, sizeof(uint32_t));
        netdev->driver->get(netdev, NETOPT_BANDWIDTH, &bwdth, sizeof(uint32_t));
        snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RXBW: %ldHz\r\n", bwdth);
    }

    return ret;
}

static char cmd_get_bandwidth(unsigned char *value)
{
    uint32_t bwdth;

    netdev->driver->get(netdev, NETOPT_BANDWIDTH, &bwdth, sizeof(uint32_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RXBW: %dHz\r\n", bwdth);
    return AT_OK;
}

static char cmd_set_rssi_threshold(unsigned char *value)
{
    char ret;
    uint32_t rssi_thr;

    ret = at_parse_extract_number(value, &rssi_thr);
    if (ret == AT_OK)
    {
        netdev->driver->set(netdev, NETOPT_CCA_THRESHOLD, &rssi_thr, sizeof(uint8_t));
        netdev->driver->get(netdev, NETOPT_CCA_THRESHOLD, &rssi_thr, sizeof(uint8_t));
        snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RSSIT: %d\r\n", rssi_thr);
    }

    return ret;
}

static char cmd_get_rssi_threshold(unsigned char *value)
{
    uint8_t rssi_thr;

    netdev->driver->get(netdev, NETOPT_CCA_THRESHOLD, &rssi_thr, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RSSIT: %d\r\n", rssi_thr);
    return AT_OK;
}

static char cmd_set_rssi_smoothing(unsigned char *value)
{
    char ret;
    uint32_t nb_samples;

    ret = at_parse_extract_number(value, &nb_samples);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_RSSI_SMOOTHING, &nb_samples, sizeof(uint8_t));

    return ret;
}

static char cmd_get_rssi_smoothing(unsigned char *value)
{
    uint8_t nb_samples;

    netdev->driver->get(netdev, NETOPT_RSSI_SMOOTHING, &nb_samples, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+RSSIS: %d\r\n", nb_samples);
    return AT_OK;
}

static char cmd_set_sync_word_size(unsigned char *value)
{
    char ret;
    uint32_t size;

    ret = at_parse_extract_number(value, &size);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_SYNC_LENGTH, &size, sizeof(uint8_t));

    return ret;
}

static char cmd_get_sync_word_size(unsigned char *value)
{
    uint8_t size;

    netdev->driver->get(netdev, NETOPT_SYNC_LENGTH, &size, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+SYNCWS: %d\r\n", size);
    return AT_OK;
}

static char cmd_set_syncword(unsigned char *value)
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

static void BinToHex (const unsigned char * buff, int length, char * output, int outLength)
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


static char cmd_get_sync_word(unsigned char *value)
{
    uint8_t sync_word[8];
    uint8_t hex_string[17];
    uint8_t sync_size;

    sync_size = netdev->driver->get(netdev, NETOPT_SYNC_WORD, sync_word, sizeof(sync_word));

    BinToHex(sync_word, sync_size, hex_string, sizeof(hex_string));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+SYNCW: %s\r\n", hex_string);
    return AT_OK;
}

static char cmd_set_mod_shaping(unsigned char *value)
{
    char ret;
    uint32_t BT;

    ret = at_parse_extract_number(value, &BT);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_MOD_SHAPING, &BT, sizeof(uint8_t));

    return ret;
}

static char cmd_get_mod_shaping(unsigned char *value)
{
    uint8_t BT;

    netdev->driver->get(netdev, NETOPT_MOD_SHAPING, &BT, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+MODS: %d\r\n", BT);
    return AT_OK;
}

static char cmd_set_crc_on(unsigned char *value)
{
    char ret;
    uint32_t enable;

    ret = at_parse_extract_number(value, &enable);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_INTEGRITY_CHECK, &enable, sizeof(uint8_t));

    return AT_OK;
}

static char cmd_get_crc_on(unsigned char *value)
{
    uint8_t enable;

    netdev->driver->get(netdev, NETOPT_INTEGRITY_CHECK, &enable, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+CRCON: %d\r\n", enable);
    return AT_OK;
}

static char cmd_set_sync_on(unsigned char *value)
{
    char ret;
    uint32_t enable;

    ret = at_parse_extract_number(value, &enable);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_SYNC_ON, &enable, sizeof(uint8_t));

    return AT_OK;
}

static char cmd_get_sync_on(unsigned char *value)
{
    uint8_t enable;

    netdev->driver->get(netdev, NETOPT_SYNC_ON, &enable, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+SYNC: %d\r\n", enable);
    return AT_OK;
}

static char cmd_set_preamble_detector_on(unsigned char *value)
{
    char ret;
    uint32_t enable;

    ret = at_parse_extract_number(value, &enable);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_PREAMBLE_DETECT_ON, &enable, sizeof(uint8_t));
    return AT_OK;
}

static char cmd_get_preamble_detector_on(unsigned char *value)
{
    uint8_t enable;

    netdev->driver->get(netdev, NETOPT_PREAMBLE_DETECT_ON, &enable, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+PRED: %d\r\n", enable);
    return AT_OK;
}

static char cmd_set_opmode(unsigned char *value)
{
    char ret;
    uint32_t opmode;

    ret = at_parse_extract_number(value, &opmode);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_STATE, &opmode, sizeof(netopt_state_t));
    return AT_OK;
}

static char cmd_set_payload_length(unsigned char *value)
{
    char ret;
    uint32_t length;

    ret = at_parse_extract_number(value, &length);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_MAX_PACKET_SIZE, &length, sizeof(uint8_t));

    return ret;
}

static char cmd_set_dc_free_scheme(unsigned char *value)
{
    char ret;
    uint32_t scheme;

    ret = at_parse_extract_number(value, &scheme);
    if (ret == AT_OK)
        netdev->driver->set(netdev, NETOPT_DC_FREE_SCHEME, &scheme, sizeof(uint8_t));

    return AT_OK;
}

static char cmd_get_dc_free_scheme(unsigned char *value)
{
    uint8_t scheme;

    netdev->driver->get(netdev, NETOPT_DC_FREE_SCHEME, &scheme, sizeof(uint8_t));
    snprintf(value, MAX_AT_RESPONSE_SIZE, "\r\n+CRCON: %d\r\n", scheme);
    return AT_OK;
}

static char cmd_start_rx(unsigned char *value)
{
    uint8_t param;

    if (current_state == STATE_TX_RUNNING || current_state == STATE_D7ATX_RUNNING)
        stop();

    /* Switch to continuous listen mode */
    param = false;
    netdev->driver->set(netdev, NETOPT_SINGLE_RECEIVE, &param, sizeof(uint8_t));

    param = NETOPT_STATE_STANDBY;
    netdev->driver->set(netdev, NETOPT_STATE, &param, sizeof(netopt_state_t));

    current_state = STATE_RX_RUNNING;
    DPRINT("Listen mode set\n");

    return AT_OK;
}

static char cmd_tx(unsigned char *value)
{
    uint8_t encoded_packet[PACKET_MAX_SIZE + 1];

    DPRINT("Sending \"%s\" payload (%d bytes)\n", value, strlen(value));

    current_state = STATE_TX_RUNNING;

    counter = 0;
    tx_frame.hw_radio_packet.length = sizeof(id) + sizeof(counter) + strlen(value) + sizeof(uint16_t); /* CRC is an uint16_t */
    memcpy(tx_frame.hw_radio_packet.data + 1, &id, sizeof(id));
    memcpy(tx_frame.hw_radio_packet.data + 1 + sizeof(id), &counter, sizeof(counter));
    /* the CRC calculation shall include all the bytes of the frame including the byte for the length*/
    memcpy(tx_frame.hw_radio_packet.data + 1 + sizeof(id) + sizeof(counter), value, strlen(value));
    uint16_t crc = __builtin_bswap16(crc_calculate(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.length + 1 - 2));
    memcpy(tx_frame.hw_radio_packet.data + 1 + sizeof(id) + sizeof(counter) + strlen(value), &crc, 2);

    DPRINT("Frame <%i>", counter);
    DPRINT_DATA(tx_frame.hw_radio_packet.data, tx_frame.hw_radio_packet.length + 1);

    // Encode the packet if not supported by xcvr
    uint8_t encoded_length = encode_packet(&tx_frame.hw_radio_packet, encoded_packet);

    DPRINT("Encoded frame len<%i>", encoded_length);
    DPRINT_DATA(encoded_packet, encoded_length);

    struct iovec vec[1];
    vec[0].iov_base = encoded_packet;
    vec[0].iov_len = encoded_length;
    if (netdev->driver->send(netdev, vec, 1) == -ENOTSUP) {
        DPRINT("Cannot send: radio is still transmitting");
        return AT_ERROR;
    }

    return AT_OK;
}

static uint32_t get_parameter(netopt_t opt, size_t maxlen)
{
    uint32_t value = 0x00000000;

    netdev->driver->get(netdev, opt, &value, maxlen);
    return value;
}

static char cmd_print_status(unsigned char *value)
{
    char status_item[MAX_AT_RESPONSE_SIZE];

    console_print("\r\nFSK modulation:\r\n");
    switch (get_parameter(NETOPT_MOD_SHAPING, sizeof(uint8_t))) {
        case 1: console_print("BT1.0 "); break;
        case 2: console_print("BT0.5 "); break;
        case 3: console_print("BT0.3 "); break;
        default: console_print("No shaping"); break;
    }

    console_print("\r\n");
    snprintf(status_item, MAX_AT_RESPONSE_SIZE, "Center frequency: %u Hz\r\n", get_parameter(NETOPT_CHANNEL, sizeof(uint32_t)));
    console_print(status_item);
    snprintf(status_item, MAX_AT_RESPONSE_SIZE, "Bitrate: %u bps\r\n", get_parameter(NETOPT_BITRATE, sizeof(uint32_t)));
    console_print(status_item);
    snprintf(status_item, MAX_AT_RESPONSE_SIZE, "Fdev:%u Hz\r\n", get_parameter(NETOPT_FDEV, sizeof(uint32_t)));
    console_print(status_item);
    snprintf(status_item, MAX_AT_RESPONSE_SIZE, "Rxbw:%u Hz\r\n", get_parameter(NETOPT_BANDWIDTH, sizeof(uint32_t)));
    console_print(status_item);
    snprintf(status_item, MAX_AT_RESPONSE_SIZE, "Rssi threshold: %ddB smoothing: %d samples\r\n",
             get_parameter(NETOPT_CCA_THRESHOLD, sizeof(uint8_t)), get_parameter(NETOPT_RSSI_SMOOTHING, sizeof(uint8_t)));
    console_print(status_item);

    if (get_parameter(NETOPT_INTEGRITY_CHECK, sizeof(uint8_t))) {
        console_print("Crc On\r\n");
    } else
        console_print("Crc Off\r\n");

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
                 get_parameter(NETOPT_PREAMBLE_LENGTH, sizeof(uint16_t)),
                 get_parameter(NETOPT_PREAMBLE_POLARITY, sizeof(uint8_t)));
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
        snprintf(status_item, MAX_AT_RESPONSE_SIZE, "\r\nsyncword: %s\r\n", hex_string);
        console_print(status_item);
    } else
        console_print("\r\nSync Off");
    console_print("\r\n");
    return AT_OK;
}

static void cmd_print_help(uint8_t *args_at);

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

    { "+MODE", cmd_set_opmode, NULL, "<%d>", "set operation mode"},
    { "+PAYLEN", cmd_set_payload_length, NULL, "<%d>", "set payload length"},
    { "+TX", cmd_tx, NULL, "<%d or string>","transmit packet over phy"},
    { "+RX", cmd_start_rx, NULL, "","start RX"},
    { "+D7ATX", transmit_packet, NULL, "<%d or string>","transmit over DASH7 protocol"},
    { "+D7ARX", start_rx, NULL, "","start RX over DASH7 protocol"},

    { "+STATUS", cmd_print_status, cmd_print_status, "","print status"},
    { "+HELP", cmd_print_help, cmd_print_help, "","this list of commands"},
    { NULL, NULL, NULL, NULL, NULL},
};

static void cmd_print_help(uint8_t *args_at)
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

static void isr_handler(void *arg) {
    //DPRINT("Netdev ISR handler outside the IRQ context");
    netdev->driver->isr(netdev);
}

static void _event_cb(netdev_t *dev, netdev_event_t event)
{
    if (event == NETDEV_EVENT_ISR) {
        sched_post_task_prio(&isr_handler, MAX_PRIORITY);
    }
    else {
        int len;
        netdev_radio_rx_info_t packet_info;
        switch (event) {
            case NETDEV_EVENT_RX_STARTED:
                DPRINT("Packet reception in progress");
                uint8_t buffer[4];
                uint8_t packet_len;

                len = dev->driver->recv(netdev, buffer, sizeof(buffer), NULL);
                DPRINT("Packet Header received %i\n", len);
                DPRINT_DATA(buffer, len);

                assert(len == 4);

#ifndef HAL_RADIO_USE_HW_DC_FREE
                pn9_encode(buffer, len);
#endif

                DPRINT("Packet Header received after PN9 decoding %i\n", len);
                DPRINT_DATA(buffer, len);

                if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
                {
#ifndef HAL_RADIO_USE_HW_FEC
                    fec_decode_packet(buffer, len, len);
#endif
                    packet_len = fec_calculated_decoded_length(buffer[0] + 1);
                }
                else
                    packet_len = buffer[0] + 1 ;

                DPRINT("RX Packet Length: %i ", packet_len);
                // set PayloadLength to the length of the expected foreground frame
                netdev->driver->set(netdev, NETOPT_MAX_PACKET_SIZE, &packet_len, sizeof(uint8_t));
                break;
            case NETDEV_EVENT_RX_COMPLETE:
                len = dev->driver->recv(netdev, NULL, 0, 0);
                dev->driver->recv(netdev, rx_frame.hw_radio_packet.data, len, &packet_info);
                DPRINT("Payload: (%d bytes), RSSI: %i", len, packet_info.rssi);
                DPRINT_DATA(rx_frame.hw_radio_packet.data, len);

#ifndef HAL_RADIO_USE_HW_DC_FREE
                pn9_encode(rx_frame.hw_radio_packet.data, len);
#endif

                DPRINT("Packet Header received after PN9 decoding %i\n", len);
                DPRINT_DATA(rx_frame.hw_radio_packet.data, len);

                if (current_channel_id.channel_header.ch_coding == PHY_CODING_FEC_PN9)
                {
#ifndef HAL_RADIO_USE_HW_FEC
                    fec_decode_packet(rx_frame.hw_radio_packet.data, len, len);
#endif
                    packet_len = fec_calculated_decoded_length(rx_frame.hw_radio_packet.length + 1);
                }
                else
                    packet_len = rx_frame.hw_radio_packet.length + 1 ;

                DPRINT("Payload decoded: len %i", packet_len);
                DPRINT_DATA(rx_frame.hw_radio_packet.data, packet_len);

                rx_frame.hw_radio_packet.rx_meta.timestamp = timer_get_counter_value();
                rx_frame.hw_radio_packet.rx_meta.crc_status = HW_CRC_UNAVAILABLE;
                rx_frame.hw_radio_packet.rx_meta.rssi = packet_info.rssi;
                rx_frame.hw_radio_packet.rx_meta.lqi = packet_info.lqi;

                packet_received(&rx_frame.hw_radio_packet);
                break;
            case NETDEV_EVENT_TX_COMPLETE:
                DPRINT("Transmission completed");
                // post a new transmission
                if (current_state == STATE_D7ATX_RUNNING)
                    timer_post_task_delay(&send_packet, TIMER_TICKS_PER_SEC * 5);
                else
                    timer_post_task_delay(&repeat_transmit, TIMER_TICKS_PER_SEC * 5);
                break;
            case NETDEV_EVENT_CAD_DONE:
                break;
            case NETDEV_EVENT_TX_TIMEOUT:
                //sx127x_set_sleep(&sx127x);
                break;
            default:
                printf("Unexpected netdev event received: %d\n", event);
                break;
        }
    }
}

void bootstrap() {
    DPRINT("Device booted at time: %d\n", timer_get_counter_value());

    //at_register_command("TEST", NULL, NULL);

    hw_radio_init(&alloc_new_packet, &release_packet);

    netdev = (netdev_t*) &xcvr;
    netdev->driver->init(netdev);

    // override the eventcallback set by the phy layer by the local event handler
    //netdev->event_callback = _event_cb;

    id = hw_get_unique_id();
    strcpy(payload, "TEST");

    rx_cfg.channel_id = current_channel_id;
    tx_cfg.channel_id = current_channel_id;

    tx_frame.hw_radio_packet.tx_meta.tx_cfg = tx_cfg;

    fifo_init(&uart_rx_fifo, uart_rx_buffer, sizeof(uart_rx_buffer));
    console_set_rx_interrupt_callback(&uart_rx_cb);

    sched_register_task(&send_packet, NULL);
    sched_register_task(&repeat_transmit, NULL);
    sched_register_task(&process_uart_rx_fifo, NULL);
    sched_register_task(&isr_handler, netdev);

    //timer_post_task_delay(&send_packet, TIMER_TICKS_PER_SEC * 5);

    // put in RX mode by default
    start_rx();

    /* start the AT interface */
    console_print("\r\nAT modem - commands:\r\n");
    cmd_print_help(NULL);
    console_rx_interrupt_enable();

    current_state = STATE_CONFIG;
}
