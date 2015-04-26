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

#include <packet.h>
#include <hwradio.h>
#include <log.h>
#include "dll.h"
#include "hwradio.h"
#include "packet_queue.h"
#include "packet.h"

#ifdef FRAMEWORK_LOG_ENABLED
#define DPRINT(...) log_print_stack_string(LOG_STACK_DLL, __VA_ARGS__)
#else
#define DPRINT(...)
#endif

static hw_radio_packet_t* alloc_new_packet(uint8_t length)
{
    // note we don't use length because in the current implementation the packets in the queue are of
    // fixed (maximum) size
    return &(packet_queue_alloc_packet()->hw_radio_packet);
}

static void release_packet(hw_radio_packet_t* hw_radio_packet)
{
    packet_queue_free_packet(packet_queue_find_packet(hw_radio_packet));
}

void packet_received(hw_radio_packet_t* packet)
{
    DPRINT("packet received @ %i , RSSI = %i", packet->rx_meta.timestamp, packet->rx_meta.rssi);
}

static void packet_transmitted(hw_radio_packet_t* hw_radio_packet)
{
    DPRINT("Transmitted packet with length = %i", hw_radio_packet->length);
    packet_queue_free_packet(packet_queue_find_packet(hw_radio_packet));
}

void dll_init()
{
    hw_radio_init(&alloc_new_packet, &release_packet);
}

void dll_tx_frame()
{
    // TODO get payload from upper layers + assemble frame, hardcoded for now
    static uint8_t data[] = {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    packet_t* packet = packet_queue_alloc_packet(); // TODO free after transmission
    memcpy(packet->hw_radio_packet.data, data, sizeof(data));
    hw_tx_cfg_t tx_cfg = {
        .channel_id = {
            .ch_coding = PHY_CODING_PN9,
            .ch_class = PHY_CLASS_NORMAL_RATE,
            .ch_freq_band = PHY_BAND_433,
            .center_freq_index = 5
        },
        .syncword_class = PHY_SYNCWORD_CLASS1,
        .eirp = 10
    };

    packet->hw_radio_packet.tx_meta.tx_cfg = tx_cfg;
    hw_radio_send_packet(&(packet->hw_radio_packet), &packet_transmitted);
}

void dll_start_foreground_scan()
{
    // TODO handle Tscan timeout
    // TODO filter subnet
    // TODO filter CRC
    // TODO filter LQ
    // TODO filter address

    // TODO get rx_cfg from access profile / scan automation, hardcoded for now
    hw_rx_cfg_t rx_cfg = {
        .channel_id = {
            .ch_coding = PHY_CODING_PN9,
            .ch_class = PHY_CLASS_NORMAL_RATE,
            .ch_freq_band = PHY_BAND_433,
            .center_freq_index = 5
        },
        .syncword_class = PHY_SYNCWORD_CLASS1
    };

    hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
}
