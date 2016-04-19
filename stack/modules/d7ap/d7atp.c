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

#include "debug.h"
#include "hwdebug.h"
#include "d7atp.h"
#include "packet_queue.h"
#include "packet.h"
#include "d7asp.h"
#include "dll.h"
#include "ng.h"
#include "log.h"
#include "fs.h"
#include "MODULE_D7AP_defs.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_TP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_TRANS, __VA_ARGS__)
#else
#define DPRINT(...)
#endif


static d7anp_addressee_t NGDEF(_current_addressee);
#define current_addressee NG(_current_addressee)

static uint8_t NGDEF(_current_dialog_id);
#define current_dialog_id NG(_current_dialog_id)

static uint8_t NGDEF(_current_transaction_id);
#define current_transaction_id NG(_current_transaction_id)

static uint8_t NGDEF(_current_access_class);
#define current_access_class NG(_current_access_class)

#define ACCESS_CLASS_NOT_SET 0xFF

static dae_access_profile_t NGDEF(_active_addressee_access_profile);
#define active_addressee_access_profile NG(_active_addressee_access_profile)

typedef enum {
    D7ATP_STATE_IDLE,
    D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD,
    D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD,
    D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST,
    D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE,
    D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD,
} state_t;

static state_t NGDEF(_d7atp_state);
#define d7atp_state NG(_d7atp_state)

#define IS_IN_TRANSACTION() (d7atp_state != D7ATP_STATE_IDLE)

static void switch_state(state_t new_state)
{
    switch(new_state)
    {
    case D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD:
        DPRINT("Switching to D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD");
        assert(d7atp_state == D7ATP_STATE_IDLE);
        d7atp_state = new_state;
        break;
    case D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD:
        DPRINT("Switching to D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD");
        assert(d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD);
        d7atp_state = new_state;
        break;
    case D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST:
        DPRINT("Switching to D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST");
        assert(d7atp_state == D7ATP_STATE_IDLE);
        d7atp_state = new_state;
        break;
    case D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE:
        DPRINT("Switching to D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE");
        assert(d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST || D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD);
        d7atp_state = new_state;
        break;
    case D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD:
        DPRINT("Switching to D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD");
        assert(d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE);
        d7atp_state = new_state;
        break;
    case D7ATP_STATE_IDLE:
        DPRINT("Switching to D7ATP_STATE_IDLE");
        assert(d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD
               || d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD
               || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE
               || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD
               || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST);
        d7atp_state = new_state;
        break;
    default:
        assert(false);
    }
}

void d7atp_signal_foreground_scan_expired()
{
    assert(d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD
           || d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD);

    switch_state(D7ATP_STATE_IDLE);
    DPRINT("Dialog terminated");
    d7asp_signal_transaction_response_period_elapsed();
}

void d7atp_init()
{
    d7atp_state = D7ATP_STATE_IDLE;
    current_access_class = ACCESS_CLASS_NOT_SET;
}

void d7atp_start_dialog(uint8_t dialog_id, uint8_t transaction_id, bool is_last_transaction, packet_t* packet, session_qos_t* qos_settings)
{
    assert(is_last_transaction); // multiple transactions in one dialog not supported yet
    switch_state(D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD);
    packet->d7atp_ctrl = (d7atp_ctrl_t){
        .ctrl_is_start = true,
        .ctrl_is_stop = is_last_transaction,
        .ctrl_is_ack_requested = qos_settings->qos_ctrl_resp_mode == SESSION_RESP_MODE_NONE? false : true,
        .ctrl_ack_not_void = qos_settings->qos_ctrl_ack_not_void,
        .ctrl_ack_record = false,
        .ctrl_is_ack_template_present = false
    };

    current_dialog_id = dialog_id;
    current_transaction_id = transaction_id;
    packet->d7atp_dialog_id = current_dialog_id;
    packet->d7atp_transaction_id = current_transaction_id;

    uint8_t access_class = packet->d7anp_addressee->ctrl.access_class;
    if(access_class != current_access_class)
        fs_read_access_class(access_class, &active_addressee_access_profile);

    bool should_include_origin_template = false;
    if(packet->d7atp_ctrl.ctrl_is_start /*&& packet->d7atp_ctrl.ctrl_is_ack_requested*/) // TODO spec only requires this when both are true, however we MAY send origin when only first is true
        should_include_origin_template = true;
        // TODO also when responding to broadcast requests

    d7anp_tx_foreground_frame(packet, should_include_origin_template, &active_addressee_access_profile);
}

