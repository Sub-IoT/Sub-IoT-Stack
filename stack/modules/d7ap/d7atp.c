/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
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
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author maarten.weyn@uantwerpen.be
 *  \author philippe.nunes@cortus.com
 *
 */
#include <assert.h>

#include "debug.h"
#include "hwdebug.h"
#include "d7ap.h"
#include "d7atp.h"
#include "packet_queue.h"
#include "packet.h"
#include "d7asp.h"
#include "dll.h"
#include "ng.h"
#include "log.h"
#include "d7ap_fs.h"
#include "MODULE_D7AP_defs.h"
#include "compress.h"
#include "phy.h"
#include "errors.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_TP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_TRANS, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif


static d7ap_addressee_t NGDEF(_current_addressee);
#define current_addressee NG(_current_addressee)

static uint8_t NGDEF(_current_dialog_id);
#define current_dialog_id NG(_current_dialog_id)

static uint8_t NGDEF(_current_transaction_id);
#define current_transaction_id NG(_current_transaction_id)

static uint8_t NGDEF(_current_access_class);
#define current_access_class NG(_current_access_class)

static timer_tick_t NGDEF(_current_Tl_received);
#define current_Tl_received NG(_current_Tl_received)

#define ACCESS_CLASS_NOT_SET 0xFF

static dae_access_profile_t NGDEF(_active_addressee_access_profile);
#define active_addressee_access_profile NG(_active_addressee_access_profile)

static bool NGDEF(_stop_dialog_after_tx);
#define stop_dialog_after_tx NG(_stop_dialog_after_tx)

static timer_event d7atp_response_period_expired_timer;
static timer_event d7atp_execution_delay_expired_timer;

static bool ctrl_xoff;

typedef enum {
    D7ATP_STATE_STOPPED,
    D7ATP_STATE_IDLE,
    D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD,
    D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD,
    D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST,
    D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE,
    D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD,
} state_t;

static state_t NGDEF(_d7atp_state) = D7ATP_STATE_STOPPED;
#define d7atp_state NG(_d7atp_state)

#define IS_IN_MASTER_TRANSACTION() (d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD || \
                                    d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD)

