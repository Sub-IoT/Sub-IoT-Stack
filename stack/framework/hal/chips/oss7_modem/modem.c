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

#include "modem.h"
#include "alp.h"
#include "alp_layer.h"
#include "d7ap.h"
#include "debug.h"
#include "errors.h"
#include "fifo.h"
#include "log.h"
#include "modem_interface.h"
#include "platform.h"
#include "scheduler.h"
#include "string.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_MODEM_LOG_ENABLED)
  #define DPRINT(...) log_print_string(__VA_ARGS__)
  #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
    #define DPRINT(...)
    #define DPRINT_DATA(...)
#endif


static modem_callbacks_t* callbacks;
static alp_init_args_t alp_init_args;

static alp_interface_config_t serial_itf_config = (alp_interface_config_t) {
    .itf_id = ALP_ITF_ID_SERIAL,
};

static void on_alp_command_completed_cb(uint8_t tag_id, bool success)
{
    if (success)
        log_print_string("Command %i completed successfully\n", tag_id);
    else
        log_print_string("Command %i failed, no ack received\n", tag_id);

    DPRINT("command with tag %i completed @ %i", tag_id, timer_get_counter_value());
    if (callbacks->command_completed_callback)
        callbacks->command_completed_callback(!success, tag_id);
}

static void on_alp_command_result_cb(alp_interface_status_t* status, uint8_t* payload, uint8_t payload_length)
{
    alp_action_t action;
    uint8_t parse_index = 0;
    while (parse_index < payload_length) {
        alp_parse_action(payload, &parse_index, &action);
        switch (action.ctrl.operation) {
        case ALP_OP_STATUS:
            if (action.interface_status.itf_id == ALP_ITF_ID_SERIAL) {
                break; // skip the serial interface status
            }
            if (action.interface_status.itf_id == ALP_ITF_ID_D7ASP) {
                if (action.interface_status.len > 0) {
                    d7ap_session_result_t* d7_result = ((d7ap_session_result_t*)status->itf_status);
                    log_print_string("recv response @ %i dB link budget from:", d7_result->rx_level);
                    log_print_data(d7_result->addressee.id, d7ap_addressee_id_length(d7_result->addressee.ctrl.id_type));
                }
            } else {
                log_print_string("itf status for itf id %i len %i", action.interface_status.itf_id, action.interface_status.len);
            }

            if (callbacks->modem_interface_status_callback) {
                callbacks->modem_interface_status_callback(action.interface_status.itf_id, action.interface_status.len, action.interface_status.itf_status);
            }
            break;
        case ALP_OP_RESPONSE_TAG:
            // skip, ALP layer will call on_alp_command_completed_cb()
            break;
        case ALP_OP_WRITE_FILE_DATA:
            if (callbacks->write_file_data_callback)
                callbacks->write_file_data_callback(action.file_data_operand.file_offset.file_id,
                    action.file_data_operand.file_offset.offset,
                    action.file_data_operand.provided_data_length,
                    action.file_data_operand.data);
            break;
        case ALP_OP_RETURN_FILE_DATA:
            if (callbacks->return_file_data_callback)
                callbacks->return_file_data_callback(action.file_data_operand.file_offset.file_id,
                    action.file_data_operand.file_offset.offset,
                    action.file_data_operand.provided_data_length,
                    action.file_data_operand.data);
            break;

        default:
            log_print_string("ALP op %i not handled", action.ctrl.operation);
            break;
        }
    }
}

void modem_cb_init(modem_callbacks_t* cbs)
{
    callbacks = cbs;
    if(cbs->modem_rebooted_callback)
      modem_interface_set_target_rebooted_callback(cbs->modem_rebooted_callback);
}

void modem_init()
{
    alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
    alp_init_args.alp_command_result_cb = &on_alp_command_result_cb;
    //alp_init_args.alp_received_unsolicited_data_cb = &on_alp_received_unsolicited_data_cb;

    alp_layer_init(&alp_init_args, true);
}


//void modem_send_ping() {
//  uint8_t ping_request[1]={0x01};
//  modem_interface_transfer_bytes((uint8_t*) &ping_request, 1, SERIAL_MESSAGE_TYPE_PING_REQUEST);
//}

//bool modem_execute_raw_alp(uint8_t* alp, uint8_t len) {
//  modem_interface_transfer_bytes(alp, len, SERIAL_MESSAGE_TYPE_ALP_DATA);
//}

//bool modem_create_and_write_file(uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data, fs_storage_class_t storage_class) {
//  if(!alloc_command())
//    return false;

//  alp_append_create_new_file_data_action(&command.fifo, file_id, length, storage_class, true, false);

