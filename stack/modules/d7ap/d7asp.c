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

static d7asp_fifo_t NGDEF(_fifo); // TODO we only use 1 fifo for now, should be multiple later (1 per on unique addressee and QoS combination)
#define fifo NG(_fifo)

static uint8_t NGDEF(_current_request_id); // TODO move ?
#define current_request_id NG(_current_request_id)

#define NO_ACTIVE_REQUEST_ID 0xFF

static uint8_t NGDEF(_current_request_retry_count);
#define current_request_retry_count NG(_current_request_retry_count)

static packet_t* NGDEF(_current_request_packet);
#define current_request_packet NG(_current_request_packet)

static uint8_t NGDEF(_single_request_retry_limit);
#define single_request_retry_limit NG(_single_request_retry_limit)

static dae_access_profile_t NGDEF(_current_access_profile);
#define current_access_profile NG(_current_access_profile)

static uint8_t NGDEF(_current_access_class);
#define current_access_class NG(_current_access_class)

#define ACCESS_CLASS_NOT_SET 0xFF

static d7asp_init_args_t* NGDEF(_d7asp_init_args);
#define d7asp_init_args NG(_d7asp_init_args)

typedef enum {
    D7ASP_STATE_IDLE,
    D7ASP_STATE_SLAVE,
    D7ASP_STATE_MASTER,
    D7ASP_STATE_SLAVE_PENDING_MASTER
} state_t;

static state_t NGDEF(_state);
#define state NG(_state)

static void switch_state(state_t new_state);

static void init_fifo()
{
    fifo = (d7asp_fifo_t){
        .token = get_rnd() % 0xFF,
        .progress_bitmap = { 0x00 },
        .success_bitmap = { 0x00 },
        .next_request_id = 0,
        .request_buffer_tail_idx = 0,
        .requests_indices = { 0x00 },
        .requests_lengths = { 0x00 },
        .request_buffer = { 0x00 }
    };
}

static void mark_current_request_done()
{
    bitmap_set(fifo.progress_bitmap, current_request_id);
    // current_request_packet will be free-ed in the packet_queue when the transaction is completed
}

