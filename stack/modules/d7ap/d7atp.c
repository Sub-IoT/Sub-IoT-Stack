/*! \file d7atp.c
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include "assert.h"
#include "d7atp.h"
#include "packet.h"
#include "dll.h"

void d7atp_start_dialog(uint8_t dialog_id, uint8_t transaction_id, packet_t* packet)
{
    // TODO only supports broadcasting data now, no ack, timeout, multiple transactions, ...

    packet->d7atp_ctrl = (d7atp_ctrl_t){
        .ctrl_is_start = true,
        .ctrl_is_stop = true,
        .ctrl_is_timeout_template_present = false,
        .ctrl_is_ack_requested = false,
        .ctrl_ack_not_void = false,
        .ctrl_ack_record = false,
        .ctrl_is_ack_template_present = false
    };

    packet->d7atp_dialog_id = dialog_id;
    packet->d7atp_transaction_id = transaction_id;

    bool should_include_origin_template = false;
    if(packet->d7atp_ctrl.ctrl_is_start /*&& packet->d7atp_ctrl.ctrl_is_ack_requested*/) // TODO spec only requires this when both are true, however we MAY send origin when only first is true
        should_include_origin_template = true;
        // TODO also when responding to broadcast requests

    d7anp_tx_foreground_frame(packet, should_include_origin_template);
}

uint8_t d7atp_assemble_packet_header(packet_t* packet, uint8_t* data_ptr)
{
    uint8_t* d7atp_header_start = data_ptr;
    (*data_ptr) = packet->d7atp_ctrl.ctrl_raw; data_ptr++;
    (*data_ptr) = packet->d7atp_dialog_id; data_ptr++;
    (*data_ptr) = packet->d7atp_transaction_id; data_ptr++;

    assert(!packet->d7atp_ctrl.ctrl_is_timeout_template_present); // TODO timeout template
    assert(!packet->d7atp_ctrl.ctrl_is_ack_template_present); // TODO ack template

    return data_ptr - d7atp_header_start;
}

bool d7atp_disassemble_packet_header(packet_t *packet, uint8_t *data_idx)
{
    packet->d7atp_ctrl.ctrl_raw = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7atp_dialog_id = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7atp_transaction_id = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;

    assert(!packet->d7atp_ctrl.ctrl_is_timeout_template_present); // TODO timeout template
    assert(!packet->d7atp_ctrl.ctrl_is_ack_template_present); // TODO ack template

    return true;
}
