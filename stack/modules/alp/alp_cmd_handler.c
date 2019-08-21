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
#include "string.h"
#include "alp_layer.h"
#include "shell.h"
#include "debug.h"
#include "d7ap.h"
#include "ng.h"
#include "log.h"
#include "MODULE_ALP_defs.h"
#include "modem_interface.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_ALP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif


static alp_cmd_handler_appl_itf_callback NGDEF(_alp_cmd_handler_appl_itf_cb);
#define alp_cmd_handler_appl_itf_cb NG(_alp_cmd_handler_appl_itf_cb)

static uint8_t alp_command[ALP_CMD_MAX_SIZE] = { 0x00 };
static uint8_t alp_resp[ALP_CMD_MAX_SIZE] = { 0x00 };

alp_interface_t alp_modem_interface;

error_t alp_cmd_send_output(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* session_config);

void modem_interface_cmd_handler(fifo_t* cmd_fifo)
{
    error_t err;
    uint8_t alp_command_len=fifo_get_size(cmd_fifo);
    start_atomic();
    err = fifo_pop(cmd_fifo, alp_command, alp_command_len); assert(err == SUCCESS); // pop full ALP command
    end_atomic();
    DPRINT_DATA(alp_command, alp_command_len);
    alp_interface_status_t temp_status = {
        .itf_id = ALP_ITF_ID_SERIAL,
        .len = 0
    };
    alp_modem_interface.receive_cb(alp_command, alp_command_len, NULL, &temp_status);
}

void alp_cmd_handler_output_alp_command(fifo_t* resp_fifo)
{
    uint8_t resp_len = fifo_get_size(resp_fifo);
    if(resp_len > 0) {

        fifo_pop(resp_fifo, alp_resp,resp_len);
        modem_interface_transfer_bytes(alp_resp, resp_len, SERIAL_MESSAGE_TYPE_ALP_DATA);
    }

}

void alp_cmd_handler_register_interface() {
    alp_modem_interface = (alp_interface_t) {
        .itf_id = ALP_ITF_ID_SERIAL,
        .itf_cfg_len = 0,
        .itf_status_len = 0,
        .receive_cb = NULL,
        .command_completed_cb = NULL,
        .response_cb = NULL,
        .transmit_cb = alp_cmd_send_output,
        .init_cb = NULL,
        .deinit_cb = NULL,
        .unique = false
    };

    alp_layer_register_interface(&alp_modem_interface);

    modem_interface_register_handler(&modem_interface_cmd_handler, SERIAL_MESSAGE_TYPE_ALP_DATA);

    DPRINT("registered interface and handler");
}

void alp_cmd_handler_set_appl_itf_callback(alp_cmd_handler_appl_itf_callback cb)
{
    alp_cmd_handler_appl_itf_cb = cb;
}

static uint8_t append_interface_status_action(d7ap_session_result_t* d7asp_result, uint8_t* ptr)
{
  // TODO refactor: duplicate of add_interface_status_action() in alp.c??
  uint8_t* ptr_start = ptr;
  (*ptr) = ALP_OP_STATUS + (1 << 6); ptr++;
  (*ptr) = ALP_ITF_ID_D7ASP; ptr++;
  (*ptr) = 12 + d7ap_addressee_id_length(d7asp_result->addressee.ctrl.id_type); ptr++;
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

error_t alp_cmd_send_output(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* session_config) {
    DPRINT("sending payload to modem");
    DPRINT_DATA(payload, payload_length);
    modem_interface_transfer_bytes(payload, payload_length, SERIAL_MESSAGE_TYPE_ALP_DATA);
}

// TODO remove after refactoring (SP should pass unsolicited resp to ALP layer, which will output if shell enabled)
void alp_cmd_handler_output_response(alp_interface_status_t result, uint8_t *alp_command, uint8_t alp_command_size)
{
    // TODO refactor, move partly to alp + call from SP when shell enabled instead of from app
    DPRINT("output D7ASP response to console");
    DPRINT_DATA(alp_command, alp_command_size);
    uint8_t* ptr = alp_resp;

    memcpy(ptr, result.data, result.len);
    ptr += result.len;

    // the actual received data ...
    memcpy(ptr, alp_command, alp_command_size); 
    
    ptr+= alp_command_size;

    modem_interface_transfer_bytes(alp_resp, ptr - alp_resp, SERIAL_MESSAGE_TYPE_ALP_DATA);
}

