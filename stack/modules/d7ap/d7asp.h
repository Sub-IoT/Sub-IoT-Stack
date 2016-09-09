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

typedef void (*d7asp_fifo_flush_completed_callback)(uint8_t fifo_token, uint8_t* progress_bitmap, uint8_t* success_bitmap, uint8_t bitmap_byte_count);
typedef void (*d7asp_fifo_request_completed_callback)(d7asp_result_t result, uint8_t* payload, uint8_t payload_length);
typedef void (*d7asp_received_unsollicited_data_callback)(d7asp_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size);
typedef void (*d7asp_received_unhandled_alp_command_callback)(uint8_t* alp_command, uint8_t alp_command_length, uint8_t* alp_response, uint8_t* alp_response_length);

typedef struct {
    d7asp_fifo_flush_completed_callback d7asp_fifo_flush_completed_cb;
    d7asp_fifo_request_completed_callback d7asp_fifo_request_completed_cb;
    d7asp_received_unsollicited_data_callback d7asp_received_unsollicited_data_cb;
    /**
     * @brief d7asp_received_unhandled_alp_command_cb Called when the stack received an ALP command which cannot be processed against the local filesystem.
     * The application is given the chance to provide a response (by filling the alp_response and alp_response_length parameters). If the application is not
     * able to process the command as well it should just return without altering these parameters.
     * It is important to know this callback is called while a D7AP transaction is in process thus be sure to return within transaction timeout limits!
     */
    d7asp_received_unhandled_alp_command_callback d7asp_received_unhandled_alp_command_cb;
} d7asp_init_args_t;

void d7asp_init(d7asp_init_args_t* init_args);
d7asp_master_session_t* d7asp_master_session_create(d7asp_master_session_config_t* d7asp_master_session_config);
d7asp_queue_result_t d7asp_queue_alp_actions(d7asp_master_session_t* session, uint8_t* alp_payload_buffer, uint8_t alp_payload_length); // TODO return status
bool d7asp_process_received_packet(packet_t* packet);

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
