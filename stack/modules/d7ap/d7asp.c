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
#include "alp.h"
#include "fs.h"
#include "scheduler.h"
#include "d7atp.h"
#include "packet_queue.h"
#include "packet.h"
#include "hwdebug.h"
#include "random.h"
#include "hwwatchdog.h"
#include "MODULE_D7AP_defs.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_SP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_SESSION, __VA_ARGS__)
#else
#define DPRINT(...)
#endif

struct d7asp_master_session {
    d7asp_master_session_config_t config;
    // TODO uint8_t dorm_timer;
    d7asp_master_session_state_t state;
    uint8_t token;
    uint8_t progress_bitmap[REQUESTS_BITMAP_BYTE_COUNT];
    uint8_t success_bitmap[REQUESTS_BITMAP_BYTE_COUNT];
    uint8_t next_request_id;
    uint8_t request_buffer_tail_idx;
    uint8_t requests_indices[MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT]; /**< Contains for every request ID the index in command_buffer the index where the request begins */
    uint8_t requests_lengths[MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT]; /**< Contains for every request ID the index in command_buffer the length of the ALP payload in that request */
    uint8_t request_buffer[MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE];
};

static d7asp_master_session_t NGDEF(_current_master_session); // TODO we only use 1 fifo for now, should be multiple later (1 per on unique addressee and QoS combination)
#define current_master_session NG(_current_master_session)

static uint8_t NGDEF(_current_request_id); // TODO move ?
#define current_request_id NG(_current_request_id)

#define NO_ACTIVE_REQUEST_ID 0xFF

static uint8_t NGDEF(_current_request_retry_count);
#define current_request_retry_count NG(_current_request_retry_count)

static packet_t* NGDEF(_current_request_packet);
#define current_request_packet NG(_current_request_packet)

static uint8_t NGDEF(_single_request_retry_limit);
#define single_request_retry_limit NG(_single_request_retry_limit)

static d7asp_init_args_t* NGDEF(_d7asp_init_args);
#define d7asp_init_args NG(_d7asp_init_args)

typedef enum {
    D7ASP_STATE_IDLE,
    D7ASP_STATE_SLAVE,
    D7ASP_STATE_MASTER,
    D7ASP_STATE_SLAVE_PENDING_MASTER
} state_t;

static state_t NGDEF(_state);
#define d7asp_state NG(_state)

static void switch_state(state_t new_state);

static void mark_current_request_done()
{
    bitmap_set(current_master_session.progress_bitmap, current_request_id);
    // current_request_packet will be free-ed in the packet_queue when the transaction is completed
}

