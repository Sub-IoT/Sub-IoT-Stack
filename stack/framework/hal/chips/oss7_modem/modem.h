/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
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

#ifndef __MODEM_H__
#define __MODEM_H__

#include "d7ap.h"
#include "hwuart.h"

// TODO for now we are assuming running on OSS-7, we can refactor later
// so it is more portable

typedef void (*modem_command_completed_callback_t)(bool with_error);
typedef void (*modem_return_file_data_callback_t)(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer);
typedef void (*modem_write_file_data_callback_t)(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer);

typedef struct {
    modem_command_completed_callback_t command_completed_callback;
    modem_return_file_data_callback_t return_file_data_callback;
    modem_write_file_data_callback_t write_file_data_callback;
} modem_callbacks_t;

void modem_init(uart_handle_t* uart_handle, modem_callbacks_t* callbacks);
void modem_reinit();
bool modem_execute_raw_alp(uint8_t* alp, uint8_t len);
bool modem_read_file(uint8_t file_id, uint32_t offset, uint32_t size);
bool modem_write_file(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* data);
bool modem_send_unsolicited_response(uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data, d7ap_session_config_t* d7_interface_config);
bool modem_send_raw_unsolicited_response(uint8_t* alp_command, uint32_t length, d7ap_session_config_t* d7_interface_config);

#endif
