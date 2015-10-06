/*! \file d7asp.h
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
 * \author glenn.ergeerts@uantwerpen.be
 * \author maarten.weyn@uantwerpen.be
 *
 */

#ifndef D7ASP_H_
#define D7ASP_H_

#include "stdint.h"
#include "stdbool.h"

#include "d7atp.h"
#include "MODULE_D7AP_defs.h"

#include "session.h"

#define D7ASP_FIFO_CONFIG_SIZE 16

typedef struct {
    union {
        uint8_t fifo_ctrl;
        struct {
            uint8_t fifo_ctrl_state : 3; // TODO using session_state_t results in "'state' is narrower than value of its type" warning
            uint8_t _rfu : 1;
            bool fifo_ctrl_preferred : 1;
            bool fifo_ctrl_stop_on_error : 1;
            uint8_t _rfu2 : 1;
            bool fifo_ctrl_nls : 1;
        };
    };
    session_qos_t qos;
    uint8_t dormant_timeout;
    uint8_t start_id;
    d7atp_addressee_t addressee;
} d7asp_fifo_config_t;

#define REQUESTS_BITMAP_BYTE_COUNT ((MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT + 7) / 8)

/**
 * /brief The state of a session FIFO
 */
typedef struct {
    d7asp_fifo_config_t config;
    // TODO uint8_t dorm_timer;
    uint8_t token;
    // TODO retry_single_cnt
    // TODO retry_total_cnt
    uint8_t progress_bitmap[REQUESTS_BITMAP_BYTE_COUNT];
    uint8_t success_bitmap[REQUESTS_BITMAP_BYTE_COUNT];
    uint8_t next_request_id;
    uint8_t request_buffer_tail_idx;
    uint8_t requests_indices[MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT]; /**< Contains for every request ID the index in command_buffer where the request begins */
    uint8_t request_buffer[MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE];
} d7asp_fifo_t;

typedef struct {
    union {
        uint8_t raw;
        struct {
            session_state_t session_state : 3;
            uint8_t _rfu : 2;
            bool retry : 1;
            bool missed : 1;
            bool nls : 1;
        };
    };
} d7asp_state_t;

typedef struct {
    uint8_t fifo_token;
    uint8_t request_id;
} d7asp_queue_result_t;

typedef struct {
    d7asp_state_t status;
    uint8_t fifo_token;
    uint8_t request_id;
    uint8_t response_to;
    d7atp_addressee_t addressee;
} d7asp_result_t;

typedef void (*d7asp_fifo_flush_completed_callback)(uint8_t fifo_token, uint8_t* progress_bitmap, uint8_t* success_bitmap, uint8_t bitmap_byte_count);

typedef struct {
    d7asp_fifo_flush_completed_callback d7asp_fifo_flush_completed_cb;
} d7asp_init_args_t; // TODO workaround: NG does not support function pointer so store in struct (for now)

void d7asp_init(d7asp_init_args_t* init_arfs);
d7asp_queue_result_t d7asp_queue_alp_actions(d7asp_fifo_config_t* d7asp_fifo_config, uint8_t* alp_payload_buffer, uint8_t alp_payload_length); // TODO return status
void d7asp_process_received_packet(packet_t* packet);

/**
 * @brief Called by DLL to signal the packet has been transmitted
 */
void d7asp_signal_packet_transmitted(packet_t* packet);


/**
 * @brief Called by DLL to signal the CSMA/CA process completed succesfully and packet can be ack-ed for QoS = None
 */
void d7asp_signal_packet_csma_ca_insertion_completed(bool succeeded);


/**
 * @brief Called by TP to signal the transaction request period has elapsed
 */
void d7asp_signal_transaction_response_period_elapsed();
#endif /* D7ASP_H_ */
