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


#include "log.h"
#include "dll.h"
#include "hwradio.h"
#include "packet_queue.h"
#include "packet.h"
#include "crc.h"
#include "assert.h"
#include "fs.h"
#include "ng.h"


#ifdef FRAMEWORK_LOG_ENABLED
#define DPRINT(...) log_print_stack_string(LOG_STACK_DLL, __VA_ARGS__)
#else
#define DPRINT(...)
#endif

static dae_access_profile_t NGDEF(_current_access_class);
#define current_access_class NG(_current_access_class)

static dll_packet_received_callback dll_rx_callback = NULL;
static dll_packet_transmitted_callback dll_tx_callback = NULL;

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


static void process_received_packets()
{
    packet_t* packet = packet_queue_get_received_packet();
    assert(packet != NULL);
    DPRINT("Processing received packet");
    packet_disassemble(packet);

    return;

    // TODO check if more received packets are pending
}

void packet_received(hw_radio_packet_t* packet)
{
    // we are in interrupt context here, so mark packet for further processing,
    // schedule it and return
    DPRINT("packet received @ %i , RSSI = %i", packet->rx_meta.timestamp, packet->rx_meta.rssi);
    packet_queue_mark_received(packet);
    sched_post_task(&process_received_packets);
}

static void packet_transmitted(hw_radio_packet_t* hw_radio_packet)
{
    DPRINT("Transmitted packet with length = %i", hw_radio_packet->length);
    packet_t* packet = packet_queue_find_packet(hw_radio_packet);

    if(dll_tx_callback != NULL)
        dll_tx_callback();

    // TODO move to CSMA
    if(packet->d7atp_ctrl.ctrl_is_start) // do not execute CSMA for guarded response (and thus do not ack) TODO validate
        d7asp_signal_packet_csma_ca_insertion_completed(); // TODO ack request when CSMA/CA succeeds and only for for QoS == None

    d7asp_signal_packet_transmitted(packet);
}

void dll_init()
{
    sched_register_task(&process_received_packets);

    hw_radio_init(&alloc_new_packet, &release_packet);

    fs_read_access_class(0, &current_access_class); // use first access class for now
}

void dll_tx_frame(packet_t* packet)
{
    packet->dll_header = (dll_header_t){
        .subnet = 0x05, // TODO hardcoded for now
        .control_target_address_set = false, // TODO assuming broadcast for now
        .control_vid_used = false, // TODO hardcoded for now
        .control_eirp_index = 0, // TODO hardcoded for now
    };

    dll_header_t* dll_header = &(packet->dll_header);
    dll_header->subnet = 0x05; // TODO hardcoded for now
    dll_header->control_eirp_index = 0; // TODO hardcoded for now
    if(packet->d7atp_addressee != NULL)
    {
        dll_header->control_target_address_set = packet->d7atp_addressee->addressee_ctrl_has_id;
        dll_header->control_vid_used = packet->d7atp_addressee->addressee_ctrl_virtual_id;
    }

    packet_assemble(packet);

    log_print_data(packet->hw_radio_packet.data, packet->hw_radio_packet.length + 1); // TODO tmp

    packet->hw_radio_packet.tx_meta.tx_cfg = (hw_tx_cfg_t){
        .channel_id.channel_header = current_access_class.subbands[0].channel_header,
        .channel_id.center_freq_index = current_access_class.subbands[0].channel_index_start,
        .syncword_class = PHY_SYNCWORD_CLASS1,
        .eirp = 10
    };

    // TODO CSMA/CA

    error_t err = hw_radio_send_packet(&(packet->hw_radio_packet), &packet_transmitted);
    assert(err == SUCCESS);
}

void dll_start_foreground_scan()
{
    // TODO handle Tscan timeout

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

uint8_t dll_assemble_packet_header(packet_t* packet, uint8_t* data_ptr)
{
    uint8_t* dll_header_start = data_ptr;
    *data_ptr = packet->dll_header.subnet; data_ptr += sizeof(packet->dll_header.subnet);
    *data_ptr = packet->dll_header.control; data_ptr += sizeof(packet->dll_header.control);
    if(packet->dll_header.control_target_address_set)
    {
        uint8_t addr_len = packet->dll_header.control_vid_used? 2 : 8;
        memcpy(data_ptr, packet->d7atp_addressee->addressee_id, addr_len); data_ptr += addr_len;
    }

    return data_ptr - dll_header_start;
}

bool dll_disassemble_packet_header(packet_t* packet, uint8_t* data_idx)
{
    packet->dll_header.subnet = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    if(packet->dll_header.subnet != current_access_class.subnet)
    {
        DPRINT("Subnet does not match current access profile, skipping packet");
        return false;
    }

    packet->dll_header.control = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    if(packet->dll_header.control_target_address_set)
    {
        uint8_t address_len = 2;
        if(!packet->dll_header.control_vid_used)
            address_len = 8;

        uint8_t uid[8];
        fs_read_uid(uid); // TODO cache
        if(memcmp(packet->hw_radio_packet.data + (*data_idx), uid, address_len) != 0)
        {
            DPRINT("Device ID filtering failed, skipping packet");
            return false;
        }

        (*data_idx) += address_len;
    }
    // TODO filter LQ
    // TODO pass to upper layer
    // TODO Tscan -= Trx


    if (dll_rx_callback != NULL)
        dll_rx_callback(); // TODO tmp upper layer should callback to app

    return true;
}

void dll_register_rx_callback(dll_packet_received_callback callback)
{
	dll_rx_callback = callback;
}


void dll_register_tx_callback(dll_packet_transmitted_callback callback)
{
	dll_tx_callback = callback;
}
