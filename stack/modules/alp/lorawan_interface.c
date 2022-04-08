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
 * \author	kwinten.schram@aloxy.io
 */

#include "alp_layer.h"
#include "lorawan_stack.h"
#include "string.h"
#include "log.h"
#include "errors.h"
#include "framework_defs.h"
#include "MODULE_ALP_defs.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_ALP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#define DPRINT_DATA(p, n) log_print_data(p, n)
#else
#define DPRINT(...)
#define DPRINT_DATA(p, n)
#endif


static void lorawan_error_handler(uint16_t* trans_id, lorawan_stack_status_t status);

static alp_interface_t interface_lorawan_otaa;
static uint16_t lorawan_trans_id;
static alp_interface_config_lorawan_otaa_t lorawan_itf_cfg;

void lorawan_rx(lorawan_AppData_t *AppData)
{
    DPRINT("command from LoRaWAN");
    alp_command_t* command = alp_layer_command_alloc(false, false);
    if (command == NULL) {
        assert(false); // TODO error handling
    }

    command->origin_itf_id = ALP_ITF_ID_LORAWAN_OTAA;
    command->respond_when_completed = false;

    fifo_put(&command->alp_command_fifo, AppData->Buff, AppData->BuffSize);
    //alp_layer_process(command->alp_command, sizeof (&command->alp_command_fifo));
    if(alp_layer_process(command)) {
        lorawan_itf_cfg.lorawan_session_config_otaa.application_port = AppData->Port; //if there will be a response, send it on the port the downlink arrived on
    }
}

void add_interface_status_lorawan(uint8_t* payload, uint8_t attempts, lorawan_stack_status_t status) {
    lorawan_session_result_t* interface_status = (lorawan_session_result_t*) payload;
    interface_status->attempts = attempts;
    interface_status->error_state = status;
    interface_status->duty_cycle_wait_time = lorawan_get_duty_cycle_delay();
}

void lorawan_command_completed(lorawan_stack_status_t status, uint8_t attempts)
{
    error_t status_buffer = (error_t)status;
    alp_interface_status_t result = (alp_interface_status_t) {
        .itf_id = ALP_ITF_ID_LORAWAN_OTAA,
        .len = 4,
        };
    add_interface_status_lorawan(result.itf_status, attempts, status);

    alp_layer_forwarded_command_completed(lorawan_trans_id, &status_buffer, &result, true);
}

static void lorawan_status_callback(lorawan_stack_status_t status, uint8_t attempts)
{

    alp_interface_status_t result = (alp_interface_status_t) {
        .itf_id = ALP_ITF_ID_LORAWAN_OTAA,
        .len = 4
    };
    add_interface_status_lorawan(result.itf_status, attempts, status);

    alp_layer_forwarded_command_completed(lorawan_trans_id, NULL, &result, (status == LORAWAN_STACK_JOIN_FAILED) || (status == LORAWAN_STACK_JOINED)); 
}

static error_t lorawan_send_otaa(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* itf_cfg)
{
    (void)expected_response_length; // suppress unused warning
    
    if(itf_cfg != NULL) { //if NULL, this is a response, use previous cfg
        lorawan_itf_cfg = *(alp_interface_config_lorawan_otaa_t*)itf_cfg;
    }
    (*trans_id) = ++lorawan_trans_id;
    lorawan_stack_status_t status = lorawan_otaa_is_joined(&lorawan_itf_cfg.lorawan_session_config_otaa);
    if(status == LORAWAN_STACK_ERROR_OK) {
        DPRINT("sending otaa payload");
        status = lorawan_stack_send(payload, payload_length, lorawan_itf_cfg.lorawan_session_config_otaa.application_port, lorawan_itf_cfg.lorawan_session_config_otaa.request_ack);
        if(status != LORAWAN_STACK_ERROR_OK)
            lorawan_trans_id--; // we don't need to keep track of this new transid as it is completed immediately
    } else { 
        DPRINT("OTAA not joined yet");
        if(status == LORAWAN_STACK_ALREADY_JOINING)
            lorawan_trans_id--; // we don't need to keep track of this new transid as it is completed immediately
    }
    lorawan_error_handler(trans_id, status);
    return SUCCESS;
}

static void lorawan_error_handler(uint16_t* trans_id, lorawan_stack_status_t status) {
    if(status != LORAWAN_STACK_ERROR_OK) {
        log_print_string("!!!LORAWAN ERROR: %d\n", status);
        error_t status_buffer = (error_t)status;
        alp_interface_status_t result = (alp_interface_status_t) {
            .itf_id = ALP_ITF_ID_LORAWAN_OTAA,
            .len = 4
        };
        add_interface_status_lorawan(result.itf_status, 1, status);
        alp_layer_forwarded_command_completed(*trans_id, &status_buffer, &result, !((status == LORAWAN_STACK_RETRY_TRANSMISSION) || (status == LORAWAN_STACK_DUTY_CYCLE_DELAY) || (status == LORAWAN_STACK_ERROR_NOT_JOINED)));
    }
}

static error_t lorawan_init_otaa() {
    return lorawan_stack_init_otaa();
}

void lorawan_interface_register() {
    lorawan_register_cbs(lorawan_rx, lorawan_command_completed, lorawan_status_callback);

    interface_lorawan_otaa = (alp_interface_t) {
        .itf_id = 0x03,
        .itf_cfg_len = sizeof(lorawan_session_config_otaa_t),
        .itf_status_len = 7,
        .init = lorawan_init_otaa,
        .deinit = lorawan_stack_deinit,
        .send_command = lorawan_send_otaa,
        .unique = true
    };
    alp_layer_register_interface(&interface_lorawan_otaa);
}
