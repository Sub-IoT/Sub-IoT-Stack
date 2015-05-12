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
#include "d7asp.h"
#include "alp.h"
#include "fs.h"
#include "scheduler.h"
#include "d7atp.h"
#include "packet_queue.h"
#include "packet.h"

static d7asp_fifo_t NGDEF(_fifo); // TODO we only use 1 fifo for now, should be multiple later (1 per on unique addressee and QoS combination)
#define fifo NG(_fifo)

static void process_fifos()
{
    log_print_stack_string(LOG_STACK_SESSION, "Processing FIFOs");

    uint8_t* data_ptr = fifo.command_buffer;
    alp_control_t alp_control = { .raw = (*data_ptr) };
    data_ptr++;

    packet_t* packet = packet_queue_alloc_packet();
    switch(alp_control.operation)
    {
        case ALP_OP_READ_FILE_DATA: ;
            alp_operand_file_data_request_t operand;
            operand.file_offset.file_id = (*data_ptr); data_ptr++;
            operand.file_offset.offset = (*data_ptr); data_ptr++; // TODO can be 1-4 bytes, assume 1 for now
            operand.requested_data_length = (*data_ptr); data_ptr++;
            fs_read_file(operand.file_offset.file_id, operand.file_offset.offset, packet->payload, operand.requested_data_length);
            packet->payload_length = operand.requested_data_length;
            d7atp_start_dialog(0, 0, packet); // TODO dialog_id and transaction_id
            break;
        default:
            assert(false);
    }

    // TODO free packet after done receiving ack, timeout, .. for now done in dll in tx callback
}

void d7asp_init()
{
    fifo = (d7asp_fifo_t){
        .command_buffer = { 0x00 }
    };

    sched_register_task(&process_fifos);
}

// TODO we assume a fifo contains only ALP commands, but according to spec this can be any kind of "Request"
// we will see later what this means. For instance how to add a request which starts D7AAdvP etc
void d7asp_queue_alp_actions(d7asp_fifo_config_t* d7asp_fifo_config, uint8_t* alp_payload_buffer, uint8_t alp_payload_length)
{
    log_print_stack_string(LOG_STACK_SESSION, "Queuing ALP actions");

    // TODO the actions should be queued in a fifo based on combination of addressee and Qos
    // for now we use only 1 queue and overwrite the config
    fifo.config.fifo_ctrl = d7asp_fifo_config->fifo_ctrl;
    fifo.config.qos = d7asp_fifo_config->qos;
    fifo.config.dormant_timeout = d7asp_fifo_config->dormant_timeout;
    fifo.config.start_id = d7asp_fifo_config->start_id;
    fifo.config.addressee.addressee_ctrl = d7asp_fifo_config->addressee.addressee_ctrl;
    memcpy(fifo.config.addressee.addressee_id, d7asp_fifo_config->addressee.addressee_id, sizeof(fifo.config.addressee.addressee_id));

    // TODO we assume there is only one ALP command in the buffer and this is transmitted before a new command is queued for now
    // we will use a fifo_t instead of a buffer later
    memcpy(fifo.command_buffer, alp_payload_buffer, alp_payload_length);

    sched_post_task(&process_fifos);
}