//  alp_append_write_file_data_action(&command.fifo, file_id, offset, length, data, true, false);

//  modem_interface_transfer_bytes(command.buffer, fifo_get_size(&command.fifo), SERIAL_MESSAGE_TYPE_ALP_DATA);

//  return true;
//}

//bool modem_create_file(uint8_t file_id, uint32_t length, fs_storage_class_t storage_class) {
//  if(!alloc_command())
//    return false;
  
//  alp_append_create_new_file_data_action(&command.fifo, file_id, length, storage_class, true, false);

//  modem_interface_transfer_bytes(command.buffer, fifo_get_size(&command.fifo), SERIAL_MESSAGE_TYPE_ALP_DATA);

//  return true;
//}

bool modem_read_file(uint8_t file_id, uint32_t offset, uint32_t size)
{
    alp_command_t* command = alp_layer_command_alloc(true, false);
    if (command == NULL)
        return false;

    alp_append_forward_action(&command->alp_command_fifo, &serial_itf_config, 0);
    alp_append_tag_request_action(&command->alp_command_fifo, command->tag_id, true);
    alp_append_read_file_data_action(&command->alp_command_fifo, file_id, offset, size, true, false);

    alp_layer_process(command);

    return true;
}

bool modem_write_file(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* data) {
    alp_command_t* command = alp_layer_command_alloc(true, false);
    if (command == NULL)
        return false;

    alp_append_forward_action(&command->alp_command_fifo, &serial_itf_config, 0);
    alp_append_tag_request_action(&command->alp_command_fifo, command->tag_id, true);
    alp_append_write_file_data_action(&command->alp_command_fifo, file_id, offset, size, data, true, false);

    alp_layer_process(command);

    return true;
}

bool modem_send_unsolicited_response(uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data,
                                     alp_interface_config_t* interface_config) {
    alp_command_t* command = alp_layer_command_alloc(true, false);
    if (command == NULL)
        return false;

    alp_append_forward_action(&command->alp_command_fifo, &serial_itf_config, 0);
    alp_append_tag_request_action(&command->alp_command_fifo, command->tag_id, true);
    alp_append_forward_action(&command->alp_command_fifo, interface_config, 0);
    alp_append_return_file_data_action(&command->alp_command_fifo, file_id, offset, length, data);

    alp_layer_process(command);
    return true;
}

bool modem_send_raw_unsolicited_response(uint8_t* alp_command, uint32_t length,
                                         alp_interface_config_t* interface_config) {
    alp_command_t* command = alp_layer_command_alloc(true, false);
    if (command == NULL)
        return false;

    alp_append_forward_action(&command->alp_command_fifo, &serial_itf_config, 0);
    alp_append_tag_request_action(&command->alp_command_fifo, command->tag_id, true);
    alp_append_forward_action(&command->alp_command_fifo, interface_config, 0);
    fifo_put(&command->alp_command_fifo, alp_command, length);

    alp_layer_process(command);

    return true;
}

bool modem_send_indirect_unsolicited_response(uint8_t data_file_id, uint32_t offset, uint32_t length, uint8_t* data,
    uint8_t interface_file_id, bool overload, d7ap_addressee_t* d7_addressee)
{
    alp_command_t* command = alp_layer_command_alloc(true, false);
    if (command == NULL)
        return false;

    alp_append_forward_action(&command->alp_command_fifo, &serial_itf_config, 0);
    alp_append_tag_request_action(&command->alp_command_fifo, command->tag_id, true);
    alp_append_indirect_forward_action(&command->alp_command_fifo, interface_file_id, overload, (uint8_t*)&d7_addressee, d7ap_addressee_id_length(d7_addressee->ctrl.id_type));

    alp_append_return_file_data_action(&command->alp_command_fifo, data_file_id, offset, length, data);

    alp_layer_process(command);
    return true;
}

int8_t modem_send_raw_indirect_unsolicited_response(uint8_t* alp_command, uint32_t length,
    uint8_t interface_file_id, bool overload, d7ap_addressee_t* d7_addressee)
{
    alp_command_t* command = alp_layer_command_alloc(true, false);
    if (command == NULL)
        return -1;

    alp_append_forward_action(&command->alp_command_fifo, &serial_itf_config, 0);
    alp_append_tag_request_action(&command->alp_command_fifo, command->tag_id, true);
    alp_append_indirect_forward_action(&command->alp_command_fifo, interface_file_id, overload, (uint8_t *) &d7_addressee, sizeof(d7_addressee));
    fifo_put(&command->alp_command_fifo, alp_command, length);

    alp_layer_process(command);
    return command->tag_id;
}
