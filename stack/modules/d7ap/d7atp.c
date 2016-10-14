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

#define IS_IN_MASTER_TRANSACTION() (d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD || \
                                    d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD)

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
        d7atp_state = new_state;
        break;
    default:
        assert(false);
    }
}

static void response_period_timeout_handler()
{
    DEBUG_PIN_CLR(2);
    DPRINT("Expiration of the response period");

    assert(d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD
           || d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD
           || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST
           || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE);

    switch_state(D7ATP_STATE_IDLE);

    DPRINT("Transaction is terminated");
    d7asp_signal_transaction_response_period_elapsed();
}


static bool schedule_response_period_timeout_handler(uint8_t timeout_tc)
{
    DEBUG_PIN_SET(2);

    timer_tick_t timeout_ticks = CONVERT_TO_TI(timeout_tc); // TODO take into account time passed since reception

    DPRINT("Starting response_period timer (%i ticks)", timeout_ticks);

    return (timer_post_task_delay(&response_period_timeout_handler, timeout_ticks) == SUCCESS);
}


void d7atp_signal_foreground_scan_expired()
{
    // In case of slave, we can consider that the dialog is terminated
    if (d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD
        || (d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST))
    {
        DPRINT("Dialog terminated");
        current_dialog_id = 0;
        d7asp_signal_transaction_response_period_elapsed();
        switch_state(D7ATP_STATE_IDLE);
    }

    // for a master the dialog terminates when the response period times out
}

void d7atp_signal_dialog_termination()
{
    DPRINT("Dialog is terminated by upper layer");

    // It means that we are not participating in a dialog and we can accept
    // segments marked with START flag set to 1.
    switch_state(D7ATP_STATE_IDLE);
    current_dialog_id = 0;

    // stop eventually the DLL foreground scan
    // TODO d7anp_stop_foreground_scan();
}

void d7atp_init()
{
    d7atp_state = D7ATP_STATE_IDLE;
    current_access_class = ACCESS_CLASS_NOT_SET;
    current_dialog_id = 0;

    sched_register_task(&response_period_timeout_handler);
}

void d7atp_start_dialog(uint8_t dialog_id, uint8_t transaction_id, bool is_last_transaction, packet_t* packet, session_qos_t* qos_settings)
{
    assert(is_last_transaction); // multiple transactions in one dialog not supported yet
    DPRINT("Start dialog %i tx %i", dialog_id, transaction_id);
    switch_state(D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD);
    packet->d7atp_ctrl = (d7atp_ctrl_t){
        .ctrl_is_start = true,
        .ctrl_is_stop = is_last_transaction,
        .ctrl_is_ack_requested = qos_settings->qos_resp_mode == SESSION_RESP_MODE_NO? false : true, // TODO in other cases as well?
        .ctrl_ack_not_void = qos_settings->qos_resp_mode == SESSION_RESP_MODE_ON_ERR? true : false,
        .ctrl_tc = true,
        .ctrl_ack_record = false
    };

    current_dialog_id = dialog_id;
    current_transaction_id = transaction_id;
    packet->d7atp_dialog_id = current_dialog_id;
    packet->d7atp_transaction_id = current_transaction_id;

    uint8_t access_class = packet->d7anp_addressee->ctrl.access_class;
    if(access_class != current_access_class)
        fs_read_access_class(access_class, &active_addressee_access_profile);

    uint8_t slave_listen_timeout = packet->d7atp_tc; // TODO for now we keep this the same as Tc (ie don't cater for transaction retries) this probably needs to be managed from upper layer
    d7anp_set_foreground_scan_timeout(packet->d7atp_tc); // only slave nodes need to be locked on the NWL channel longer than Tc, requester only used Tc timer

    d7anp_tx_foreground_frame(packet, true, &active_addressee_access_profile, slave_listen_timeout);
}

void d7atp_respond_dialog(packet_t* packet)
{
    switch_state(D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE);

    // modify the request headers and turn this into a response
    d7atp_ctrl_t* d7atp = &(packet->d7atp_ctrl);

    // leave ctrl_is_ack_requested as is, keep the requester value
    d7atp->ctrl_ack_not_void = false; // TODO
    d7atp->ctrl_ack_record = false; // TODO validate
    d7atp->ctrl_tc = false;

    bool should_include_origin_template = false; // we don't need to send origin ID, the requester will filter based on dialogID, but ...

    if ((!packet->dll_header.control_target_address_set)
            || (packet->d7atp_ctrl.ctrl_is_start
            && packet->d7atp_ctrl.ctrl_is_ack_requested))
    {
        /*
         * origin template is provided in all requests in which the START flag is set to 1
         * and requesting responses, and in all responses to broadcast requests
         */
        should_include_origin_template = true;
    }

    uint8_t slave_listen_timeout = 0;
    // we are the slave here, so we don't need to lock the other party on the channel, unless we want to signal a pending dormant session with this addressee
    if (!packet->d7atp_ctrl.ctrl_is_start)
        slave_listen_timeout = 0;
    // TODO dormant sessions

    // dialog and transaction id remain the same
    d7anp_tx_foreground_frame(packet, should_include_origin_template, &active_addressee_access_profile, slave_listen_timeout);
}

