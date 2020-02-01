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

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_ALP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#define DPRINT_DATA(p, n) log_print_data(p, n)
#else
#define DPRINT(...)
#define DPRINT_DATA(p, n)
#endif


static void lorawan_error_handler(uint16_t* trans_id, lorawan_stack_status_t status);

static alp_itf_id_t current_lorawan_interface_type = ALP_ITF_ID_LORAWAN_OTAA;
alp_interface_t interface_lorawan_otaa;
alp_interface_t interface_lorawan_abp;
uint16_t lorawan_trans_id;
bool otaa_just_inited = false;
bool abp_just_inited = false;

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
    payload[0] = ALP_OP_STATUS + (1 << 6);
    payload[1] = current_lorawan_interface_type;
    payload[2] = 4; //length
    payload[3] = attempts;
    payload[4] = status;
    uint16_t wait_time = __builtin_bswap16(lorawan_get_duty_cycle_delay());
    memcpy(&payload[5], (uint8_t*)&wait_time, 2);
}

    void lorawan_command_completed(lorawan_stack_status_t status, uint8_t attempts) {
    error_t status_buffer = (error_t)status;
    alp_interface_status_t result = (alp_interface_status_t) {
        .itf_id = current_lorawan_interface_type,
        .len = 7,
        };
    add_interface_status_lorawan(result.itf_status, attempts, status);

    alp_layer_forwarded_command_completed(lorawan_trans_id, &status_buffer, &result);
}

static void lorawan_status_callback(lorawan_stack_status_t status, uint8_t attempts)
{
    alp_command_t* command = alp_layer_command_alloc(false, false);
    command->respond_when_completed=true;

    alp_interface_status_t result = (alp_interface_status_t) {
        .itf_id = current_lorawan_interface_type,
        .len = 7
    };
    add_interface_status_lorawan(result.itf_status, attempts, status);

    alp_layer_forwarded_command_completed(command->trans_id, NULL, &result);
}

static error_t lorawan_send_otaa(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* itf_cfg)
{
    alp_interface_config_lorawan_otaa_t* lorawan_itf_cfg = (alp_interface_config_lorawan_otaa_t*)itf_cfg;
    if(!otaa_just_inited && (lorawan_otaa_is_joined(&lorawan_itf_cfg->lorawan_session_config_otaa))) {
        DPRINT("sending otaa payload");
        current_lorawan_interface_type = ALP_ITF_ID_LORAWAN_OTAA;
        lorawan_stack_status_t status = lorawan_stack_send(payload, payload_length, lorawan_itf_cfg->lorawan_session_config_otaa.application_port, lorawan_itf_cfg->lorawan_session_config_otaa.request_ack);
        lorawan_error_handler(trans_id, status);
    } else { //OTAA not joined yet or still initing
        DPRINT("OTAA not joined yet");
        otaa_just_inited = false;
        alp_command_t* command = alp_layer_get_command_by_transid(*trans_id, ALP_ITF_ID_LORAWAN_OTAA);
        // TODO why? fifo_put(&command->alp_response_fifo, payload, payload_length);
        lorawan_error_handler(trans_id, LORAWAN_STACK_ERROR_NOT_JOINED);
    }
    return SUCCESS;
}

static error_t lorawan_send_abp(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* itf_cfg) {
    alp_interface_config_lorawan_abp_t* lorawan_itf_cfg = (alp_interface_config_lorawan_abp_t*)itf_cfg;
    if(!abp_just_inited) {
        lorawan_itf_cfg->lorawan_session_config_abp.devAddr = __builtin_bswap32(lorawan_itf_cfg->lorawan_session_config_abp.devAddr);
        lorawan_itf_cfg->lorawan_session_config_abp.network_id = __builtin_bswap32(lorawan_itf_cfg->lorawan_session_config_abp.network_id);
        lorawan_abp_is_joined(&lorawan_itf_cfg->lorawan_session_config_abp);
    } else
        abp_just_inited = false;
    DPRINT("sending abp payload");
    current_lorawan_interface_type = ALP_ITF_ID_LORAWAN_ABP;
    lorawan_stack_status_t status = lorawan_stack_send(payload, payload_length, lorawan_itf_cfg->lorawan_session_config_abp.application_port, lorawan_itf_cfg->lorawan_session_config_abp.request_ack);
    lorawan_error_handler(trans_id, status);
    return SUCCESS;
}

static void lorawan_error_handler(uint16_t* trans_id, lorawan_stack_status_t status) {
    if(status != LORAWAN_STACK_ERROR_OK) {
        log_print_string("!!!LORAWAN ERROR: %d\n", status);
        error_t status_buffer = (error_t)status;
        alp_interface_status_t result = (alp_interface_status_t) {
            .itf_id = current_lorawan_interface_type,
            .len = 7
        };
        add_interface_status_lorawan(result.itf_status, 1, status);
        alp_layer_forwarded_command_completed(*trans_id, &status_buffer, &result);
    } else
    lorawan_trans_id = *trans_id;
}

static void lorawan_init_otaa(alp_interface_config_t* itf_cfg) {
    lorawan_stack_init_otaa(&((alp_interface_config_lorawan_otaa_t*)itf_cfg)->lorawan_session_config_otaa);
    otaa_just_inited = true;
}

static void lorawan_init_abp(alp_interface_config_t* itf_cfg) {
    alp_interface_config_lorawan_abp_t* lorawan_itf_cfg = (alp_interface_config_lorawan_abp_t*)itf_cfg;
    lorawan_itf_cfg->lorawan_session_config_abp.devAddr = __builtin_bswap32(lorawan_itf_cfg->lorawan_session_config_abp.devAddr);
    lorawan_itf_cfg->lorawan_session_config_abp.network_id = __builtin_bswap32(lorawan_itf_cfg->lorawan_session_config_abp.network_id);
    lorawan_stack_init_abp(&lorawan_itf_cfg->lorawan_session_config_abp);
    abp_just_inited = true;
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

    interface_lorawan_abp = (alp_interface_t) {
        .itf_id = ALP_ITF_ID_LORAWAN_ABP,
        .itf_cfg_len = sizeof(lorawan_session_config_abp_t),
        .itf_status_len = 7,
        .init = lorawan_init_abp,
        .deinit = lorawan_stack_deinit,
        .send_command = lorawan_send_abp,
        .unique = true
    };
    alp_layer_register_interface(&interface_lorawan_abp);

}