static void flush_fifos()
{
    assert(state == D7ASP_STATE_MASTER);
    log_print_stack_string(LOG_STACK_SESSION, "Flushing FIFOs");

    if(current_request_id == NO_ACTIVE_REQUEST_ID)
    {
        // find first request which is not acked or dropped
        int8_t found_next_req_index = bitmap_search(fifo.progress_bitmap, false, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
        if(found_next_req_index == -1 || found_next_req_index == fifo.next_request_id)
        {
            // we handled all requests ...
            log_print_stack_string(LOG_STACK_SESSION, "FIFO flush completed");
            if(d7asp_init_args != NULL && d7asp_init_args->d7asp_fifo_flush_completed_cb != NULL)
                d7asp_init_args->d7asp_fifo_flush_completed_cb(fifo.token, fifo.progress_bitmap, fifo.success_bitmap, REQUESTS_BITMAP_BYTE_COUNT);

            init_fifo();
            switch_state(D7ASP_STATE_IDLE);
            return;
        }

        current_request_id = found_next_req_index;
        current_request_retry_count = 0;

        current_request_packet = packet_queue_alloc_packet();
        packet_queue_mark_processing(current_request_packet);
        current_request_packet->d7atp_addressee = &(fifo.config.addressee);

        memcpy(current_request_packet->payload, fifo.request_buffer + fifo.requests_indices[current_request_id], fifo.requests_lengths[current_request_id]);
        current_request_packet->payload_length = fifo.requests_lengths[current_request_id];

        uint8_t access_class = fifo.config.addressee.addressee_ctrl_access_class;
        if(access_class != current_access_class)
            fs_read_access_class(access_class, &current_access_profile);
    }
    else
    {
        // retrying request ...
        log_print_stack_string(LOG_STACK_SESSION, "Current request retry count: %i", current_request_retry_count);
        if(current_request_retry_count == single_request_retry_limit)
        {
            // mark request as failed and pop
            mark_current_request_done();
            log_print_stack_string(LOG_STACK_SESSION, "Request reached single request retry limit (%i), skipping request", single_request_retry_limit);
            packet_queue_free_packet(current_request_packet);
            current_request_id = NO_ACTIVE_REQUEST_ID;
            sched_post_task(&flush_fifos); // continue flushing until all request handled ...
            return;
        }

        // TODO retry total cnt
        // TODO stop on error
    }

    d7atp_start_dialog(fifo.token, current_request_id, true, current_request_packet, &fifo.config.qos, &current_access_profile);
}



// TODO document state diagram
static void switch_state(state_t new_state)
{
    switch(new_state)
    {
        case D7ASP_STATE_MASTER:
            switch(state)
            {
                case D7ASP_STATE_IDLE:
                    state = new_state;
                    sched_post_task(&flush_fifos);
                    log_print_stack_string(LOG_STACK_SESSION, "Switching to state D7ASP_STATE_MASTER");
                    break;
                case D7ASP_STATE_SLAVE_PENDING_MASTER:
                    state = new_state;
                    sched_post_task(&flush_fifos);
                    log_print_stack_string(LOG_STACK_SESSION, "Switching to state D7ASP_STATE_MASTER");
                    break;
                case D7ASP_STATE_SLAVE:
                    state = D7ASP_STATE_SLAVE_PENDING_MASTER;
                    log_print_stack_string(LOG_STACK_SESSION, "Switching to state D7ASP_STATE_SLAVE_PENDING_MASTER");
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
            switch(state)
            {
                case D7ASP_STATE_IDLE:
                    state = new_state;
                    current_request_id = NO_ACTIVE_REQUEST_ID;
                    log_print_stack_string(LOG_STACK_SESSION, "Switching to state D7ASP_STATE_SLAVE");
                    break;
                default:
                    assert(false);
            }
            break;
        case D7ASP_STATE_SLAVE_PENDING_MASTER:
            assert(state == D7ASP_STATE_SLAVE);
            state = D7ASP_STATE_SLAVE_PENDING_MASTER;
            log_print_stack_string(LOG_STACK_SESSION, "Switching to state D7ASP_STATE_SLAVE_PENDING_MASTER");
            break;
        case D7ASP_STATE_IDLE:
            state = new_state;
            log_print_stack_string(LOG_STACK_SESSION, "Switching to state D7ASP_STATE_IDLE");
            break;
        default:
            assert(false);
    }
}

void d7asp_init(d7asp_init_args_t* init_args)
{
    state = D7ASP_STATE_IDLE;
    current_access_class = ACCESS_CLASS_NOT_SET;
    d7asp_init_args = init_args;
    current_request_id = NO_ACTIVE_REQUEST_ID;

    init_fifo();

    sched_register_task(&flush_fifos);
}

// TODO we assume a fifo contains only ALP commands, but according to spec this can be any kind of "Request"
// we will see later what this means. For instance how to add a request which starts D7AAdvP etc
d7asp_queue_result_t d7asp_queue_alp_actions(d7asp_fifo_config_t* d7asp_fifo_config, uint8_t* alp_payload_buffer, uint8_t alp_payload_length)
{
    log_print_stack_string(LOG_STACK_SESSION, "Queuing ALP actions");

    assert(fifo.request_buffer_tail_idx + alp_payload_length < MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE);
    assert(fifo.next_request_id < MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT); // TODO do not assert but let upper layer handle this

    // TODO the actions should be queued in a fifo based on combination of addressee and Qos
    // for now we use only 1 queue and overwrite the config
    fifo.config.fifo_ctrl = d7asp_fifo_config->fifo_ctrl;
    fifo.config.qos = d7asp_fifo_config->qos;
    fifo.config.dormant_timeout = d7asp_fifo_config->dormant_timeout;
    fifo.config.start_id = d7asp_fifo_config->start_id;
    fifo.config.addressee.addressee_ctrl = d7asp_fifo_config->addressee.addressee_ctrl;
    memcpy(fifo.config.addressee.addressee_id, d7asp_fifo_config->addressee.addressee_id, sizeof(fifo.config.addressee.addressee_id));
    single_request_retry_limit = fifo.config.qos.qos_retry_single;

    // add request to buffer
    // TODO request can contain 1 or more ALP commands, find a way to group commands in requests instead of dumping all requests in one buffer
    uint8_t request_id = fifo.next_request_id;
    fifo.requests_indices[request_id] = fifo.request_buffer_tail_idx;
    fifo.requests_lengths[request_id] = alp_payload_length;
    memcpy(fifo.request_buffer + fifo.request_buffer_tail_idx, alp_payload_buffer, alp_payload_length);
    fifo.request_buffer_tail_idx += alp_payload_length + 1;
    fifo.next_request_id++;

    if(state == D7ASP_STATE_IDLE)
        switch_state(D7ASP_STATE_MASTER);
    else if(state == D7ASP_STATE_SLAVE)
        switch_state(D7ASP_STATE_SLAVE_PENDING_MASTER);

    return (d7asp_queue_result_t){ .fifo_token = fifo.token, .request_id = request_id };
}

void d7asp_process_received_packet(packet_t* packet)
{
    d7asp_result_t result = {
        .status = {
            .session_state = SESSION_STATE_DONE, // TODO
            .nls = packet->d7anp_ctrl.nls_enabled,
            .retry = false, // TODO
            .missed = false, // TODO
        },
        .response_to = 0, // TODO
        .addressee = packet->d7atp_addressee
        // .fifo_token and .request_id filled below
    };

    if(state == D7ASP_STATE_MASTER)
    {
        assert(fifo.config.qos.qos_ctrl_resp_mode > SESSION_RESP_MODE_NONE);
        assert(packet->d7atp_dialog_id == fifo.token);
        assert(packet->d7atp_transaction_id == current_request_id);

        // received ack
        result.fifo_token = fifo.token;
        result.request_id = current_request_id;
        log_print_stack_string(LOG_STACK_SESSION, "Received ACK");
        bitmap_set(fifo.success_bitmap, current_request_id);
        mark_current_request_done();
        assert(packet != current_request_packet);

        if(d7asp_init_args->d7asp_fifo_request_completed_cb)
            d7asp_init_args->d7asp_fifo_request_completed_cb(result, packet->payload, packet->payload_length);

        packet_queue_free_packet(packet); // ACK can be cleaned
    }
    else if(state == D7ASP_STATE_IDLE || state == D7ASP_STATE_SLAVE)
    {
        // received a request, start slave session, process and respond
        if(state == D7ASP_STATE_IDLE)
            switch_state(D7ASP_STATE_SLAVE); // don't switch when already in slave state

        result.fifo_token = packet->d7atp_dialog_id;
        result.request_id = packet->d7atp_transaction_id;

        // TODO notify upper layer?

        // build response, we will reuse the same packet for this
        alp_process_command(packet->payload, packet->payload_length, packet->payload, &packet->payload_length);

        // execute slave transaction
        if(packet->payload_length == 0 && !packet->d7atp_ctrl.ctrl_is_ack_requested)
        {
            // no need to respond, clean up
            packet_queue_free_packet(packet);
            switch_state(D7ASP_STATE_IDLE);
            return;
        }

        log_print_stack_string(LOG_STACK_SESSION, "Sending response");
        d7atp_respond_dialog(packet);
    }
    else
        assert(false);
}


void d7asp_signal_packet_transmitted(packet_t *packet)
{
    log_print_stack_string(LOG_STACK_SESSION, "Packet transmitted");

    if(state == D7ASP_STATE_SLAVE || state == D7ASP_STATE_SLAVE_PENDING_MASTER)
    {
        // when in slave session we can immediately cleanup the transmitted response.
        // requests (in master sessions) will be cleanup upon termination of the dialog.
        packet_queue_free_packet(packet);
    }
}


static void on_request_completed()
{
    assert(state == D7ASP_STATE_MASTER);
    if(!bitmap_get(fifo.progress_bitmap, current_request_id))
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
    if(state == D7ASP_STATE_MASTER) // TODO only relevant for master sessions?
    {
        if(!succeeded)
        {
            on_request_completed();
            return;
        }

        // for the lowest QoS level the packet is ack-ed when CSMA/CA process succeeded
        if(fifo.config.qos.qos_ctrl_resp_mode == SESSION_RESP_MODE_NONE)
        {
            mark_current_request_done();
            bitmap_set(fifo.success_bitmap, current_request_id);
        }
    }
}

void d7asp_signal_transaction_response_period_elapsed()
{
    if(state == D7ASP_STATE_MASTER)
        on_request_completed();
    else if(state == D7ASP_STATE_SLAVE)
        switch_state(D7ASP_STATE_IDLE);
    else if(state == D7ASP_STATE_SLAVE_PENDING_MASTER)
    {
        switch_state(D7ASP_STATE_MASTER);
        sched_post_task(&flush_fifos);
    }
}
