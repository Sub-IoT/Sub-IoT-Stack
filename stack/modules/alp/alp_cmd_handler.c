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

#include "hwatomic.h"
#include "types.h"
#include "alp_layer.h"
#include "shell.h"
#include "console.h"
#include "debug.h"
#include "d7ap.h"
#include "ng.h"
#include "log.h"

#if defined(MODULE_ALP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#else
#define DPRINT(...)
#endif


static alp_cmd_handler_appl_itf_callback NGDEF(_alp_cmd_handler_appl_itf_cb);
#define alp_cmd_handler_appl_itf_cb NG(_alp_cmd_handler_appl_itf_cb)

static uint8_t alp_command[ALP_CMD_MAX_SIZE] = { 0x00 };
static uint8_t alp_resp[ALP_CMD_MAX_SIZE] = { 0x00 };

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

            if(fifo_get_size(cmd_fifo) >= SHELL_CMD_HEADER_SIZE + 3 + alp_command_len)
            {
                start_atomic();
                    err = fifo_pop(cmd_fifo, alp_command, SHELL_CMD_HEADER_SIZE + 3); assert(err == SUCCESS); // pop header
                    err = fifo_pop(cmd_fifo, alp_command, alp_command_len); assert(err == SUCCESS); // pop full ALP command
                end_atomic();

                alp_layer_process_command_console_output(alp_command, alp_command_len);
            }
            else
            {
                //DPRINT("ALP command not complete yet");
            }
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

void alp_cmd_handler_output_alp_command(fifo_t* resp_fifo)
{
    uint8_t resp_len = fifo_get_size(resp_fifo);
    if(resp_len > 0) {
        DPRINT("output ALP cmd of size %i", resp_len);
        console_print_byte(SERIAL_ALP_FRAME_SYNC_BYTE);
        console_print_byte(SERIAL_ALP_FRAME_VERSION);
        console_print_byte(resp_len);
        fifo_pop(resp_fifo, alp_resp, resp_len);
        console_print_bytes(alp_resp, resp_len);
    }
    // TODO crc?
}


void alp_cmd_handler_set_appl_itf_callback(alp_cmd_handler_appl_itf_callback cb)
{
    alp_cmd_handler_appl_itf_cb = cb;
}


static uint8_t append_interface_status_action(d7ap_session_result_t* d7asp_result, uint8_t* ptr)
{
  // TODO refactor: duplicate of add_interface_status_action() in alp.c??
  uint8_t* ptr_start = ptr;
  (*ptr) = ALP_OP_RETURN_STATUS + (1 << 6); ptr++;
  (*ptr) = ALP_ITF_ID_D7ASP; ptr++;
  (*ptr) = d7asp_result->channel.channel_header; ptr++;
  uint16_t center_freq_index_be = __builtin_bswap16(d7asp_result->channel.center_freq_index);
  memcpy(ptr, &center_freq_index_be, 2); ptr += 2;
  memcpy(ptr, &(d7asp_result->rx_level), 1); ptr += 1;
  (*ptr) = d7asp_result->link_budget; ptr++;
  (*ptr) = d7asp_result->target_rx_level; ptr++;
  (*ptr) = d7asp_result->status.raw; ptr++;
  (*ptr) = d7asp_result->fifo_token; ptr++;
  (*ptr) = d7asp_result->seqnr; ptr++;
  (*ptr) = d7asp_result->response_to; ptr++;
  (*ptr) = d7asp_result->addressee.ctrl.raw; ptr++;
  (*ptr) = d7asp_result->addressee.access_class; ptr++;
  uint8_t address_len = d7ap_addressee_id_length(d7asp_result->addressee.ctrl.id_type);
  memcpy(ptr, d7asp_result->addressee.id, address_len); ptr += address_len;
  return ptr - ptr_start;
}

// TODO remove after refactoring (SP should pass unsolicited resp to ALP layer, which will output if shell enabled)
void alp_cmd_handler_output_d7asp_response(d7ap_session_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size)
{
    // TODO refactor, move partly to alp + call from SP when shell enabled instead of from app
    DPRINT("output D7ASP response to console");
    uint8_t* ptr = alp_resp;

    (*ptr) = SERIAL_ALP_FRAME_SYNC_BYTE; ptr++;               // serial interface sync byte
    (*ptr) = SERIAL_ALP_FRAME_VERSION; ptr++;               // serial interface version
    ptr++;                              // payload length byte, skip for now and fill later

    ptr += append_interface_status_action(&d7asp_result, ptr);

    // the actual received data ...
    memcpy(ptr, alp_command, alp_command_size); ptr+= alp_command_size;

    alp_resp[2] = ptr - (alp_resp + 3);       // fill length byte

    console_print_bytes(alp_resp, ptr - alp_resp);
}