static void flush_fifos()
{
    assert(d7asp_state == D7ASP_STATE_MASTER);
    DPRINT("Flushing FIFOs");
    hw_watchdog_feed(); // TODO do here?

    if(current_request_id == NO_ACTIVE_REQUEST_ID)
    {
        // find first request which is not acked or dropped
        int8_t found_next_req_index = bitmap_search(current_master_session.progress_bitmap, false, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
        if(found_next_req_index == -1 || found_next_req_index == current_master_session.next_request_id)
        {
            // we handled all requests ...
            DPRINT("FIFO flush completed");
            alp_d7asp_fifo_flush_completed(current_master_session.token, current_master_session.progress_bitmap, current_master_session.success_bitmap, REQUESTS_BITMAP_BYTE_COUNT);
            // TODO move callback to ALP?
//            if(d7asp_init_args != NULL && d7asp_init_args->d7asp_fifo_flush_completed_cb != NULL)
//                d7asp_init_args->d7asp_fifo_flush_completed_cb(fifo.token, fifo.progress_bitmap, fifo.success_bitmap, REQUESTS_BITMAP_BYTE_COUNT);


            current_master_session.state = D7ASP_MASTER_SESSION_IDLE;
            switch_state(D7ASP_STATE_IDLE);
            return;
        }

        current_request_id = found_next_req_index;
        current_request_retry_count = 0;

        current_request_packet = packet_queue_alloc_packet();
        packet_queue_mark_processing(current_request_packet);
        current_request_packet->d7anp_addressee = &(current_master_session.config.addressee); // TODO explicitly pass addressee down the stack layers?

        memcpy(current_request_packet->payload, current_master_session.request_buffer + current_master_session.requests_indices[current_request_id], current_master_session.requests_lengths[current_request_id]);
        current_request_packet->payload_length = current_master_session.requests_lengths[current_request_id];
    }
    else
    {
        // retrying request ...
        DPRINT("Current request retry count: %i", current_request_retry_count);
        if(current_request_retry_count == single_request_retry_limit)
        {
            // mark request as failed and pop
            mark_current_request_done();
            DPRINT("Request reached single request retry limit (%i), skipping request", single_request_retry_limit);
            packet_queue_free_packet(current_request_packet);
            current_request_id = NO_ACTIVE_REQUEST_ID;
            sched_post_task(&flush_fifos); // continue flushing until all request handled ...
            return;
        }

        // TODO stop on error
    }

    // TODO calculate D7ANP timeout (and update during transaction lifetime) (based on Tc, channel, cs, payload size, # msgs, # retries)
    d7atp_start_dialog(current_master_session.token, current_request_id, true, current_request_packet, &current_master_session.config.qos);
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
                    d7asp_state = new_state;
                    sched_post_task(&flush_fifos);
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
                    d7asp_state = new_state;
                    current_request_id = NO_ACTIVE_REQUEST_ID;
                    DPRINT("Switching to state D7ASP_STATE_SLAVE");
                    break;
                default:
                    assert(false);
            }
            break;
        case D7ASP_STATE_SLAVE_PENDING_MASTER:
            assert(d7asp_state == D7ASP_STATE_SLAVE);
            d7asp_state = D7ASP_STATE_SLAVE_PENDING_MASTER;
            DPRINT("Switching to state D7ASP_STATE_SLAVE_PENDING_MASTER");
            break;
        case D7ASP_STATE_IDLE:
            d7asp_state = new_state;
            DPRINT("Switching to state D7ASP_STATE_IDLE");
            break;
        default:
            assert(false);
    }
}

void d7asp_init(d7asp_init_args_t* init_args)
{
    d7asp_state = D7ASP_STATE_IDLE;
    d7asp_init_args = init_args;
    current_request_id = NO_ACTIVE_REQUEST_ID;

    current_master_session.state = D7ASP_MASTER_SESSION_IDLE;

    sched_register_task(&flush_fifos);
}

d7asp_master_session_t* d7asp_master_session_create(d7asp_master_session_config_t* d7asp_master_session_config) {
    // TODO for now we assume only one concurrent session, in the future we should dynamically allocate (or return from pool) a session
    assert(current_master_session.state == D7ASP_MASTER_SESSION_IDLE);

    current_master_session = (d7asp_master_session_t){
        .state = D7ASP_MASTER_SESSION_IDLE,
        .token = get_rnd() % 0xFF,
        .progress_bitmap = { 0x00 },
        .success_bitmap = { 0x00 },
        .next_request_id = 0,
        .request_buffer_tail_idx = 0,
        .requests_indices = { 0x00 },
        .requests_lengths = { 0x00 },
        .request_buffer = { 0x00 }
    };

    current_master_session.config.qos = d7asp_master_session_config->qos;
    current_master_session.config.dormant_timeout = d7asp_master_session_config->dormant_timeout;
    current_master_session.config.addressee.ctrl = d7asp_master_session_config->addressee.ctrl;
    memcpy(current_master_session.config.addressee.id, d7asp_master_session_config->addressee.id, sizeof(current_master_session.config.addressee.id));

    return &current_master_session;
}