static void switch_state(state_t new_state)
{
    switch(new_state)
    {
    case D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD:
        DPRINT("Switching to D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD");
        assert(d7atp_state == D7ATP_STATE_IDLE ||
               d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD);
        d7atp_state = new_state;
        break;
    case D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD:
        DPRINT("Switching to D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD");
        assert(d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD);
        d7atp_state = new_state;
        break;
    case D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST:
        DPRINT("Switching to D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST");
        assert(d7atp_state == D7ATP_STATE_IDLE || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD);
        d7atp_state = new_state;
        break;
    case D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE:
        DPRINT("Switching to D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE");
        assert(d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST);
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

static void execution_delay_timeout_handler()
{
    assert(d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD);

    // After the Execution Delay period, the Requester engages in a DLL foreground scan for a duration of TC
    DPRINT("Execution Delay period is expired @%i",  timer_get_counter_value());

    d7anp_start_foreground_scan();
}

static void terminate_dialog()
{
    DPRINT("Dialog terminated");

    switch_state(D7ATP_STATE_IDLE);
    current_dialog_id = 0;
    current_transaction_id = NO_ACTIVE_REQUEST_ID;
    stop_dialog_after_tx = false;

    // Discard eventually the Tc timer
    timer_cancel_event(&d7atp_response_period_expired_timer);

    d7asp_signal_dialog_terminated();
    dll_notify_dialog_terminated();
}

static void response_period_timeout_handler()
{
//    DEBUG_PIN_CLR(2);
    DPRINT("Expiration of the response period");

    assert(d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD
           || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST
           || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE);

    current_transaction_id = NO_ACTIVE_REQUEST_ID;
    DPRINT("Transaction is terminated");
    d7asp_signal_transaction_terminated();

    if (stop_dialog_after_tx)
    {
        // no FG scan required after transaction, we can end dialog now
        d7anp_stop_foreground_scan();
        terminate_dialog();
    }
    else if (current_Tl_received > 0)
    {
        d7anp_set_foreground_scan_timeout(current_Tl_received);
        d7anp_start_foreground_scan();
    }
}

static timer_tick_t adjust_timeout_value(timer_tick_t timeout_ticks, timer_tick_t request_received_timestamp)
{

    // Adjust the timeout value according the time passed since reception
    timer_tick_t delta = timer_get_counter_value() - request_received_timestamp;
    if (timeout_ticks > delta)
        timeout_ticks -= delta;
    else
        timeout_ticks = 0;

    DPRINT("adjusted timeout val = %i (-%i)", timeout_ticks, delta);
    return timeout_ticks;
}

static void sel_config_modified_callback(uint8_t file_id)
{
    d7a_segment_filter_options_t segment_filter_options;
    uint32_t length = D7A_FILE_SEL_CONF_SEGMENT_FILTER_SIZE;
    d7ap_fs_read_file(D7A_FILE_SEL_CONF_FILE_ID, D7A_FILE_SEL_CONF_SEGMENT_FILTER_OFFSET, &segment_filter_options.raw, &length, ROOT_AUTH);
    ctrl_xoff = segment_filter_options.xoff;
}

static void schedule_response_period_timeout_handler(timer_tick_t timeout_ticks)
{
//    DEBUG_PIN_SET(2);

    DPRINT("Starting response_period timer (%i ticks)", timeout_ticks);

    d7atp_response_period_expired_timer.next_event = timeout_ticks;
    error_t rtc = timer_add_event(&d7atp_response_period_expired_timer);
    assert(rtc == SUCCESS);
}

void d7atp_signal_foreground_scan_expired()
{
    // Reset the transaction Id
    current_transaction_id = NO_ACTIVE_REQUEST_ID;

    // In case of slave, we can consider that the dialog is terminated
    if (d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD
        || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST
        || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE )
    {
        terminate_dialog();
    }
    else if (d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD)
    {
        d7asp_signal_transaction_terminated();
    }
    else
    {
        DPRINT("A FG scan initiated probably by an advertising background frame "
        "or after a dormant session notification is expired");
        dll_notify_dialog_terminated(); // enter scan automation
    }
}

void d7atp_signal_dialog_termination()
{
    DPRINT("Dialog is terminated by upper layer");

    // It means that we are not participating in a dialog and we can accept
    // segments marked with START flag set to 1.
    switch_state(D7ATP_STATE_IDLE);
    current_dialog_id = 0;
    current_transaction_id = NO_ACTIVE_REQUEST_ID;

    // Discard eventually the Tc timer
    timer_cancel_event(&d7atp_response_period_expired_timer);

    if(current_Tl_received == 0) {
      DPRINT("Tl = 0, stop FG scan");
      // stop the DLL foreground scan
      d7anp_stop_foreground_scan();

      // notify DLL that the dialog is over
      dll_notify_dialog_terminated();
    } else {
      DPRINT("Tl set, don't stop FG scan");
    }
}

void d7atp_stop_transaction()
{
    DPRINT("Current transaction is stopped by upper layer");

    assert(d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD);
    current_transaction_id = NO_ACTIVE_REQUEST_ID;

    // stop the DLL foreground scan
    d7anp_stop_foreground_scan();
}

void d7atp_init()
{
    assert(d7atp_state == D7ATP_STATE_STOPPED);

    d7atp_state = D7ATP_STATE_IDLE;
    current_access_class = ACCESS_CLASS_NOT_SET;
    current_dialog_id = 0;
    current_Tl_received = 0;
    stop_dialog_after_tx = false;
    timer_init_event(&d7atp_response_period_expired_timer, &response_period_timeout_handler);
    timer_init_event(&d7atp_execution_delay_expired_timer, &execution_delay_timeout_handler);

    d7ap_fs_register_file_modified_callback(D7A_FILE_SEL_CONF_FILE_ID, &sel_config_modified_callback);
    sel_config_modified_callback(D7A_FILE_SEL_CONF_FILE_ID);
}

void d7atp_notify_access_profile_file_changed(uint8_t file_id)
{
    // invalidate cached access class
    current_access_class = ACCESS_CLASS_NOT_SET;
}

void d7atp_stop()
{
    d7atp_state = D7ATP_STATE_STOPPED;
    timer_cancel_event(&d7atp_response_period_expired_timer);
    timer_cancel_event(&d7atp_execution_delay_expired_timer);
}

error_t d7atp_send_request(uint8_t dialog_id, uint8_t transaction_id, bool is_last_transaction,
                        packet_t* packet, d7ap_session_qos_t* qos_settings, uint8_t listen_timeout, uint8_t expected_response_length)
{
    // unused parameters
    (void)is_last_transaction;
    (void)listen_timeout;

    /* check that we are not initiating a different dialog if a dialog is still ongoing */
    if (current_dialog_id)
    {
        assert( dialog_id == current_dialog_id);
    }

    if (d7atp_state != D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD)
        switch_state(D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD);

    if ( packet->type == RETRY_REQUEST )
    {
        DPRINT("Retry the transmission with the same packet content");
        current_transaction_id = transaction_id;
        goto send_packet;
    }

    current_dialog_id = dialog_id;
    current_transaction_id = transaction_id;
    packet->d7atp_dialog_id = current_dialog_id;
    packet->d7atp_transaction_id = current_transaction_id;

    uint8_t access_class = packet->d7anp_addressee->access_class;
    if (access_class != current_access_class)
    {
        d7ap_fs_read_access_class(packet->d7anp_addressee->access_specifier, &active_addressee_access_profile);
        current_access_class = access_class;
    }

    DPRINT("Start dialog Id=%i transID=%i on AC=%x, expected resp len=%i", dialog_id, transaction_id, access_class, expected_response_length);

    bool ack_requested = true;
    if ((qos_settings->qos_resp_mode == SESSION_RESP_MODE_NO || qos_settings->qos_resp_mode == SESSION_RESP_MODE_NO_RPT)
        && expected_response_length == 0)
      ack_requested = false;

    // TODO based on what do we calculate Tc? payload length alone is not enough, depends on for example use of FEC, encryption ..
    // keep the same as transmission timeout for now

    // FG scan timeout is set (and scan started) in d7atp_signal_packet_transmitted() for now, to be verified

    packet->d7atp_ctrl = (d7atp_ctrl_t){
        .ctrl_is_start = true,
        .ctrl_is_ack_requested = ack_requested,
        .ctrl_ack_not_void = qos_settings->qos_resp_mode == SESSION_RESP_MODE_ON_ERR? true : false,
        .ctrl_te = false,
        .ctrl_agc = false,
        .ctrl_ack_record = false
    };

    if (ack_requested)
    {
        // TODO payload length does not include headers ... + hardcoded subband
        // TODO this length does not include lower layers overhead for now, use a minimum len of 50 for now ...
        if (expected_response_length < 50)
            expected_response_length = 50;

        uint16_t tx_duration_response = phy_calculate_tx_duration(active_addressee_access_profile.channel_header.ch_class,
                                                                  active_addressee_access_profile.channel_header.ch_coding,
                                                                  expected_response_length, false);
        uint8_t nb = 1;
        if (packet->d7anp_addressee->ctrl.id_type == ID_TYPE_NOID)
            nb = 32;
        else if (packet->d7anp_addressee->ctrl.id_type == ID_TYPE_NBID)
            nb = CT_DECOMPRESS(packet->d7anp_addressee->id[0]);

        // Tc(NB, LEN, CH) = ceil((SFC  * NB  + 1) * TTX(CH, LEN) + TG) with NB the number of concurrent devices and SF the collision Avoidance Spreading Factor
        uint16_t resp_tc = (SFc * nb + 1) * tx_duration_response + t_g;
        packet->d7atp_tc = compress_data(resp_tc, true);

        packet->d7atp_ctrl.ctrl_te = true;
        uint16_t execution_time = 6; // normal flow takes about 6 ticks
        if(active_addressee_access_profile.channel_header.ch_coding == PHY_CODING_FEC_PN9)
            execution_time += (packet->payload_length / 6); // software FEC decoding 6 bytes takes about 1 tick
        if(packet->d7anp_addressee->ctrl.nls_method != AES_NONE)
            execution_time += (packet->payload_length / 27); // software AES decryption 27 bytes takes about 1 tick
        packet->d7atp_te = compress_data(execution_time, true);

        DPRINT("Tc <%i (Ti)> Tc <0x%02x (CT)> Tx duration <%i>", resp_tc, packet->d7atp_tc, tx_duration_response);
    }

send_packet:
    return(d7anp_tx_foreground_frame(packet, true));
}

error_t d7atp_send_response(packet_t* packet)
{
    switch_state(D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE);

    // modify the request headers and turn this into a response
    d7atp_ctrl_t* d7atp = &(packet->d7atp_ctrl);

    // leave ctrl_is_ack_requested as is, keep the requester value
    d7atp->ctrl_ack_not_void = false; // TODO
    d7atp->ctrl_ack_record = false; // TODO validate

    bool should_include_origin_template = false; // we don't need to send origin ID, the requester will filter based on dialogID, but ...

    if (ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type))
        packet->type = RESPONSE_TO_BROADCAST;
    else
        packet->type = RESPONSE_TO_UNICAST;

    if (ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type)
            || (packet->d7atp_ctrl.ctrl_is_start
            && packet->d7atp_ctrl.ctrl_is_ack_requested))
    {
        /*
         * origin template is provided in all requests in which the START flag is set to 1
         * and requesting responses, and in all responses to broadcast requests
         */
        should_include_origin_template = true;
    }

    /* 
     * a setting in the SEL_config file should be able to tell the requester this node should not be put as preferred. This XOFF bit indicates that
     */
    packet->d7atp_ctrl.ctrl_xoff = ctrl_xoff;

    // we are the slave here, so we don't need to lock the other party on the channel, unless we want to signal a pending dormant session with this addressee
    if (packet->d7atp_ctrl.ctrl_is_start) {
        DPRINT("extending dialog");
    }

    // dialog and transaction id remain the same
    DPRINT("Tl=%i", packet->d7atp_tl);
    return (d7anp_tx_foreground_frame(packet, should_include_origin_template));
}

