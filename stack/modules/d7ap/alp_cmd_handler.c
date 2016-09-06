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

#define ALP_CMD_MAX_SIZE 0xFF

#include "types.h"
#include "alp.h"
#include "shell.h"
#include "console.h"
#include "debug.h"
#include "MODULE_D7AP_defs.h"
#include "ng.h"

static alp_cmd_handler_appl_itf_callback NGDEF(_alp_cmd_handler_appl_itf_cb);
#define alp_cmd_handler_appl_itf_cb NG(_alp_cmd_handler_appl_itf_cb)

#define SERIAL_ALP_FRAME_SYNC_BYTE 0xC0
#define SERIAL_ALP_FRAME_VERSION   0x00

void alp_cmd_handler(fifo_t* cmd_fifo)
{
    // AT$D<serial ALP command>
    // where <serial ALP command> is constructed as follows:
    // <sync byte (0xC0)><version (0x00)><length of ALP command (1 byte)><ALP command> // TODO CRC
    // TODO other commands (AT$D to return ALP status)
    if(fifo_get_size(cmd_fifo) > SHELL_CMD_HEADER_SIZE + 2)
    {
        uint8_t byte;
        error_t err;
        fifo_peek(cmd_fifo, &byte, SHELL_CMD_HEADER_SIZE, 1);
        if(byte == SERIAL_ALP_FRAME_SYNC_BYTE)
        {
            err = fifo_peek(cmd_fifo, &byte, SHELL_CMD_HEADER_SIZE + 1, 1); assert(err == SUCCESS);
            assert(byte == SERIAL_ALP_FRAME_VERSION); // only version 0 implemented for now // TODO pop and return error
            uint8_t alp_command_len;
            err = fifo_peek(cmd_fifo, &alp_command_len, SHELL_CMD_HEADER_SIZE + 2, 1); assert(err == SUCCESS);
            if(fifo_get_size(cmd_fifo) < SHELL_CMD_HEADER_SIZE + 3 + alp_command_len)
                return; // ALP command not complete yet, don't pop

            uint8_t alp_command[ALP_CMD_MAX_SIZE] = { 0x00 };
            err = fifo_pop(cmd_fifo, alp_command, SHELL_CMD_HEADER_SIZE + 3); assert(err == SUCCESS); // pop header
            err = fifo_pop(cmd_fifo, alp_command, alp_command_len); assert(err == SUCCESS); // pop full ALP command

            uint8_t alp_response[ALP_CMD_MAX_SIZE] = { 0x00 };
            uint8_t alp_response_len = 0;
            alp_process_command_console_output(alp_command, alp_command_len);
        }
        else
        {
            assert(false); // TODO
        }

//        else if(alp_interface_id == ALP_ITF_ID_APP)
//        {
//            if(alp_cmd_handler_appl_itf_cb != NULL)
//              alp_cmd_handler_appl_itf_cb(payload, length);
//        }
    }
}

void alp_cmd_handler_output_alp_command(uint8_t *alp_command, uint8_t alp_command_len)
{
    console_print_byte(SERIAL_ALP_FRAME_SYNC_BYTE);
    console_print_byte(SERIAL_ALP_FRAME_VERSION);
    console_print_byte(alp_command_len);
    console_print_bytes(alp_command, alp_command_len);
    // TODO crc?
}

static uint8_t append_interface_status_action(d7asp_result_t* d7asp_result, uint8_t* ptr)
{
  uint8_t* ptr_start = ptr;
  (*ptr) = ALP_OP_RETURN_STATUS + (1 << 6); ptr++;
  (*ptr) = ALP_ITF_ID_D7ASP; ptr++;
  memcpy(ptr, &(d7asp_result->channel), 3); ptr += 3; // TODO might need to reorder fields in channel_id
  memcpy(ptr, &(d7asp_result->rx_level), 1); ptr += 1;
  (*ptr) = d7asp_result->link_budget; ptr++;
  (*ptr) = d7asp_result->target_rx_level; ptr++;
  (*ptr) = d7asp_result->status.raw; ptr++;
  (*ptr) = d7asp_result->fifo_token; ptr++;
  (*ptr) = d7asp_result->seqnr; ptr++;
  (*ptr) = d7asp_result->response_to; ptr++;
  (*ptr) = d7asp_result->addressee->ctrl.raw; ptr++;
  uint8_t address_len = d7anp_addressee_id_length(d7asp_result->addressee->ctrl.id_type);
  memcpy(ptr, d7asp_result->addressee->id, address_len); ptr += address_len;
  return ptr - ptr_start;
}

void alp_cmd_handler_output_d7asp_response(d7asp_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size)
{
    // TODO refactor, move partly to alp + call from SP when shell enabled instead of from app
    uint8_t data[MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE] = { 0x00 };
    uint8_t* ptr = data;

    (*ptr) = SERIAL_ALP_FRAME_SYNC_BYTE; ptr++;               // serial interface sync byte
    (*ptr) = SERIAL_ALP_FRAME_VERSION; ptr++;               // serial interface version
    ptr++;                              // payload length byte, skip for now and fill later

    ptr += append_interface_status_action(&d7asp_result, ptr);

    // the actual received data ...
    memcpy(ptr, alp_command, alp_command_size); ptr+= alp_command_size;

    data[2] = ptr - (data + 3);       // fill length byte

    console_print_bytes(data, ptr - data);
}

void alp_cmd_handler_output_command_completed(uint8_t tag_id, bool error)
{
  // TODO refactor, move partly to alp + call from SP when shell enabled instead of from app
  uint8_t data[MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE] = { 0x00 };
  uint8_t* ptr = data;

  (*ptr) = SERIAL_ALP_FRAME_SYNC_BYTE; ptr++;               // serial interface sync byte
  (*ptr) = SERIAL_ALP_FRAME_VERSION; ptr++;               // serial interface version
  ptr++;                              // payload length byte, skip for now and fill later

  alp_control_tag_response_t control = {
    .operation = ALP_OP_RETURN_TAG,
    .error = error
  };

  (*ptr) = control.raw; ptr++;
  (*ptr) = tag_id; ptr++;

  data[2] = ptr - (data + 3);       // fill length byte

  console_print_bytes(data, ptr - data);
}

void alp_cmd_handler_set_appl_itf_callback(alp_cmd_handler_appl_itf_callback cb)
{
    alp_cmd_handler_appl_itf_cb = cb;
}