uint8_t d7atp_assemble_packet_header(packet_t* packet, uint8_t* data_ptr)
{
    uint8_t* d7atp_header_start = data_ptr;
    (*data_ptr) = packet->d7atp_ctrl.ctrl_raw; data_ptr++;
    if (packet->d7atp_ctrl.ctrl_tc)
        (*data_ptr) = packet->d7atp_tc; data_ptr++;
    (*data_ptr) = packet->d7atp_dialog_id; data_ptr++;
    (*data_ptr) = packet->d7atp_transaction_id; data_ptr++;

    // Provide the Responder or Requester ACK template when requested
    if ((d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD) && (packet->d7atp_ctrl.ctrl_is_ack_requested)) {
        //TODO check if at least one Responder has set the ACK_REQ flag
        //TODO aggregate the Device IDs of the Responders that set their ACK_REQ flags.
    }
    else if(packet->d7atp_ctrl.ctrl_is_ack_requested && packet->d7atp_ctrl.ctrl_ack_not_void)
    {
        // add Responder ACK template
        (*data_ptr) = packet->d7atp_transaction_id; data_ptr++; // transaction ID start
        (*data_ptr) = packet->d7atp_transaction_id; data_ptr++; // transaction ID stop
        // TODO ACK bitmap, support for multiple segments to ack not implemented yet
    }

    return data_ptr - d7atp_header_start;
}

bool d7atp_disassemble_packet_header(packet_t *packet, uint8_t *data_idx)
{
    packet->d7atp_ctrl.ctrl_raw = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;

    if (packet->d7atp_ctrl.ctrl_tc)
        packet->d7atp_tc = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;

    packet->d7atp_dialog_id = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7atp_transaction_id = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;

    if(packet->d7atp_ctrl.ctrl_is_ack_requested && packet->d7atp_ctrl.ctrl_ack_not_void)
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

        if (packet->d7atp_ctrl.ctrl_tc)
        {
            schedule_response_period_timeout_handler(packet->d7atp_tc);
        }
    }
    else if(d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE)
    {
        switch_state(D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD);
    }
    else if(d7atp_state == D7ATP_STATE_IDLE)
        assert(!packet->d7atp_ctrl.ctrl_is_ack_requested); // can only occur in this case

    d7asp_signal_packet_transmitted(packet);
}

void d7atp_signal_packet_csma_ca_insertion_completed(bool succeeded)
{
    assert((d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD) ||
            (d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE));

    if(!succeeded)
    {
        DPRINT("CSMA-CA insertion failed, stopping transaction");
        switch_state(D7ATP_STATE_IDLE);
    }

    d7asp_signal_packet_csma_ca_insertion_completed(succeeded);
}

void d7atp_process_received_packet(packet_t* packet)
{
    bool extension = false;

    assert(d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD
           || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD
           || d7atp_state == D7ATP_STATE_IDLE); // IDLE: when doing channel scanning outside of transaction


    // copy addressee from NP origin
    current_addressee.ctrl.id_type = packet->d7anp_ctrl.origin_addressee_ctrl_id_type;
    current_addressee.ctrl.access_class = packet->d7anp_ctrl.origin_addressee_ctrl_access_class;
    memcpy(current_addressee.id, packet->origin_access_id, 8);
    packet->d7anp_addressee = &current_addressee;

    DPRINT("Recvd dialog %i trans id %i, curr %i - %i", packet->d7atp_dialog_id, packet->d7atp_transaction_id, current_dialog_id, current_transaction_id);

    if(IS_IN_MASTER_TRANSACTION())
    {
        if(packet->d7atp_dialog_id != current_dialog_id || packet->d7atp_transaction_id != current_transaction_id)
        {
            DPRINT("Unexpected dialog ID or transaction ID received, skipping segment");
            packet_queue_free_packet(packet);
            return;
        }

        if(packet->d7atp_ctrl.ctrl_is_start)
        {
            // if this is the last transaction, it means that the extension procedure is initiated by the responder
            if (packet->d7atp_ctrl.ctrl_is_stop)
            {
                DPRINT("Dialog terminated, we need to start a new dialog this time as a responder");
                // TODO check Tl and set NP FG scan timeout to allow for dormant sessions
                current_dialog_id = 0;
                switch_state(D7ATP_STATE_IDLE);
                extension = true;
            }
            else
            {
                DPRINT("Start dialog not allowed when in master transaction state, skipping segment");
                packet_queue_free_packet(packet);
                return;
            }
        }
    }
    else
    {
         /*
          * when participating in a Dialog, responder discards segments with Dialog ID
          * not matching the recorded Dialog ID
          */
        if ((current_dialog_id) && (current_dialog_id != packet->d7atp_dialog_id))
        {
            DPRINT("Filtered frame with Dialog ID not matching the recorded Dialog ID");
            packet_queue_free_packet(packet);
            return;
        }

        // When not participating in a Dialog
        if ((!current_dialog_id) && (!packet->d7atp_ctrl.ctrl_is_start))
        {
            //Responders discard segments marked with START flag set to 0 until they receive a segment with START flag set to 1
            DPRINT("Filtered frame with START cleared");
            packet_queue_free_packet(packet);
            return;
        }

        switch_state(D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST);

        // we are in a dialog, update the FG scan timer value according the proposed value in the received frame
        if (packet->d7anp_listen_timeout)
            d7anp_set_foreground_scan_timeout(packet->d7anp_listen_timeout);

        current_dialog_id = packet->d7atp_dialog_id;
        current_transaction_id = packet->d7atp_transaction_id;

        channel_id_t rx_channel = packet->hw_radio_packet.rx_meta.rx_cfg.channel_id;

        // set active_addressee_access_profile to the access_profile supplied by the requester
        if(current_access_class != current_addressee.ctrl.access_class)
        {
            fs_read_access_class(current_addressee.ctrl.access_class, &active_addressee_access_profile);
            current_access_class = current_addressee.ctrl.access_class;
        }

        // but respond on the channel where we received the request on
        active_addressee_access_profile.subbands[0].channel_header = rx_channel.channel_header;
        active_addressee_access_profile.subbands[0].channel_index_start = rx_channel.center_freq_index;
        active_addressee_access_profile.subbands[0].channel_index_end = rx_channel.center_freq_index;
    }

    d7asp_process_received_packet(packet, extension);
}
