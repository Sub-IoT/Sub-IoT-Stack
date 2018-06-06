/*! \file d7asp.c
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
 */

#include "string.h"
#include "debug.h"
#include "ng.h"
#include "log.h"
#include "bitmap.h"
#include "d7asp.h"
#include "alp_layer.h"
#include "fs.h"
#include "scheduler.h"
#include "d7atp.h"
#include "packet_queue.h"
#include "packet.h"
#include "hwdebug.h"
#include "random.h"
#include "hwwatchdog.h"
#include "MODULE_D7AP_defs.h"
#include "errors.h"
#include "compress.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_SP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_SESSION, __VA_ARGS__)
#else
#define DPRINT(...)
#endif


struct d7asp_master_session {
    d7ap_master_session_config_t config;
    // TODO uint8_t dorm_timer;
    d7asp_master_session_state_t state;
    uint8_t token;
    uint8_t progress_bitmap[REQUESTS_BITMAP_BYTE_COUNT];
    uint8_t success_bitmap[REQUESTS_BITMAP_BYTE_COUNT];
    uint8_t next_request_id;
    uint8_t request_buffer_tail_idx;
    uint8_t requests_indices[MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT]; /**< Contains for every request ID the index in command_buffer the index where the request begins */
    uint8_t requests_lengths[MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT]; /**< Contains for every request ID the index in command_buffer the length of the ALP payload in that request */
    uint8_t response_lengths[MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT]; /**< Contains for every request ID the index in command_buffer the expected length of the ALP response for the specific request */
    uint8_t request_buffer[MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE];
};

static d7asp_master_session_t NGDEF(_current_master_session); // TODO we only use 1 fifo for now, should be multiple later (1 per on unique addressee and QoS combination)
#define current_master_session NG(_current_master_session)

static uint8_t NGDEF(_current_request_id); // TODO move ?
#define current_request_id NG(_current_request_id)

static uint8_t NGDEF(_current_request_retry_count);
#define current_request_retry_count NG(_current_request_retry_count)

static packet_t* NGDEF(_current_request_packet);
#define current_request_packet NG(_current_request_packet)

static uint8_t NGDEF(_single_request_retry_limit);
#define single_request_retry_limit NG(_single_request_retry_limit)

static packet_t* NGDEF(_current_response_packet);
#define current_response_packet NG(_current_response_packet)

typedef enum {
    D7ASP_STATE_STOPPED,
    D7ASP_STATE_IDLE,
    D7ASP_STATE_SLAVE,
    D7ASP_STATE_MASTER,
    D7ASP_STATE_SLAVE_PENDING_MASTER,
    D7ASP_STATE_PENDING_MASTER
} state_t;

static state_t NGDEF(_state) = D7ASP_STATE_STOPPED;
#define d7asp_state NG(_state)

static void switch_state(state_t new_state);

static void mark_current_request_done()
{
    bitmap_set(current_master_session.progress_bitmap, current_request_id);
    // current_request_packet will be free-ed in the packet_queue when the transaction is completed
}

static void mark_current_request_successful()
{
    bitmap_set(current_master_session.success_bitmap, current_request_id);
}