uint8_t d7atp_assemble_packet_header(packet_t* packet, uint8_t* data_ptr)
{
    uint8_t* d7atp_header_start = data_ptr;
    (*data_ptr) = packet->d7atp_ctrl.ctrl_raw; data_ptr++;
    (*data_ptr) = packet->d7atp_dialog_id; data_ptr++;
    (*data_ptr) = packet->d7atp_transaction_id; data_ptr++;
    if (packet->d7atp_ctrl.ctrl_agc) {
        (*data_ptr) = packet->d7atp_target_rx_level_i;
        data_ptr++;
    }

    if (packet->d7atp_ctrl.ctrl_tl) {
        (*data_ptr) = packet->d7atp_tl;
        data_ptr++;
    }

    if (packet->d7atp_ctrl.ctrl_te) {
        (*data_ptr) = packet->d7atp_te;
        data_ptr++;
    }

    if (packet->d7atp_ctrl.ctrl_is_ack_requested && d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD) {
        (*data_ptr) = packet->d7atp_tc;
        data_ptr++;
    }

    //TODO check if at least one Responder has set the ACK_REQ flag
    //TODO aggregate the Device IDs of the Responders that set their ACK_REQ flags.
    // Provide the Responder or Requester ACK template when requested
    if (packet->d7atp_ctrl.ctrl_is_ack_requested && packet->d7atp_ctrl.ctrl_ack_not_void)
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
    if(packet->hw_radio_packet.length < (*data_idx + 3))
    {
        return false;
    }
    packet->d7atp_ctrl.ctrl_raw = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7atp_dialog_id = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7atp_transaction_id = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    if (packet->d7atp_ctrl.ctrl_agc) {
        if(packet->hw_radio_packet.length < (*data_idx + 1))
        {
            return false;
        }
        packet->d7atp_target_rx_level_i = packet->hw_radio_packet.data[(*data_idx)];
        (*data_idx)++;
    }

    if (packet->d7atp_ctrl.ctrl_tl) {
        if(packet->hw_radio_packet.length < (*data_idx + 1))
        {
            return false;
        }
        packet->d7atp_tl = packet->hw_radio_packet.data[(*data_idx)];
        (*data_idx)++;
    }
    else
        packet->d7atp_tl = 0;

    if (packet->d7atp_ctrl.ctrl_te) {
        if(packet->hw_radio_packet.length < (*data_idx + 1))
        {
            return false;
        }
        packet->d7atp_te = packet->hw_radio_packet.data[(*data_idx)];
        (*data_idx)++;
    }
    else
        packet->d7atp_te = 0;

    if ((d7atp_state != D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD) && (packet->d7atp_ctrl.ctrl_is_ack_requested)) {
        if(packet->hw_radio_packet.length < (*data_idx + 1))
        {
            return false;
        }
        packet->d7atp_tc = packet->hw_radio_packet.data[(*data_idx)];
        (*data_idx)++;
    }

    if (packet->d7atp_ctrl.ctrl_is_ack_requested && packet->d7atp_ctrl.ctrl_ack_not_void)
    {
        if(packet->hw_radio_packet.length < (*data_idx + 2))
        {
            return false;
        }
        packet->d7atp_ack_template.ack_transaction_id_start = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
        packet->d7atp_ack_template.ack_transaction_id_stop = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
        // TODO ACK bitmap, support for multiple segments to ack not implemented yet
    }

    return true;
}