// TODO we assume a fifo contains only ALP commands, but according to spec this can be any kind of "Request"
// we will see later what this means. For instance how to add a request which starts D7AAdvP etc
d7asp_queue_result_t d7asp_queue_alp_actions(d7asp_master_session_t* session, uint8_t* alp_payload_buffer, uint8_t alp_payload_length)
{
    DPRINT("Queuing ALP actions");
    // TODO can be called in all session states?
    assert(session == &current_master_session); // TODO tmp
    assert(session->request_buffer_tail_idx + alp_payload_length < MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE);
    assert(session->next_request_id < MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT); // TODO do not assert but let upper layer handle this

    single_request_retry_limit = 3; // TODO read from SEL config file

    // add request to buffer
    // TODO request can contain 1 or more ALP commands, find a way to group commands in requests instead of dumping all requests in one buffer
    uint8_t request_id = session->next_request_id;
    session->requests_indices[request_id] = session->request_buffer_tail_idx;
    session->requests_lengths[request_id] = alp_payload_length;
    memcpy(session->request_buffer + session->request_buffer_tail_idx, alp_payload_buffer, alp_payload_length);
    session->request_buffer_tail_idx += alp_payload_length + 1;
    session->next_request_id++;

    // TODO for master only set to pending when asked by upper layer (ie new function call)
    if(d7asp_state == D7ASP_STATE_IDLE)
        switch_state(D7ASP_STATE_MASTER);
    else if(d7asp_state == D7ASP_STATE_SLAVE)
        switch_state(D7ASP_STATE_SLAVE_PENDING_MASTER);

    return (d7asp_queue_result_t){ .fifo_token = session->token, .request_id = request_id };
}

bool d7asp_process_received_packet(packet_t* packet)
{
    hw_watchdog_feed(); // TODO do here?
    d7asp_result_t result = {
        .channel = packet->hw_radio_packet.rx_meta.rx_cfg.channel_id,
        .rx_level =  - packet->hw_radio_packet.rx_meta.rssi,
        .link_budget = (packet->dll_header.control_eirp_index + 32) - packet->hw_radio_packet.rx_meta.rssi,
        .target_rx_level = 80, // TODO not implemented yet, use default for now
        .status = {
            .ucast = 0, // TODO
            .nls = packet->d7anp_ctrl.origin_addressee_ctrl_nls_enabled,
            .retry = false, // TODO
            .missed = false, // TODO
        },
        .response_to = packet->d7anp_timeout,
        .addressee = packet->d7anp_addressee
        // .fifo_token and .seqnr filled below
    };

    if(d7asp_state == D7ASP_STATE_MASTER)
    {
        assert(packet->d7atp_dialog_id == current_master_session.token);
        assert(packet->d7atp_transaction_id == current_request_id);

        // received ack
        DPRINT("Received ACK");
        if(current_master_session.config.qos.qos_resp_mode != SESSION_RESP_MODE_NO
           && current_master_session.config.qos.qos_resp_mode != SESSION_RESP_MODE_NO_RPT)
        {
          // for SESSION_RESP_MODE_NO and SESSION_RESP_MODE_NO_RPT the request was already marked as done
          // upon successfull CSMA insertion. We don't care about response in these cases.

          result.fifo_token = current_master_session.token;
          result.seqnr = current_request_id;
          bitmap_set(current_master_session.success_bitmap, current_request_id);
          mark_current_request_done();
          assert(packet != current_request_packet);
        }

        alp_d7asp_request_completed(result, packet->payload, packet->payload_length);
//          if(d7asp_init_args != NULL && d7asp_init_args->d7asp_fifo_request_completed_cb != NULL)
//              d7asp_init_args->d7asp_fifo_request_completed_cb(result, packet->payload, packet->payload_length); // TODO ALP should notify app if needed, refactor

        packet_queue_free_packet(packet); // ACK can be cleaned
        return true;
    }
    else if(d7asp_state == D7ASP_STATE_IDLE || d7asp_state == D7ASP_STATE_SLAVE)
    {
        // received a request, start slave session, process and respond
        if(d7asp_state == D7ASP_STATE_IDLE)
            switch_state(D7ASP_STATE_SLAVE); // don't switch when already in slave state

        result.fifo_token = packet->d7atp_dialog_id;
        result.seqnr = packet->d7atp_transaction_id;


        // TODO move to ALP
        if(packet->payload_length > 0)
        {
            if(alp_get_operation(packet->payload) == ALP_OP_RETURN_FILE_DATA)
            {
                // received unsollicited data, notify appl
                DPRINT("Received unsollicited data");
                if(d7asp_init_args != NULL && d7asp_init_args->d7asp_received_unsollicited_data_cb != NULL)
                    d7asp_init_args->d7asp_received_unsollicited_data_cb(result, packet->payload, packet->payload_length);

                packet->payload_length = 0; // no response payload
            }
            else
            {
                // build response, we will reuse the same packet for this
                // we will first try to process the command against the local FS
                // if the FS handler cannot process this, and a status response is requested, a status operand will be present in the response payload
                bool handled = alp_process_command(packet->payload, packet->payload_length, packet->payload, &packet->payload_length, ALP_CMD_ORIGIN_D7ASP);

                // ... and if not handled we'll give the application a chance to handle this by returning an ALP response.
                // if the application fails to handle the request as well the ALP status operand supplied by alp_process_command_fs_itf() will be transmitted (if requested)
                if(!handled)
                {
                  DPRINT("ALP command could not be processed by local FS");
                  if(d7asp_init_args != NULL && d7asp_init_args->d7asp_received_unhandled_alp_command_cb != NULL)
                  {
                      DPRINT("ALP command passed to application for processing");
                      d7asp_init_args->d7asp_received_unhandled_alp_command_cb(packet->payload, packet->payload_length, packet->payload, &packet->payload_length);
                  }
                }
            }
        }

        // TODO notify upper layer?

        // execute slave transaction
        if(packet->payload_length == 0 && !packet->d7atp_ctrl.ctrl_is_ack_requested)
            goto discard_request; // no need to respond, clean up

        DPRINT("Sending response");
        d7atp_respond_dialog(packet);
        return true;
    }
    else
        assert(false);

    discard_request:
        packet_queue_free_packet(packet);
        return false;
}


