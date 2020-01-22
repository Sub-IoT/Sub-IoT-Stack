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
#include "hwsystem.h"
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
#include "platform_defs.h"
#include "platform.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_ALP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif

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

    alp_layer_process_command(alp_command, alp_command_len, ALP_ITF_ID_SERIAL, NULL);
}

// TODO move
void alp_cmd_handler_register_interface() {
    alp_modem_interface = (alp_interface_t) {
        .itf_id = ALP_ITF_ID_SERIAL,
        .itf_cfg_len = 0,
        .itf_status_len = 0,
        .send_command = alp_cmd_send_output,
        .init = NULL,
        .deinit = NULL,
        .unique = false
    };

    alp_layer_register_interface(&alp_modem_interface);

#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
    modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, SERIAL_MODEM_INTERFACE_UART_STATE_PIN, SERIAL_MODEM_INTERFACE_TARGET_UART_STATE_PIN);
#else
    modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, (pin_id_t) 0, (pin_id_t) 0);
#endif

    modem_interface_register_handler(&modem_interface_cmd_handler, SERIAL_MESSAGE_TYPE_ALP_DATA);

    DPRINT("registered interface and handler");
}

error_t alp_cmd_send_output(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* session_config) {
    DPRINT("sending payload to modem");
    DPRINT_DATA(payload, payload_length);
    modem_interface_transfer_bytes(payload, payload_length, SERIAL_MESSAGE_TYPE_ALP_DATA);

    return SUCCESS;
}