void d7atp_signal_packet_transmitted(packet_t* packet)
{
    d7asp_signal_packet_transmitted(packet);

    if (d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD)
    {
        switch_state(D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD);

        if (packet->d7atp_ctrl.ctrl_is_ack_requested)
        {
            timer_tick_t Tc = CT_DECOMPRESS(packet->d7atp_tc);

            // Check if an Execution Delay period needs to be observed
            if (packet->d7atp_ctrl.ctrl_te)
            {
                timer_tick_t Te = adjust_timeout_value(CT_DECOMPRESS(packet->d7atp_te), packet->hw_radio_packet.tx_meta.timestamp);
                if (Te)
                {
                    d7anp_set_foreground_scan_timeout(Tc + 2); // we include Tt here for now

                    d7atp_execution_delay_expired_timer.next_event = Te;
                    timer_add_event(&d7atp_execution_delay_expired_timer);
                    return;
                }
                // if the the time passed since transmission is greater than Te, Tc is updated to include Te
                // and the foreground scan is started immediately
                else
                    Tc += CT_DECOMPRESS(packet->d7atp_te);
            }

            Tc = adjust_timeout_value( Tc, packet->hw_radio_packet.tx_meta.timestamp);
            d7anp_set_foreground_scan_timeout(Tc + 2); // we include Tt here for now
            d7anp_start_foreground_scan();
        }
        else
        {
            current_transaction_id = NO_ACTIVE_REQUEST_ID;
            d7asp_signal_transaction_terminated();
        }
    }
    else if (d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE)
    {
        switch_state(D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD);

        if (stop_dialog_after_tx)
        {
            // no FG scan required after transaction, we can end dialog now
            d7anp_stop_foreground_scan();
            terminate_dialog();
        }
    }
    else if (d7atp_state == D7ATP_STATE_IDLE)
        assert(!packet->d7atp_ctrl.ctrl_is_ack_requested); // can only occur in this case
}