void d7asp_signal_packet_transmitted(packet_t *packet)
{
    DPRINT("Packet transmitted");

    if(d7asp_state == D7ASP_STATE_SLAVE || d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER)
    {
        // when in slave session we can immediately cleanup the transmitted response.
        // requests (in master sessions) will be cleanup upon termination of the dialog.
        packet_queue_free_packet(packet);
    }
}


static void on_request_completed()
{
    assert(d7asp_state == D7ASP_STATE_MASTER);
    if(!bitmap_get(current_master_session.progress_bitmap, current_request_id))
    {
        current_request_retry_count++;
        // the request may be retransmitted, don't free yet (this will be done in flush_fifo() when failed)
    }
    else
    {
        // request completed, no retries needed so we can free the packet
        current_request_id = NO_ACTIVE_REQUEST_ID;
        packet_queue_free_packet(current_request_packet);
    }


    sched_post_task(&flush_fifos); // continue flushing until all request handled ...
}

void d7asp_signal_packet_csma_ca_insertion_completed(bool succeeded)
{
    if(d7asp_state == D7ASP_STATE_MASTER) // TODO only relevant for master sessions?
    {
        if(!succeeded)
        {
            on_request_completed();
            return;
        }

        // for the lowest QoS level the packet is ack-ed when CSMA/CA process succeeded
        if(current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_NO)
        {
            mark_current_request_done();
            bitmap_set(current_master_session.success_bitmap, current_request_id);
        }
    }
}

void d7asp_signal_transaction_response_period_elapsed()
{
    if(d7asp_state == D7ASP_STATE_MASTER)
        on_request_completed();
    else if(d7asp_state == D7ASP_STATE_SLAVE)
        switch_state(D7ASP_STATE_IDLE);
    else if(d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER)
    {
        switch_state(D7ASP_STATE_MASTER);
        sched_post_task(&flush_fifos);
    }
}
