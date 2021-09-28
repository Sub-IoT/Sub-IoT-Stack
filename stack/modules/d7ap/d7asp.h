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
 */

/*! \file d7asp.h
 * \addtogroup D7ASP
 * \ingroup D7AP
 * @{
 * \brief Session Layer Protocol APIs
 * \author	glenn.ergeerts@uantwerpen.be
 * \author	philippe.nunes@cortus.com
 */

#ifndef D7ASP_H_
#define D7ASP_H_

#include "stdint.h"
#include "stdbool.h"

#include "MODULE_D7AP_defs.h"
#include "d7ap.h"
#include "packet.h"

#define D7ASP_FIFO_CONFIG_SIZE 16

#define NO_ACTIVE_REQUEST_ID 0xFF

// index [0 ..  7] --> byte 1
// index [8 .. 15] --> byte 2
// index [16.. 23] --> byte 3
// so the byte count can be calculated as the integer quotient of the division + 1 byte
#define REQUESTS_BITMAP_BYTE_COUNT (MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT/8) + 1

/**
 * /brief The state of a session FIFO
 */
typedef struct d7asp_master_session d7asp_master_session_t;

void d7asp_init();
void d7asp_stop();
uint8_t d7asp_master_session_create(d7ap_session_config_t* d7asp_master_session_config);

uint8_t d7asp_queue_request(uint8_t session_token, uint8_t* alp_payload_buffer, uint8_t alp_payload_length, uint8_t expected_alp_response_length);

error_t d7asp_send_response(uint8_t* payload, uint8_t length);

/**
 * @brief Processes a received packet, and switch to slave state in case
 * the flag extension is set and all requests are handled.
 */
void d7asp_process_received_response(packet_t* packet, bool extension);

/**
 * @brief Processes an unsolicited incoming request
 *
 * @returns Whether or not a response is expected. If true the response will be supplied asynchronously by the client.
 */
bool d7asp_process_received_packet(packet_t* packet);

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

/** @}*/
