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

#include "d7anp.h"
#include "d7atp.h"
#include "MODULE_D7AP_defs.h"

#include "session.h"

#define D7ASP_FIFO_CONFIG_SIZE 16

#define NO_ACTIVE_REQUEST_ID 0xFF

typedef struct {
    session_qos_t qos;
    uint8_t dormant_timeout;
    d7anp_addressee_t addressee;
} d7asp_master_session_config_t;

typedef enum {
  D7ASP_MASTER_SESSION_IDLE,
  D7ASP_MASTER_SESSION_DORMANT,
  D7ASP_MASTER_SESSION_PENDING,
  D7ASP_MASTER_SESSION_ACTIVE,
} d7asp_master_session_state_t;

#define REQUESTS_BITMAP_BYTE_COUNT ((MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT + 7) / 8)

/**
 * /brief The state of a session FIFO
 */
typedef struct d7asp_master_session d7asp_master_session_t;

typedef struct {
    union {
        uint8_t raw;
        struct {
            uint8_t _rfu : 4;
            bool ucast : 1;
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
    channel_id_t channel;
    uint8_t rx_level;
    uint8_t link_budget;
    uint8_t target_rx_level;
    d7asp_state_t status;
    uint8_t fifo_token;
    uint8_t seqnr;
    uint8_t response_to;
    d7anp_addressee_t* addressee;
} d7asp_result_t;


void d7asp_init();
d7asp_master_session_t* d7asp_master_session_create(d7asp_master_session_config_t* d7asp_master_session_config);
d7asp_queue_result_t d7asp_queue_alp_actions(d7asp_master_session_t* session, uint8_t* alp_payload_buffer, uint8_t alp_payload_length, uint8_t expected_alp_response_length); // TODO return status

/**
 * @brief Processes a received packet, and prepares the response packet if needed.
 *
 * @returns Wether or not a response is provided. If true the response is contained in the supplied packet.
 * the caller is responsible for sending the response.
 */

bool d7asp_process_received_packet(packet_t* packet, bool extension);

/**
 * @brief Called by DLL to signal the CSMA/CA process completed succesfully and packet can be ack-ed for QoS = None
 */
void d7asp_signal_packet_transmitted(packet_t* packet);

/**
 * @brief Called by DLL to signal the CSMA/CA process failed
 */
void d7asp_signal_transmission_failure();

/**
 * @brief Called by TP to signal the dialog is terminated
 */
void d7asp_signal_dialog_terminated();

/**
 * @brief Called by TP to signal the transaction is terminated
 */
void d7asp_signal_transaction_terminated();

#endif /* D7ASP_H_ */
