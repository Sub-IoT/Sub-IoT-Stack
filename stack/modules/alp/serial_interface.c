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

/*
 * \author	glenn.ergeerts@uantwerpen.be
 */

#include "serial_interface.h"

#define ALP_CMD_MAX_SIZE 0xFF

#include "hwatomic.h"
#include "hwsystem.h"
#include "types.h"
#include "string.h"
#include "alp_layer.h"
#include "debug.h"
#include "ng.h"
#include "log.h"
#include "MODULE_ALP_defs.h"
#include "modem_interface.h"
#include "platform_defs.h"
#include "platform.h"
#include "framework_defs.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_ALP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif

static uint8_t alp_command[ALP_CMD_MAX_SIZE] = { 0x00 };

alp_interface_t alp_modem_interface;

static error_t serial_interface_send(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* session_config);

static alp_interface_status_t serial_itf_status = {
    .itf_id = ALP_ITF_ID_SERIAL,
    .len = 0
};

static void serial_interface_cmd_handler(fifo_t* cmd_fifo)
{
    error_t err;
    uint8_t alp_command_len=fifo_get_size(cmd_fifo);
    start_atomic();
    err = fifo_pop(cmd_fifo, alp_command, alp_command_len); assert(err == SUCCESS); // pop full ALP command
    end_atomic();

    alp_command_t* command = alp_layer_command_alloc(false, false);
    if(command == NULL)
    {
        log_print_error_string("serial_interface_cmd_handler: unable to allocate alp command");
        return;

    }
    command->origin_itf_id = ALP_ITF_ID_SERIAL;
    alp_append_interface_status(command, &serial_itf_status);
    fifo_put(&command->alp_command_fifo, alp_command, alp_command_len);
    // alp_layer_process(command->alp_command, fifo_get_size(&command->alp_command_fifo));
    alp_layer_process(command);
}

void serial_interface_register() {
    alp_modem_interface = (alp_interface_t) {
        .itf_id = ALP_ITF_ID_SERIAL,
        .itf_cfg_len = 0,
        .itf_status_len = 0,
        .send_command = &serial_interface_send,
        .init = NULL,
        .deinit = NULL,
        .unique = false
    };

    modem_interface_init();

    alp_layer_register_interface(&alp_modem_interface);

    modem_interface_register_handler(&serial_interface_cmd_handler, SERIAL_MESSAGE_TYPE_ALP_DATA);
}

static error_t serial_interface_send(uint8_t* payload, uint8_t payload_length,
    __attribute__((__unused__)) uint8_t expected_response_length,
    __attribute__((__unused__)) uint16_t* trans_id,
    __attribute__((__unused__)) alp_interface_config_t* session_config)
{
    DPRINT("sending payload to serial interface");
    DPRINT_DATA(payload, payload_length);
    return modem_interface_transfer_bytes(payload, payload_length, SERIAL_MESSAGE_TYPE_ALP_DATA);
}

