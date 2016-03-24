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
#include "debug.h"
#include "MODULE_D7AP_defs.h"


void alp_cmd_handler(fifo_t* cmd_fifo)
{
    // AT$D<ALP interface ID><Length byte><ALP interface config><ALP command>
    // interface: 0xD7 = D7ASP, 0x00 = own filesystem
    // interface config: D7ASP fifo config in case of interface 0xD7, void for interface 0x00
    // where length is the length of interface config and ALP command
    if(fifo_get_size(cmd_fifo) > SHELL_CMD_HEADER_SIZE + 2)
    {
        uint8_t alp_command[ALP_CMD_HANDLER_HEADER_SIZE + MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE] = { 0x00 };
        fifo_peek(cmd_fifo, alp_command, SHELL_CMD_HEADER_SIZE, ALP_CMD_HANDLER_HEADER_SIZE);
        uint8_t alp_interface_id = alp_command[0];
        assert(alp_interface_id == ALP_ITF_ID_FS || alp_interface_id == ALP_ITF_ID_D7ASP);
        uint8_t length = alp_command[1];
        if(fifo_get_size(cmd_fifo) < SHELL_CMD_HEADER_SIZE + ALP_CMD_HANDLER_HEADER_SIZE + length)
            return; // ALP command not complete yet, don't pop

        fifo_pop(cmd_fifo, alp_command, SHELL_CMD_HEADER_SIZE + ALP_CMD_HANDLER_HEADER_SIZE + length);
        if(alp_interface_id == ALP_ITF_ID_D7ASP)
        {
            // parse D7ASP fifo config
            uint8_t* ptr = alp_command + SHELL_CMD_HEADER_SIZE + ALP_CMD_HANDLER_HEADER_SIZE;
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
            alp_cmd_handler_process_fs_itf(alp_command + SHELL_CMD_HEADER_SIZE + ALP_CMD_HANDLER_HEADER_SIZE, length);
        }
    }
}

void alp_cmd_handler_process_fs_itf(uint8_t* alp_command, uint8_t alp_command_length)
{
    uint8_t alp_response[128] = { 0 };
    uint8_t alp_reponse_length = 0;
    alp_process_command(alp_command, alp_command_length, alp_response, &alp_reponse_length);

    if(alp_reponse_length > 0)
        shell_return_output(ALP_CMD_HANDLER_ID, alp_response, alp_reponse_length); // TODO transmit ALP interface ID as well?
}

void alp_cmd_handler_output_unsollicited_response(d7asp_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size, hw_rx_metadata_t* rx_meta)
{
    uint8_t data[1 + sizeof(hw_rx_metadata_t) + MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE] = { 0x00 };
    uint8_t* ptr = data;
    // TODO transmit ALP interface ID as well?
    // TODO rx meta
    (*ptr) = d7asp_result.status.raw; ptr++;
    (*ptr) = d7asp_result.fifo_token; ptr++;
    (*ptr) = d7asp_result.request_id; ptr++;
    (*ptr) = d7asp_result.response_to; ptr++;
    (*ptr) = d7asp_result.addressee->addressee_ctrl; ptr++;
    uint8_t address_len = d7asp_result.addressee->addressee_ctrl_virtual_id? 2 : 8; // TODO according to spec this can be 1 byte as well?
    memcpy(ptr, d7asp_result.addressee->addressee_id, address_len); ptr += address_len;
    memcpy(ptr, alp_command, alp_command_size); ptr+= alp_command_size;
    shell_return_output(ALP_CMD_HANDLER_ID, data, ptr - data);
}
