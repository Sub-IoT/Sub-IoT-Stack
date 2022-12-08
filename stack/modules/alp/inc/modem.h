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

#ifndef __MODEM_H__
#define __MODEM_H__
#include "modules_defs.h"
#include "MODULE_ALP_defs.h"
#include "hwuart.h"
#include "lorawan_stack.h"
#include "alp.h"
#include "modem_interface.h"

// TODO for now we are assuming running on OSS-7, we can refactor later
// so it is more portable

#ifdef MODULE_ALP_USE_EXTERNAL_MODEM

typedef void (*modem_command_completed_callback_t)(bool with_error,uint8_t tag_id);
typedef void (*modem_return_file_data_callback_t)(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer);
typedef void (*modem_write_file_data_callback_t)(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer);
typedef void (*modem_interface_status_callback_t)(uint8_t interface_id, uint8_t len, uint8_t* interface_status);

typedef struct {
    modem_command_completed_callback_t command_completed_callback;
    modem_interface_status_callback_t modem_interface_status_callback;
    modem_return_file_data_callback_t return_file_data_callback;
    modem_write_file_data_callback_t write_file_data_callback;
    modem_interface_target_rebooted_callback_t modem_rebooted_callback;
} modem_callbacks_t;

// TODO doc

void modem_init();
void modem_cb_init(modem_callbacks_t* cbs);
void modem_reinit();

int16_t modem_execute_raw_alp(uint8_t* alp, uint8_t len);
alp_command_t* modem_read_file(uint8_t file_id, uint32_t offset, uint32_t size);
int16_t modem_write_file(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* data);
int16_t modem_send_unsolicited_response(uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data, alp_interface_config_t* interface_config);
int16_t modem_send_raw_unsolicited_response(uint8_t* alp_command, uint32_t length, alp_interface_config_t* interface_config);
int16_t modem_send_indirect_unsolicited_response(uint8_t data_file_id, uint32_t offset, uint32_t length, uint8_t* data, uint8_t interface_file_id);
int16_t modem_send_raw_indirect_unsolicited_response(uint8_t* alp_command, uint32_t length, uint8_t interface_file_id);
void modem_send_ping();
alp_command_t* modem_start();
alp_command_t* modem_stop();
alp_command_t* modem_restart();

#endif // MODULE_ALP_USE_EXTERNAL_MODEM
#endif