void d7atp_respond_dialog(packet_t* packet)
{
    switch_state(D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE);

    // modify the request headers and turn this into a response
    d7atp_ctrl_t* d7atp = &(packet->d7atp_ctrl);
    d7atp->ctrl_is_start = 0;
    d7atp->ctrl_is_ack_template_present = d7atp->ctrl_is_ack_requested? true : false;
    d7atp->ctrl_is_ack_requested = false;
    d7atp->ctrl_ack_not_void = false; // TODO validate
    d7atp->ctrl_ack_record = false; // TODO validate

    // dialog and transaction id remain the same

    d7anp_tx_foreground_frame(packet, true, &active_addressee_access_profile);
}

uint8_t d7atp_assemble_packet_header(packet_t* packet, uint8_t* data_ptr)
{
    uint8_t* d7atp_header_start = data_ptr;
    (*data_ptr) = packet->d7atp_ctrl.ctrl_raw; data_ptr++;
    (*data_ptr) = packet->d7atp_dialog_id; data_ptr++;
    (*data_ptr) = packet->d7atp_transaction_id; data_ptr++;

    if(packet->d7atp_ctrl.ctrl_is_ack_template_present)
    {
        // add ACK template
        (*data_ptr) = packet->d7atp_transaction_id; data_ptr++; // transaction ID start
        (*data_ptr) = packet->d7atp_transaction_id; data_ptr++; // transaction ID stop
        // TODO ACK bitmap, support for multiple segments to ack not implemented yet
    }

    return data_ptr - d7atp_header_start;
}

bool d7atp_disassemble_packet_header(packet_t *packet, uint8_t *data_idx)
{
    packet->d7atp_ctrl.ctrl_raw = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7atp_dialog_id = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7atp_transaction_id = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;

    if(packet->d7atp_ctrl.ctrl_is_ack_template_present)
    {
        packet->d7atp_ack_template.ack_transaction_id_start = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
        packet->d7atp_ack_template.ack_transaction_id_stop = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
        // TODO ACK bitmap, support for multiple segments to ack not implemented yet
    }

    return true;
}

void d7atp_signal_packet_transmitted(packet_t* packet)
{
    if(d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD)
    {
        switch_state(D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD);       
    }
    else if(d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE)
    {
        switch_state(D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD);
    }
    else
        assert(false);

    d7asp_signal_packet_transmitted(packet);
}

void d7atp_signal_packet_csma_ca_insertion_completed(bool succeeded)
{
    if(!succeeded)
    {
        DPRINT("CSMA-CA insertion failed, stopping transaction");
        switch_state(D7ATP_STATE_IDLE);
    }

    d7asp_signal_packet_csma_ca_insertion_completed(succeeded);
}

void d7atp_process_received_packet(packet_t* packet)
{
    assert(d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD
           || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD
           || d7atp_state == D7ATP_STATE_IDLE); // IDLE: when doing channel scanning outside of transaction

    if(IS_IN_TRANSACTION())
    {
        if(packet->d7atp_dialog_id != current_dialog_id || packet->d7atp_transaction_id != current_transaction_id)
        {
            DPRINT("Unexpected dialog ID or transaction ID received, skipping segment");
            packet_queue_free_packet(packet);
            return;
        }

        // TODO assert(!packet->d7atp_ctrl.ctrl_is_start); // start dialog not allowed when in master transaction state
        if(packet->d7atp_ctrl.ctrl_is_start)
        {
            DPRINT("Start dialog not allowed when in master transaction state, skipping segment");
            packet_queue_free_packet(packet);
            return;
        }
    }
    else
    {
        // not in a transaction, start a new slave transaction
        switch_state(D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST);
        current_dialog_id = packet->d7atp_dialog_id;
        current_transaction_id = packet->d7atp_transaction_id;
        DPRINT("Dialog id %i transaction id %i", current_dialog_id, current_transaction_id);

        // copy addressee from NP origin
        current_addressee.ctrl.has_id = packet->d7anp_ctrl.origin_access_id_present;
        current_addressee.ctrl.virtual_id = packet->d7anp_ctrl.origin_access_id_is_vid;
        current_addressee.ctrl.access_class = packet->d7anp_ctrl.origin_access_class;
        memcpy(current_addressee.id, packet->origin_access_id, 8);
        packet->d7anp_addressee = &current_addressee;

        // set active_addressee_access_profile to the access_profile we received the request on
        uint8_t access_class_received = fs_read_dll_conf_active_access_class();
        if(current_access_class != access_class_received)
        {
            fs_read_access_class(access_class_received, &active_addressee_access_profile);
            current_access_class = access_class_received;
        }
    }

    if(!d7asp_process_received_packet(packet))
        switch_state(D7ATP_STATE_IDLE);
}
