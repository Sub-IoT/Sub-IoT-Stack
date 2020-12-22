/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2019 Aloxy
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

static alp_itf_id_t current_lorawan_interface_type = ALP_ITF_ID_LORAWAN_OTAA;
static alp_interface_t interface_lorawan_otaa;
static uint16_t lorawan_trans_id;
static bool otaa_just_inited = false;

void lorawan_rx(lorawan_AppData_t *AppData)
{
    DPRINT("command from LoRaWAN");
    alp_command_t* command = alp_layer_command_alloc(false, false);
    if (command == NULL) {
        assert(false); // TODO error handling
    }

    command->origin_itf_id = current_lorawan_interface_type;
    command->respond_when_completed = false;

    fifo_put(&command->alp_command_fifo, AppData->Buff, AppData->BuffSize);
    //alp_layer_process(command->alp_command, sizeof (&command->alp_command_fifo));
    alp_layer_process(command);
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
        .itf_id = current_lorawan_interface_type,
        .len = 4,
        };
    add_interface_status_lorawan(result.itf_status, attempts, status);

    alp_layer_forwarded_command_completed(lorawan_trans_id, &status_buffer, &result, true);
}

static void lorawan_status_callback(lorawan_stack_status_t status, uint8_t attempts)
{

    alp_interface_status_t result = (alp_interface_status_t) {
        .itf_id = current_lorawan_interface_type,
        .len = 4
    };
    add_interface_status_lorawan(result.itf_status, attempts, status);

    alp_layer_forwarded_command_completed(lorawan_trans_id, NULL, &result, (status == LORAWAN_STACK_JOIN_FAILED) || (status == LORAWAN_STACK_JOINED)); 
}

static error_t lorawan_send_otaa(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* itf_cfg)
{
    (void)expected_response_length; // suppress unused warning
    alp_interface_config_lorawan_otaa_t* lorawan_itf_cfg = (alp_interface_config_lorawan_otaa_t*)itf_cfg;
    (*trans_id) = ++lorawan_trans_id;
    if(!otaa_just_inited && (lorawan_otaa_is_joined(&lorawan_itf_cfg->lorawan_session_config_otaa))) {
        DPRINT("sending otaa payload");
        current_lorawan_interface_type = ALP_ITF_ID_LORAWAN_OTAA;
        lorawan_stack_status_t status = lorawan_stack_send(payload, payload_length, lorawan_itf_cfg->lorawan_session_config_otaa.application_port, lorawan_itf_cfg->lorawan_session_config_otaa.request_ack);
        lorawan_error_handler(trans_id, status);
    } else { //OTAA not joined yet or still initing
        DPRINT("OTAA not joined yet");
        otaa_just_inited = false;
        // alp_command_t* command = alp_layer_get_command_by_transid(*trans_id, ALP_ITF_ID_LORAWAN_OTAA);
        // TODO why? fifo_put(&command->alp_response_fifo, payload, payload_length);
        lorawan_error_handler(trans_id, LORAWAN_STACK_ERROR_NOT_JOINED);
    }
    return SUCCESS;
}

static void lorawan_error_handler(uint16_t* trans_id, lorawan_stack_status_t status) {
    if(status != LORAWAN_STACK_ERROR_OK) {
        log_print_string("!!!LORAWAN ERROR: %d\n", status);
        error_t status_buffer = (error_t)status;
        alp_interface_status_t result = (alp_interface_status_t) {
            .itf_id = current_lorawan_interface_type,
            .len = 4
        };
        add_interface_status_lorawan(result.itf_status, 1, status);
        alp_layer_forwarded_command_completed(*trans_id, &status_buffer, &result, false);
    } else {
        lorawan_trans_id = *trans_id;
    }
}

static void lorawan_init_otaa(alp_interface_config_t* itf_cfg) {
    lorawan_stack_init_otaa(&((alp_interface_config_lorawan_otaa_t*)itf_cfg)->lorawan_session_config_otaa);
    otaa_just_inited = true;
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
