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

/*
 * \author	glenn.ergeerts@uantwerpen.be
 */

#include "alp_cmd_handler.h"

#define ALP_CMD_HANDLER_HEADER_SIZE 2 // <ALP interface ID> <Length byte>


#include "types.h"
#include "alp.h"
#include "shell.h"
#include "console.h"
#include "debug.h"
#include "MODULE_D7AP_defs.h"
#include "ng.h"

static alp_cmd_handler_appl_itf_callback NGDEF(_alp_cmd_handler_appl_itf_cb);
#define alp_cmd_handler_appl_itf_cb NG(_alp_cmd_handler_appl_itf_cb)

void alp_cmd_handler(fifo_t* cmd_fifo)
{
    // AT$D<ALP interface ID><Length byte><ALP interface config><ALP command>
    // interface: 0xD7 = D7ASP, 0x00 = own filesystem, 0x01 = application
    // interface config: D7ASP fifo config in case of interface 0xD7, void for interface 0x00 and 0x01
    // where length is the length of interface config and ALP command
    if(fifo_get_size(cmd_fifo) > SHELL_CMD_HEADER_SIZE + 2)
    {
        uint8_t alp_command[ALP_CMD_HANDLER_HEADER_SIZE + MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE] = { 0x00 };
        fifo_peek(cmd_fifo, alp_command, SHELL_CMD_HEADER_SIZE, ALP_CMD_HANDLER_HEADER_SIZE);
        uint8_t alp_interface_id = alp_command[0];
        assert(alp_interface_id == ALP_ITF_ID_FS
               || alp_interface_id == ALP_ITF_ID_D7ASP
               || alp_interface_id == ALP_ITF_ID_APP);

        uint8_t length = alp_command[1];
        if(fifo_get_size(cmd_fifo) < SHELL_CMD_HEADER_SIZE + ALP_CMD_HANDLER_HEADER_SIZE + length)
            return; // ALP command not complete yet, don't pop

        fifo_pop(cmd_fifo, alp_command, SHELL_CMD_HEADER_SIZE + ALP_CMD_HANDLER_HEADER_SIZE + length);
        uint8_t* payload = alp_command + SHELL_CMD_HEADER_SIZE + ALP_CMD_HANDLER_HEADER_SIZE;
        if(alp_interface_id == ALP_ITF_ID_D7ASP)
        {
            // parse D7ASP fifo config
            uint8_t* ptr = payload;
            d7asp_fifo_config_t fifo_config;
            fifo_config.fifo_ctrl = (*ptr); ptr++;
            memcpy(&(fifo_config.qos), ptr, 4); ptr += 4;
            fifo_config.dormant_timeout = (*ptr); ptr++;
            fifo_config.start_id = (*ptr); ptr++;
            memcpy(&(fifo_config.addressee), ptr, 9); ptr += 9;
            uint8_t alp_command_length = length - D7ASP_FIFO_CONFIG_SIZE;

            // queue ALP command
            d7asp_queue_alp_actions(&fifo_config, ptr, alp_command_length);
        }
        else if(alp_interface_id == ALP_ITF_ID_FS)
        {
            alp_cmd_handler_process_fs_itf(payload, length);
        }
        else if(alp_interface_id == ALP_ITF_ID_APP)
        {
            if(alp_cmd_handler_appl_itf_cb != NULL)
              alp_cmd_handler_appl_itf_cb(payload, length);
        }
    }
}

void alp_cmd_handler_process_fs_itf(uint8_t* alp_command, uint8_t alp_command_length)
{
    uint8_t serial_interface_frame[128] = { 0x00 };
    uint8_t* ptr = serial_interface_frame;

    (*ptr) = 0xC0; ptr++;               // serial interface sync byte
    (*ptr) = 0x00; ptr++;               // serial interface version

    uint8_t alp_reponse_length = 0;
    alp_process_command_fs_itf(alp_command, alp_command_length, ptr + 1, &alp_reponse_length);

    if(alp_reponse_length > 0)
    {
        (*ptr) = alp_reponse_length;
        console_print_bytes(serial_interface_frame, alp_reponse_length + 3);
    }
}

static uint8_t append_interface_status_action(d7asp_result_t* d7asp_result, uint8_t* ptr)
{
  uint8_t* ptr_start = ptr;
  (*ptr) = ALP_OP_RETURN_STATUS + (1 << 6); ptr++;
  (*ptr) = ALP_ITF_ID_D7ASP; ptr++;
  memcpy(ptr, &(d7asp_result->channel), 3); ptr += 3; // TODO might need to reorder fields in channel_id
  memcpy(ptr, &(d7asp_result->rx_level), 1); ptr += 1;
  (*ptr) = d7asp_result->link_budget; ptr++;
  (*ptr) = d7asp_result->status.raw; ptr++;
  (*ptr) = d7asp_result->fifo_token; ptr++;
  (*ptr) = d7asp_result->seqnr; ptr++;
  (*ptr) = d7asp_result->response_to; ptr++;
  (*ptr) = d7asp_result->addressee->ctrl.raw; ptr++;
  uint8_t address_len = d7asp_result->addressee->ctrl.id_type == ID_TYPE_VID? 2 : 8; // TODO according to spec this can be 1 byte as
  memcpy(ptr, d7asp_result->addressee->id, address_len); ptr += address_len;
  return ptr - ptr_start;
}

void alp_cmd_handler_output_unsollicited_response(d7asp_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size)
{
    uint8_t data[MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE] = { 0x00 };
    uint8_t* ptr = data;

    (*ptr) = 0xC0; ptr++;               // serial interface sync byte
    (*ptr) = 0x00; ptr++;               // serial interface version
    ptr++;                              // payload length byte, skip for now and fill later

    ptr += append_interface_status_action(&d7asp_result, ptr);

    // the actual received data ...
    memcpy(ptr, alp_command, alp_command_size); ptr+= alp_command_size;

    data[2] = ptr - (data + 3);       // fill length byte

    console_print_bytes(data, ptr - data);
}

void alp_cmd_handler_set_appl_itf_callback(alp_cmd_handler_appl_itf_callback cb)
{
    alp_cmd_handler_appl_itf_cb = cb;
}
