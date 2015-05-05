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
#include "crc.h"
#include "assert.h"
#include "ng.h"


#ifdef FRAMEWORK_LOG_ENABLED
#define DPRINT(...) log_print_stack_string(LOG_STACK_DLL, __VA_ARGS__)
#else
#define DPRINT(...)
#endif

static dae_access_profile_t NGDEF(_current_access_class);
#define current_access_class NG(_current_access_class)

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


static void process_received_messages()
{
    packet_t* packet = packet_queue_get_received_packet();
    dll_header_t header;
    if(packet)
    {
        DPRINT("Processing received packet");
        log_print_data(packet->hw_radio_packet.data, packet->hw_radio_packet.length + 1); // TODO tmp
        uint16_t crc = crc_calculate(packet->hw_radio_packet.data, packet->hw_radio_packet.length - 2);
        if(memcmp(&crc, packet->hw_radio_packet.data + packet->hw_radio_packet.length + 1 - 2, 2) != 0)
        {
            DPRINT("CRC invalid, skipping packet");
            goto cleanup;
        }

        header.subnet = packet->hw_radio_packet.data[1];
        if(header.subnet != current_access_class.subnet)
        {
            DPRINT("Subnet does not match current access profile, skipping packet");
            goto cleanup;
        }

        header.control = packet->hw_radio_packet.data[2];
        if(header.control_target_address_set)
        {
            uint8_t address_len = 2;
            if(!header.control_vid_used)
                address_len = 8;

            // TODO get deviceid / vid from FS and uncomment check, assert for now
            assert(false);
//            if(memcmp(packet->hw_radio_packet.data + 3, id, address_len) != 0)
//            {
//                DPRINT("Device ID filtering failed, skipping packet");
//                goto cleanup;
//            }
        }
        // TODO filter LQ
        // TODO pass to upper layer
        // TODO Tscan -= Trx
    }

    packet_queue_free_packet(packet); // TODO this should be moved to upper layers when done processing
    return;

    // TODO check if more received packets are pending
    cleanup:
        packet_queue_free_packet(packet);
        return;
}

void packet_received(hw_radio_packet_t* packet)
{
    // we are in interrupt context here, so mark packet for further processing,
    // schedule it and return
    DPRINT("packet received @ %i , RSSI = %i", packet->rx_meta.timestamp, packet->rx_meta.rssi);
    packet_queue_mark_received(packet);
    sched_post_task(&process_received_messages);
}

static void packet_transmitted(hw_radio_packet_t* hw_radio_packet)
{
    DPRINT("Transmitted packet with length = %i", hw_radio_packet->length);
    packet_queue_free_packet(packet_queue_find_packet(hw_radio_packet));
}

void dll_init()
{
    sched_register_task(&process_received_messages);

    hw_radio_init(&alloc_new_packet, &release_packet);
}

void dll_tx_frame()
{
    static uint8_t payload[] = { 0, 1, 2, 3, 4 };
    // TODO get payload from upper layers, hardcoded for now
    packet_t* packet = packet_queue_alloc_packet();

    dll_header_t header = {
        .subnet = 0x05, // TODO hardcoded for now
        .control_target_address_set = false, // TODO assuming broadcast for now
        .control_vid_used = false, // TODO hardcoded for now
        .control_eirp_index = 0, // TODO hardcoded for now
    };

    uint8_t* data_ptr = packet->hw_radio_packet.data + 1; // skip length field for now, we fill this later
    *data_ptr = header.subnet; data_ptr += sizeof(header.subnet);
    *data_ptr = header.control; data_ptr += sizeof(header.control);
    // TODO target address, assuming broadcast for now
    uint8_t payload_size = sizeof(payload);
    memcpy(data_ptr, payload, payload_size); data_ptr += payload_size;
    packet->hw_radio_packet.length = data_ptr - packet->hw_radio_packet.data - 1 + 2; // exclude the length byte and add CRC bytes
    uint16_t crc = crc_calculate(packet->hw_radio_packet.data, packet->hw_radio_packet.length - 2);
    memcpy(data_ptr, &crc, 2);

    log_print_data(packet->hw_radio_packet.data, packet->hw_radio_packet.length + 1); // TODO tmp

    hw_tx_cfg_t tx_cfg = {
        .channel_id = {
            .channel_header.ch_coding = PHY_CODING_PN9,
            .channel_header.ch_class = PHY_CLASS_NORMAL_RATE,
            .channel_header.ch_freq_band = PHY_BAND_433,
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

    // TODO get access profile from FS hardcoded for now

    subband_t subband = {
        .channel_header = {
            .ch_coding = PHY_CODING_PN9,
            .ch_class = PHY_CLASS_NORMAL_RATE,
            .ch_freq_band = PHY_BAND_433
        },
        .channel_index_start = 0,
        .channel_index_end = 0,
        .eirp = 0,
        .ccao = 0
    };

    current_access_class.control_scan_type_is_foreground = true;
    current_access_class.control_csma_ca_mode = CSMA_CA_MODE_UNC;
    current_access_class.control_number_of_subbands = 1;
    current_access_class.subnet = 0x05;
    current_access_class.scan_automation_period = 0;
    current_access_class.transmission_timeout_period = 0;
    current_access_class.subbands[0] = subband;

    // TODO only access class using 1 subband which contains 1 channel index is supported for now

    hw_rx_cfg_t rx_cfg = {
        .channel_id = {
            .channel_header = current_access_class.subbands[0].channel_header,
            .center_freq_index = current_access_class.subbands[0].channel_index_start
        },
        .syncword_class = PHY_SYNCWORD_CLASS1
    };

    hw_radio_set_rx(&rx_cfg, &packet_received, NULL);
}