void d7atp_signal_transmission_failure()
{
    assert((d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_REQUEST_PERIOD) ||
           (d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE));

    DPRINT("CSMA-CA insertion failed, stopping transaction");

    if (d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_SENDING_RESPONSE && stop_dialog_after_tx)
    {
        /* For Slaves, terminate the dialog if no FG scan and no response period are scheduled.*/
        d7anp_stop_foreground_scan();
        terminate_dialog();
        return;
    }

    d7asp_signal_transmission_failure();
}

void d7atp_process_received_packet(packet_t* packet)
{
    bool extension = false;
    timer_tick_t Tc;

    assert(d7atp_state == D7ATP_STATE_MASTER_TRANSACTION_RESPONSE_PERIOD
           || d7atp_state == D7ATP_STATE_SLAVE_TRANSACTION_RESPONSE_PERIOD
           || d7atp_state == D7ATP_STATE_IDLE); // IDLE: when doing channel scanning outside of transaction

    // copy addressee from NP origin
    current_addressee.ctrl.id_type = packet->d7anp_ctrl.origin_id_type;
    current_addressee.access_class = packet->origin_access_class;
    DPRINT("ORI AC=0x%02x", packet->origin_access_class);
    memcpy(current_addressee.id, packet->origin_access_id, 8);
    packet->d7anp_addressee = &current_addressee;

    DPRINT("Recvd dialog %i trans id %i, curr %i - %i", packet->d7atp_dialog_id, packet->d7atp_transaction_id, current_dialog_id, current_transaction_id);

    if (packet->d7atp_tl)
        current_Tl_received = CT_DECOMPRESS(packet->d7atp_tl);
    else
        current_Tl_received = 0;
    DPRINT("Tl=%i (CT) -> %i (Ti) ", packet->d7atp_tl, current_Tl_received);
    if (IS_IN_MASTER_TRANSACTION())
    {
        if (packet->d7atp_dialog_id != current_dialog_id || packet->d7atp_transaction_id != current_transaction_id)
        {
            DPRINT("Unexpected dialog ID or transaction ID received, skipping segment");
            packet_queue_free_packet(packet);
            return;
        }

        // Check if a new dialog initiated by the responder is allowed
        if (packet->d7atp_ctrl.ctrl_is_start)
        {
            // if this is a unicast response and the last transaction, the extension procedure is allowed
            // TODO validate this is still working now we don't have the stop bit any more
            if (!ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type))
            {
                current_Tl_received = adjust_timeout_value(current_Tl_received, packet->hw_radio_packet.rx_meta.timestamp);
                DPRINT("Adjusted Tl=%i (Ti) ", current_Tl_received);
                DPRINT("Responder wants to append a new dialog");
                d7anp_set_foreground_scan_timeout(current_Tl_received);
                d7anp_start_foreground_scan();
                current_dialog_id = 0;
                switch_state(D7ATP_STATE_IDLE);
                extension = true;
            }
            else
            {
                log_print_error_string("Start dialog not allowed when in master transaction state, skipping segment");
                packet_queue_free_packet(packet);
                return;
            }
        }

        d7asp_process_received_response(packet, extension);
    }
    else
    {
         /*
          * when participating in a Dialog, responder discards segments with Dialog ID
          * not matching the recorded Dialog ID
          */
        if ((current_dialog_id) && (current_dialog_id != packet->d7atp_dialog_id))
        {
            log_print_error_string("Filtered frame with Dialog ID not matching the recorded Dialog ID");
            packet_queue_free_packet(packet);
            return;
        }

        // When not participating in a Dialog
        if ((!current_dialog_id) && (!packet->d7atp_ctrl.ctrl_is_start))
        {
            //Responders discard segments marked with START flag set to 0 until they receive a segment with START flag set to 1
            log_print_error_string("Filtered frame with START cleared");
            packet_queue_free_packet(packet);
            return;
        }

         // The FG scan is only started when the response period expires.
        if (packet->d7atp_ctrl.ctrl_is_ack_requested)
        {
            Tc = CT_DECOMPRESS(packet->d7atp_tc);

            DPRINT("Tc=%i (CT) -> %i (Ti) ", packet->d7atp_tc, Tc);

            // extend Tc with Te
            if (packet->d7atp_ctrl.ctrl_te)
                Tc += CT_DECOMPRESS(packet->d7atp_te);

            // We choose to start the FG scan after the execution delay and the response period so Tl = Tl - Tc - Te
            if (current_Tl_received > Tc)
                current_Tl_received -= Tc;
            else
            {
                current_Tl_received = 0;
                // If there is no listen period, then we can end the dialog after the response transmission
                stop_dialog_after_tx = true;
            }


            Tc = adjust_timeout_value(Tc, packet->hw_radio_packet.rx_meta.timestamp);

            if (Tc == 0)
            {
                log_print_error_string("Discard the request since the response period is expired");
                packet_queue_free_packet(packet);
                return;
            }

            /* stop eventually the FG scan and force the radio to go back to IDLE */
            d7anp_stop_foreground_scan();
        }
        else
        {
            current_Tl_received = adjust_timeout_value(current_Tl_received, packet->hw_radio_packet.rx_meta.timestamp);
            if (current_Tl_received > 0)
            {
                d7anp_set_foreground_scan_timeout(current_Tl_received);
                d7anp_start_foreground_scan();
            }
        }

        switch_state(D7ATP_STATE_SLAVE_TRANSACTION_RECEIVED_REQUEST);

        current_dialog_id = packet->d7atp_dialog_id;
        current_transaction_id = packet->d7atp_transaction_id;

        // store the received timestamp for later usage (eg CCA). the rx_meta.timestamp can be
        // overwritten since it is stored in a union with tx_meta and can thus be changed when
        // trying to transmit
        packet->request_received_timestamp = packet->hw_radio_packet.rx_meta.timestamp;

        // set active_addressee_access_profile to the access_profile supplied by the requester
        if (current_access_class != current_addressee.access_class)
        {
            d7ap_fs_read_access_class(current_addressee.access_specifier, &active_addressee_access_profile);
            current_access_class = current_addressee.access_class;
        }

        // DLL is taking care that we respond on the channel where we received the request on

        bool response_payload_expected = d7asp_process_received_packet(packet);

        if (packet->d7atp_ctrl.ctrl_is_ack_requested)
        {
            if (response_payload_expected)
            {
                // wait for response payload, should be sent before expiration of the response period
                schedule_response_period_timeout_handler(Tc);
            }
            else
            {
                // If a listen period is set, start the foreground scan after the response period
                if (current_Tl_received)
                    schedule_response_period_timeout_handler(Tc);

                // if ACK requested and no response payload expected, send the ack now
                packet->payload_length = 0;
                d7atp_send_response(packet);
            }
        }
        else if (current_Tl_received == 0)
            terminate_dialog();
    }
}
