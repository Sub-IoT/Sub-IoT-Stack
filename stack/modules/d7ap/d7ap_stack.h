/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 * Copyright 2018 Cortus SA
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file d7ap_stack.h
 * \addtogroup D7AP_STACK
 * \ingroup D7AP
 * @{
 * \brief D7AP stack APIs
 * \author   glenn.ergeerts@uantwerpen.be
 * \author   philippe.nunes@cortus.com
 */

#ifndef OSS_7_D7AP_STACK_H
#define OSS_7_D7AP_STACK_H

#include "d7ap.h"

/**
 * @brief Initializes the D7AP stack, should be called during the platform initialization
 */
void d7ap_stack_init(void);

/**
 * @brief Stops the D7AP stack tasks and free the hardware resources
 */
void d7ap_stack_stop();

error_t d7ap_stack_send(uint8_t client_id, d7ap_session_config_t* config, uint8_t* payload,
                        uint8_t len, uint8_t expected_response_length, uint16_t *trans_id);

bool d7ap_stack_process_unsolicited_request(uint8_t* payload, uint8_t length, d7ap_session_result_t result);

void d7ap_stack_process_received_response(uint8_t* payload, uint8_t length, d7ap_session_result_t result);

void d7ap_stack_session_completed(uint8_t session_token, uint8_t* progress_bitmap, uint8_t* success_bitmap, uint8_t bitmap_byte_count);

bool d7ap_stack_is_client_session_active(uint8_t client_id);

void d7ap_stack_signal_active_master_session(uint8_t session_token);

void d7ap_stack_signal_slave_session_terminated(void);

#endif //OSS_7_D7AP_STACK_H

/** @}*/
