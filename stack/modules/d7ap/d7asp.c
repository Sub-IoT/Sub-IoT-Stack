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
#include "assert.h"
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

static d7asp_fifo_t NGDEF(_fifo); // TODO we only use 1 fifo for now, should be multiple later (1 per on unique addressee and QoS combination)
#define fifo NG(_fifo)

static uint8_t NGDEF(_active_request_id); // TODO move ?
#define active_request_id NG(_active_request_id)

static void init_fifo()
{
    fifo = (d7asp_fifo_t){
        .progress_bitmap = { 0x00 },
        .next_request_id = 0,
        .request_buffer_tail_idx = 0,
        .requests_indices = { 0x00 },
        .request_buffer = { 0x00 }
    };
}

static void flush_fifos()
{
    log_print_stack_string(LOG_STACK_SESSION, "Flushing FIFOs");

    // find first request which is not acked or dropped
    uint8_t found_next_req_index = bitmap_search(fifo.progress_bitmap, false, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
    if(found_next_req_index == -1 || found_next_req_index == fifo.next_request_id)
    {
        // we handled all requests ...
        log_print_stack_string(LOG_STACK_SESSION, "FIFO flush completed");
        // TODO notify upper layer
        init_fifo();
        return;
    }

    active_request_id = found_next_req_index;

    packet_t* packet = packet_queue_alloc_packet();
    packet->d7atp_addressee = &(fifo.config.addressee);

    alp_process_command(fifo.request_buffer + fifo.requests_indices[active_request_id], packet);

    d7atp_start_dialog(0, 0, packet); // TODO dialog_id and transaction_id
}

void d7asp_init()
{
    init_fifo();

    sched_register_task(&flush_fifos);
}

// TODO we assume a fifo contains only ALP commands, but according to spec this can be any kind of "Request"
// we will see later what this means. For instance how to add a request which starts D7AAdvP etc
void d7asp_queue_alp_actions(d7asp_fifo_config_t* d7asp_fifo_config, uint8_t* alp_payload_buffer, uint8_t alp_payload_length)
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

    // add request to buffer
    fifo.requests_indices[fifo.next_request_id] = fifo.request_buffer_tail_idx;
    memcpy(fifo.request_buffer + fifo.request_buffer_tail_idx, alp_payload_buffer, alp_payload_length);
    fifo.request_buffer_tail_idx += alp_payload_length + 1;
    fifo.next_request_id++;

    sched_post_task(&flush_fifos);
}

void d7asp_process_received_packet(packet_t* packet)
{
    d7asp_result_t result = {
        .status = {
            .session_state = SESSION_STATE_DONE, // TODO slave session state can be active as well, assuming done now
            .nls = packet->d7anp_ctrl.nls_enabled,
            .retry = false, // TODO
            .missed = false, // TODO
        },
        .fifo_token = packet->d7atp_dialog_id,
        .request_id = packet->d7atp_transaction_id,
        .response_to = 0, // TODO
        .addressee = {
            .addressee_ctrl_has_id = packet->d7anp_ctrl.origin_access_id_present? true : false,
            .addressee_ctrl_virtual_id = packet->dll_header.control_vid_used,
            .addressee_ctrl_access_class = packet->d7anp_ctrl.origin_access_class,
        },
    };

    memcpy(result.addressee.addressee_id, packet->origin_access_id, 8);

    alp_process_received_command_d7asp(result, packet->payload, packet->payload_length);
}

void d7asp_ack_current_request()
{
    bitmap_set(fifo.progress_bitmap, active_request_id); // TODO only when acked or dropped
    sched_post_task(&flush_fifos); // keep flushing until all request handled ...
}
