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
#include "ng.h"

static d7atp_addressee_t NGDEF(_current_addressee);
#define current_addressee NG(_current_addressee)

void d7atp_start_dialog(uint8_t dialog_id, uint8_t transaction_id, packet_t* packet, session_qos_t* qos_settings)
{
    // TODO only supports broadcasting data now, no ack, timeout, multiple transactions, ...

    packet->d7atp_ctrl = (d7atp_ctrl_t){
        .ctrl_is_start = true,
        .ctrl_is_stop = true,
        .ctrl_is_timeout_template_present = false,
        .ctrl_is_ack_requested = qos_settings->qos_ctrl_resp_mode == SESSION_RESP_MODE_NONE? false : true,
        .ctrl_ack_not_void = qos_settings->qos_ctrl_ack_not_void,
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

void d7atp_respond_dialog(packet_t* packet)
{
    // modify the request headers and turn this into a response
    d7atp_ctrl_t* d7atp = &(packet->d7atp_ctrl);
    d7atp->ctrl_is_start = 0;
    d7atp->ctrl_is_ack_template_present = d7atp->ctrl_is_ack_requested? true : false;
    d7atp->ctrl_is_ack_requested = false;
    d7atp->ctrl_ack_not_void = false; // TODO validate
    d7atp->ctrl_ack_record = false; // TODO validate
    d7atp->ctrl_is_timeout_template_present = false;  // TODO validate

    // dialog and transaction id remain the same

    // copy addressee from NP origin
    current_addressee.addressee_ctrl_has_id = packet->d7anp_ctrl.origin_access_id_present;
    current_addressee.addressee_ctrl_virtual_id = packet->d7anp_ctrl.origin_access_id_is_vid;
    current_addressee.addressee_ctrl_access_class = packet->d7anp_ctrl.origin_access_class;
    memcpy(current_addressee.addressee_id, packet->origin_access_id, 8);
    packet->d7atp_addressee = &current_addressee;

    d7anp_tx_foreground_frame(packet, true);
}

uint8_t d7atp_assemble_packet_header(packet_t* packet, uint8_t* data_ptr)
{
    uint8_t* d7atp_header_start = data_ptr;
    (*data_ptr) = packet->d7atp_ctrl.ctrl_raw; data_ptr++;
    (*data_ptr) = packet->d7atp_dialog_id; data_ptr++;
    (*data_ptr) = packet->d7atp_transaction_id; data_ptr++;

    assert(!packet->d7atp_ctrl.ctrl_is_timeout_template_present); // TODO timeout template

    if(packet->d7atp_ctrl.ctrl_is_ack_template_present)
    {
        // add ACK template
        (*data_ptr) = packet->d7atp_transaction_id; data_ptr++; // transaction ID start
        // TODO ACK bitmap, support for multiple segments to ack not implemented yet
    }

    return data_ptr - d7atp_header_start;
}

bool d7atp_disassemble_packet_header(packet_t *packet, uint8_t *data_idx)
{
    packet->d7atp_ctrl.ctrl_raw = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7atp_dialog_id = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7atp_transaction_id = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;

    assert(!packet->d7atp_ctrl.ctrl_is_timeout_template_present); // TODO timeout template

    if(packet->d7atp_ctrl.ctrl_is_ack_template_present)
    {
        packet->d7atp_ack_template.ack_transaction_id_start = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
        // TODO ACK bitmap, support for multiple segments to ack not implemented yet
    }

    return true;
}
