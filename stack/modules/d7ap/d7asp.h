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

typedef enum {
    SESSION_STATE_IDLE = 0x00,
    SESSION_STATE_DORMANT = 0x01,
    SESSION_STATE_PENDING = 0x02,
    SESSION_STATE_ACTIVE = 0x03,
    SESSION_STATE_DONE = 0x04,
} session_state_t; // TODO move to session

typedef struct {
    union {
        uint8_t fifo_ctrl;
        struct {
            bool fifo_ctrl_nls : 1;
            uint8_t _rfu : 1;
            bool fifo_ctrl_stop_on_error : 1;
            bool fifo_ctrl_preferred : 1;
            uint8_t fifo_ctrl_state : 2; // TODO using session_state_t results in "'state' is narrower than value of its type" warning
        };
    };
    uint32_t qos; // TODO define struct
    uint8_t dormant_timeout;
    uint8_t start_id;
    d7atp_addressee_t addressee;
} d7asp_fifo_config_t;

typedef struct {
    d7asp_fifo_config_t config;
    // TODO uint8_t dorm_timer;
    // TODO uint8_t token;
    // TODO retry_single_cnt
    // TODO retry_total_cnt
    // TODO progress_bitmap
    // TODO success_bitmap
    // TODO next_id_cnt
    uint8_t command_buffer[MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE];
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
    d7asp_state_t status;
    uint8_t fifo_token;
    uint8_t request_id;
    uint8_t response_to;
    d7atp_addressee_t addressee;
} d7asp_result_t;

void d7asp_init();
void d7asp_queue_alp_actions(d7asp_fifo_config_t* d7asp_fifo_config, uint8_t* alp_payload_buffer, uint8_t alp_payload_length); // TODO return status
void d7asp_process_received_packet(packet_t* packet);
#endif /* D7ASP_H_ */