static void init_master_session(d7asp_master_session_t* session) {
    session->state = D7ASP_MASTER_SESSION_IDLE;
    session->token = get_rnd() % 0xFF;
    memset(session->progress_bitmap, 0x00, REQUESTS_BITMAP_BYTE_COUNT);
    memset(session->success_bitmap, 0x00, REQUESTS_BITMAP_BYTE_COUNT);
    session->next_request_id = 0;
    session->request_buffer_tail_idx = 0;
    memset(session->requests_indices, 0x00, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
    memset(session->requests_lengths, 0x00, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
    memset(session->response_lengths, 255, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
    memset(session->request_buffer, 0x00, MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE);
}

static void flush_completed() {
    DPRINT("FIFO flush completed");

    // TODO When a Session does not terminate on success, the Session is automatically re-activated using
    // the RETRY_MODE pattern defined in the Configuration file

    // single flush of the FIFO without retry
    alp_layer_d7asp_fifo_flush_completed(current_master_session.token, current_master_session.progress_bitmap,
                                   current_master_session.success_bitmap, current_master_session.next_request_id - 1);
    init_master_session(&current_master_session);
    current_master_session.state = D7ASP_MASTER_SESSION_IDLE;
    d7atp_signal_dialog_termination();
    switch_state(D7ASP_STATE_IDLE);
}

static void flush_fifos()
{
    error_t ret;

    if (d7asp_state != D7ASP_STATE_MASTER && d7asp_state != D7ASP_STATE_PENDING_MASTER)
    {
        DPRINT("Flushing FIFOs can't be executed in this state <%d>", d7asp_state);
        return;
    }

    if(current_master_session.state != D7ASP_MASTER_SESSION_PENDING) {
      DPRINT("No sessions in pending state, skipping");
      return;
    }

    current_master_session.state = D7ASP_MASTER_SESSION_ACTIVE;
    if (d7asp_state == D7ASP_STATE_PENDING_MASTER)
        switch_state(D7ASP_STATE_MASTER);

    DPRINT("Flushing FIFOs");
    hw_watchdog_feed(); // TODO do here?

    if (current_request_id == NO_ACTIVE_REQUEST_ID)
    {
        // find first request which is not acked or dropped
        int8_t found_next_req_index = bitmap_search(current_master_session.progress_bitmap, false, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
        if (found_next_req_index == -1 || found_next_req_index == current_master_session.next_request_id)
        {
            // we handled all requests ...
            flush_completed();
            return;
        }

        current_request_id = found_next_req_index;
        current_request_retry_count = 0;

        current_request_packet = packet_queue_alloc_packet();
        assert(current_request_packet);
        packet_queue_mark_processing(current_request_packet);
        current_request_packet->d7anp_addressee = &(current_master_session.config.addressee); // TODO explicitly pass addressee down the stack layers?

        memcpy(current_request_packet->payload, current_master_session.request_buffer + current_master_session.requests_indices[current_request_id], current_master_session.requests_lengths[current_request_id]);
        current_request_packet->payload_length = current_master_session.requests_lengths[current_request_id];

        if (current_request_id == 0)
            current_request_packet->type = INITIAL_REQUEST;
        else
            current_request_packet->type =  SUBSEQUENT_REQUEST;

        // TODO calculate Tl
        // Tl should correspond to the maximum time needed to send the remaining requests in the FIFO including the RETRY parameter
    }
    else
    {
        // retrying request ...
        DPRINT("Current request retry count: %i", current_request_retry_count);
        if (current_request_retry_count == single_request_retry_limit)
        {
            // mark request as failed and pop
            mark_current_request_done();
            DPRINT("Request reached single request retry limit (%i), skipping request", single_request_retry_limit);
            packet_queue_free_packet(current_request_packet);
            current_request_id = NO_ACTIVE_REQUEST_ID;
            sched_post_task(&flush_fifos); // continue flushing until all request handled ...
            return;
        }

        packet_queue_mark_processing(current_request_packet);
        current_request_packet->type = RETRY_REQUEST;
        // TODO stop on error
    }

    uint8_t listen_timeout = 0; // TODO calculate timeout (and update during transaction lifetime) (based on Tc, channel, cs, payload size, # msgs, # retries)
    ret = d7atp_send_request(current_master_session.token, current_request_id, (current_request_id == current_master_session.next_request_id - 1),
                       current_request_packet, &current_master_session.config.qos, listen_timeout, current_master_session.response_lengths[current_request_id]);
    if (ret == EPERM)
    {
        // this is probably because no further encryption is possible (frame counter reaches the maximum value)
        // TODO return an error code to the application?
        DPRINT("Request sending is not allowed likely because frame counter reaches its maximum value");
    }
}

// TODO document state diagram
static void switch_state(state_t new_state)
{
    switch(new_state)
    {
        case D7ASP_STATE_MASTER:
            switch(d7asp_state)
            {
                case D7ASP_STATE_IDLE:
                    d7asp_state = new_state;
                    sched_post_task(&flush_fifos);
                    DPRINT("Switching to state D7ASP_STATE_MASTER");
                    break;
                case D7ASP_STATE_SLAVE_PENDING_MASTER:
                case D7ASP_STATE_PENDING_MASTER:
                    d7asp_state = new_state;
                    DPRINT("Switching to state D7ASP_STATE_MASTER");
                    break;
                case D7ASP_STATE_SLAVE:
                    d7asp_state = D7ASP_STATE_SLAVE_PENDING_MASTER;
                    DPRINT("Switching to state D7ASP_STATE_SLAVE_PENDING_MASTER");
                    break;
                case D7ASP_STATE_MASTER:
                    // new requests in fifo, reschedule for later flushing
                    // TODO sched_post_task(&flush_fifos);
                    break;
                default:
                    assert(false);
            }

            break;
        case D7ASP_STATE_SLAVE:
            switch(d7asp_state)
            {
                case D7ASP_STATE_IDLE:
                case D7ASP_STATE_MASTER:
                    d7asp_state = new_state;
                    current_request_id = NO_ACTIVE_REQUEST_ID;
                    DPRINT("Switching to state D7ASP_STATE_SLAVE");
                    break;
                default:
                    assert(false);
            }
            break;
        case D7ASP_STATE_SLAVE_PENDING_MASTER:
            assert(d7asp_state == D7ASP_STATE_SLAVE || d7asp_state == D7ASP_STATE_PENDING_MASTER);
            d7asp_state = D7ASP_STATE_SLAVE_PENDING_MASTER;
            DPRINT("Switching to state D7ASP_STATE_SLAVE_PENDING_MASTER");
            break;
        case D7ASP_STATE_PENDING_MASTER:
            assert(d7asp_state == D7ASP_STATE_IDLE || d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER);
            d7asp_state = D7ASP_STATE_PENDING_MASTER;
            DPRINT("Switching to state D7ASP_STATE_PENDING_MASTER");
            break;
        case D7ASP_STATE_IDLE:
            d7asp_state = new_state;
            current_request_id = NO_ACTIVE_REQUEST_ID;
            DPRINT("Switching to state D7ASP_STATE_IDLE");
            break;
        default:
            assert(false);
    }
}

void d7asp_init()
{
    assert(d7asp_state == D7ASP_STATE_STOPPED);

    d7asp_state = D7ASP_STATE_IDLE;
    current_request_id = NO_ACTIVE_REQUEST_ID;

    current_master_session.state = D7ASP_MASTER_SESSION_IDLE;
    DPRINT("REQUESTS_BITMAP_BYTE_COUNT %d", REQUESTS_BITMAP_BYTE_COUNT);
    DPRINT("FIFO_MAX_REQUESTS_COUNT %d", MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);

    sched_register_task(&flush_fifos);
}

void d7asp_stop()
{
    d7asp_state = D7ASP_STATE_STOPPED;
    timer_cancel_task(&flush_fifos);
    sched_cancel_task(&flush_fifos);
}

d7asp_master_session_t* d7asp_master_session_create(d7ap_master_session_config_t* d7asp_master_session_config) {
    // TODO for now we assume only one concurrent session, in the future we should dynamically allocate (or return from pool) a session

    if (current_master_session.state != D7ASP_MASTER_SESSION_IDLE)
    {
        // Requests can be pushed in the FIFO by upper layer anytime
        if ((current_master_session.config.addressee.access_class == d7asp_master_session_config->addressee.access_class) &&
            (current_master_session.config.addressee.ctrl.raw == d7asp_master_session_config->addressee.ctrl.raw) &&
            memcmp(current_master_session.config.addressee.id, d7asp_master_session_config->addressee.id, alp_addressee_id_length(d7asp_master_session_config->addressee.ctrl.id_type)));
        return &current_master_session;

        // TODO create a pending session or a dormant session if TO (DORM_TIMER) !=0
    }

    init_master_session(&current_master_session);

    DPRINT("Create master session %d", current_master_session.token);

    current_master_session.config.qos = d7asp_master_session_config->qos;
    current_master_session.config.dormant_timeout = d7asp_master_session_config->dormant_timeout;
    current_master_session.config.addressee.ctrl = d7asp_master_session_config->addressee.ctrl;
    current_master_session.config.addressee.access_class = d7asp_master_session_config->addressee.access_class;
    memcpy(current_master_session.config.addressee.id, d7asp_master_session_config->addressee.id, sizeof(current_master_session.config.addressee.id));

    //TODO actually use dormant timeout. For now it is infinite
    if(current_master_session.config.dormant_timeout)
      current_master_session.state = D7ASP_MASTER_SESSION_DORMANT;

    return &current_master_session;
}

// TODO we assume a fifo contains only ALP commands, but according to spec this can be any kind of "Request"
// we will see later what this means. For instance how to add a request which starts D7AAdvP etc
d7asp_queue_result_t d7asp_queue_alp_actions(d7asp_master_session_t* session, uint8_t* alp_payload_buffer, uint8_t alp_payload_length, uint8_t expected_alp_response_length)
{
    DPRINT("Queuing ALP actions");
    // TODO can be called in all session states?
    assert(session == &current_master_session); // TODO tmp
    assert(session->request_buffer_tail_idx + alp_payload_length < MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE);
    assert(session->next_request_id < MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT); // TODO do not assert but let upper layer handle this
    assert(!(expected_alp_response_length > 0 &&
             (session->config.qos.qos_resp_mode == SESSION_RESP_MODE_NO || session->config.qos.qos_resp_mode == SESSION_RESP_MODE_NO_RPT))); // TODO return error
    single_request_retry_limit = 1; // TODO read from SEL config file

    // add request to buffer
    // TODO request can contain 1 or more ALP commands, find a way to group commands in requests instead of dumping all requests in one buffer
    uint8_t request_id = session->next_request_id;
    session->requests_indices[request_id] = session->request_buffer_tail_idx;
    session->requests_lengths[request_id] = alp_payload_length;
    session->response_lengths[request_id] = expected_alp_response_length;
    memcpy(session->request_buffer + session->request_buffer_tail_idx, alp_payload_buffer, alp_payload_length);
    session->request_buffer_tail_idx += alp_payload_length + 1;
    session->next_request_id++;

    // TODO for master only set to pending when asked by upper layer (ie new function call)
    if (d7asp_state == D7ASP_STATE_IDLE)
    {
        switch_state(D7ASP_STATE_PENDING_MASTER);
        sched_post_task(&flush_fifos);
    }
    else if (d7asp_state == D7ASP_STATE_SLAVE)
        switch_state(D7ASP_STATE_SLAVE_PENDING_MASTER);

    if(current_master_session.state == D7ASP_MASTER_SESSION_IDLE) {
      current_master_session.state = D7ASP_MASTER_SESSION_PENDING;
      DPRINT("converting IDLE session to PENDING");
    } else if(current_master_session.state == D7ASP_MASTER_SESSION_DORMANT) {
      DPRINT("session is dormant, not activating");
    }

    return (d7asp_queue_result_t){ .fifo_token = session->token, .request_id = request_id };
}

bool d7asp_process_received_packet(packet_t* packet, bool extension)
{
    hw_watchdog_feed(); // TODO do here?
    d7ap_session_result_t result = {
        .channel = packet->hw_radio_packet.rx_meta.rx_cfg.channel_id,
        .rx_level =  - packet->hw_radio_packet.rx_meta.rssi,
        .link_budget = (packet->dll_header.control_eirp_index - 32) - packet->hw_radio_packet.rx_meta.rssi,
        .target_rx_level = 80, // TODO not implemented yet, use default for now
        .status = {
            .ucast = 0, // TODO
            .nls = (packet->d7anp_ctrl.nls_method ? true : false),
            .retry = false, // TODO
            .missed = false, // TODO
        },
        .response_to = packet->d7atp_tc,
        .addressee = *packet->d7anp_addressee
        // .fifo_token and .seqnr filled below
    };

    if (d7asp_state == D7ASP_STATE_MASTER)
    {
        assert(packet->d7atp_dialog_id == current_master_session.token);
        assert(packet->d7atp_transaction_id == current_request_id);

        // received ack
        DPRINT("Received ACK");
        if (current_master_session.config.qos.qos_resp_mode != SESSION_RESP_MODE_NO
           && current_master_session.config.qos.qos_resp_mode != SESSION_RESP_MODE_NO_RPT)
        {
            // for SESSION_RESP_MODE_NO and SESSION_RESP_MODE_NO_RPT the request was already marked as done
            // upon successfull CSMA insertion. We don't care about response in these cases.

            result.fifo_token = current_master_session.token;
            result.seqnr = current_request_id;
            mark_current_request_successful();
            mark_current_request_done();
            assert(packet != current_request_packet);
        }

        alp_layer_process_d7asp_result(packet->payload, packet->payload_length, packet->payload, &packet->payload_length, result);

        packet_queue_free_packet(packet); // ACK can be cleaned

        /* In case of unicast session, it is acceptable to switch to the next request before the expiration of Tc */
        if (!ID_TYPE_IS_BROADCAST(current_master_session.config.addressee.ctrl.id_type))
        {
            DPRINT("Request completed, don't wait end of transaction");
            packet_queue_free_packet(current_request_packet);

            // terminate the dialog if all request handled
            // we need to switch to the state idle otherwise we may receive a new packet before the task flush_fifos is handled
            // in this case, we may assert since the state remains MASTER
            if (current_request_id == current_master_session.next_request_id - 1)
            {
                flush_completed();
                return false;
            }
            current_request_id = NO_ACTIVE_REQUEST_ID;
            sched_post_task(&flush_fifos); // continue flushing until all request handled ...
            // stop the current transaction
            d7atp_stop_transaction();
        }
        // switch to the state slave when the D7ATP Dialog Extension Procedure is initiated and all request are handled
        else if ((extension) && (current_request_id == current_master_session.next_request_id - 1))
        {
            DPRINT("Dialog Extension Procedure is initiated, mark the FIFO flush"
                    " completed before switching to a responder state");
            packet_queue_free_packet(current_request_packet);
            alp_layer_d7asp_fifo_flush_completed(current_master_session.token, current_master_session.progress_bitmap,
                                           current_master_session.success_bitmap, current_master_session.next_request_id - 1);
            current_master_session.state = D7ASP_MASTER_SESSION_IDLE;
            switch_state(D7ASP_STATE_SLAVE);
        }
        return false;
    }
    else if (d7asp_state == D7ASP_STATE_IDLE
            || d7asp_state == D7ASP_STATE_SLAVE
            || d7asp_state == D7ASP_STATE_PENDING_MASTER
            || d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER)
    {
        // received a request, start slave session, process and respond
        if (d7asp_state == D7ASP_STATE_IDLE)
            switch_state(D7ASP_STATE_SLAVE); // don't switch when already in slave state
        else if (d7asp_state == D7ASP_STATE_PENDING_MASTER)
            switch_state(D7ASP_STATE_SLAVE_PENDING_MASTER);

        result.fifo_token = packet->d7atp_dialog_id;
        result.seqnr = packet->d7atp_transaction_id;

        if (packet->payload_length > 0)
        {
            alp_layer_process_d7asp_result(packet->payload, packet->payload_length, packet->payload, &packet->payload_length, result);
        }

        // execute slave transaction
        if (!packet->d7atp_ctrl.ctrl_is_ack_requested)
            goto discard_request; // no need to respond, clean up

        DPRINT("Sending response");

        current_response_packet = packet;

        if (current_master_session.state == D7ASP_MASTER_SESSION_DORMANT &&
            (!ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type)) &&
            memcmp(current_master_session.config.addressee.id, packet->d7anp_addressee->id, alp_addressee_id_length(packet->d7anp_addressee->ctrl.id_type)) == 0) {
          DPRINT("pending dormant session for requester");
          current_master_session.state = D7ASP_MASTER_SESSION_PENDING;
        }

        /*
         * activate the dialog extension procedure in the unicast response if the dialog is terminated
         * and a master session is pending
         */
        if ((!ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type)) &&
                (d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER))
        {
            packet->d7atp_ctrl.ctrl_is_start = true;
            packet->d7atp_ctrl.ctrl_tl = true;
            packet->d7atp_tl = compress_data(10240, true); // TODO set according the time remaining in the current transaction
                                                           // + the maximum time to send the first request of the pending session.
        }
        else
            packet->d7atp_ctrl.ctrl_is_start = 0;

        return true;
    }
    else
        assert(false);

    discard_request:
        packet_queue_free_packet(packet);
        return false;
}

static void on_request_completed()
{
    assert(d7asp_state == D7ASP_STATE_MASTER);
    if (!bitmap_get(current_master_session.progress_bitmap, current_request_id))
    {
        current_request_retry_count++;
        // the request may be retransmitted, don't free yet (this will be done in flush_fifo() when failed)
    }
    else
    {
        // request completed, no retries needed so we can free the packet
        packet_queue_free_packet(current_request_packet);

        // terminate the dialog if all request handled
        // we need to switch to the state idle otherwise we may receive a new packet before the task flush_fifos is handled
        // in this case, we may assert since the state remains MASTER
        if (current_request_id == current_master_session.next_request_id - 1)
        {
            flush_completed();
            return;
        }
        current_request_id = NO_ACTIVE_REQUEST_ID;
    }

    sched_post_task(&flush_fifos); // continue flushing until all request handled ...
}

void d7asp_signal_packet_transmitted(packet_t *packet)
{
    DPRINT("Packet transmitted");
    if (d7asp_state == D7ASP_STATE_MASTER)
    {
        // for the lowest QoS level the packet is ack-ed when CSMA/CA process succeeded
        if (current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_NO ||
           current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_NO_RPT)
        {
            mark_current_request_done();
            mark_current_request_successful();
        }
    }
    else if (d7asp_state == D7ASP_STATE_SLAVE || d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER)
    {
        assert(current_response_packet == packet);

        // when in slave session we can immediately cleanup the transmitted response.
        // requests (in master sessions) will be cleanup upon termination of the dialog.
        current_response_packet = NULL;
        packet_queue_free_packet(packet);
    }
}

void d7asp_signal_transmission_failure()
{
    if (d7asp_state == D7ASP_STATE_MASTER)
        on_request_completed();
    else if (d7asp_state == D7ASP_STATE_SLAVE ||
             d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER)
    {
        packet_queue_free_packet(current_response_packet);
        current_response_packet = NULL;
    }
}

void d7asp_signal_transaction_terminated()
{
    assert(d7asp_state == D7ASP_STATE_MASTER);

    on_request_completed();
}

void d7asp_signal_dialog_terminated()
{
    assert(d7asp_state == D7ASP_STATE_SLAVE ||
           d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER);

    if (current_response_packet)
    {
        DPRINT("Discard the response since the dialog is now terminated");
        packet_queue_free_packet(current_response_packet);
        current_response_packet = NULL;
    }

    if (d7asp_state == D7ASP_STATE_SLAVE)
        switch_state(D7ASP_STATE_IDLE);
    else if (d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER)
    {
        switch_state(D7ASP_STATE_PENDING_MASTER);
        sched_post_task(&flush_fifos);
    }
}
