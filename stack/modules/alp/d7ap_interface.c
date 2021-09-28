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


#include "alp_layer.h"
#include "string.h"
#include "log.h"
#include "MODULE_ALP_defs.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_ALP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#define DPRINT_DATA(p, n) log_print_data(p, n)
#else
#define DPRINT(...)
#define DPRINT_DATA(p, n)
#endif

static void response_from_d7ap(uint16_t trans_id, uint8_t* payload, uint8_t len, d7ap_session_result_t result);
static bool command_from_d7ap(uint8_t* payload, uint8_t len, d7ap_session_result_t result, bool response_expected);
static void d7ap_command_completed(uint16_t trans_id, error_t error);

static alp_interface_t d7_alp_interface;
static uint8_t alp_client_id;
static bool inited = false;

static error_t d7ap_interface_init()
{
    if(inited)
        return EALREADY;
    d7ap_init();

    d7ap_resource_desc_t alp_desc = {
        .receive_cb = response_from_d7ap,
        .transmitted_cb = d7ap_command_completed,
        .unsolicited_cb = command_from_d7ap
    };  
    alp_client_id = d7ap_register(&alp_desc);
    inited = true;

    DPRINT("alp_client_id is %i",alp_client_id);
    return SUCCESS;
}

static void d7ap_interface_stop()
{
    if(!inited)
        return;
    d7ap_stop();
    inited = false;
}

static alp_interface_status_t serialize_session_result_to_alp_interface_status(const d7ap_session_result_t* session_result)
{
    alp_interface_status_t d7_status = (alp_interface_status_t) {
        .itf_id = ALP_ITF_ID_D7ASP,
        .len = 12 + d7ap_addressee_id_length(session_result->addressee.ctrl.id_type),
    };

    uint8_t* ptr = d7_status.itf_status;
    (*ptr) = session_result->channel.channel_header;
    ptr++;
    uint16_t center_freq = __builtin_bswap16(session_result->channel.center_freq_index);
    memcpy(ptr, &center_freq, 2);
    ptr += 2;
    (*ptr) = session_result->rx_level;
    ptr++;
    (*ptr) = session_result->link_budget;
    ptr++;
    (*ptr) = session_result->target_rx_level;
    ptr++;
    (*ptr) = session_result->status.raw;
    ptr++;
    (*ptr) = session_result->fifo_token;
    ptr++;
    (*ptr) = session_result->seqnr;
    ptr++;
    (*ptr) = session_result->response_to;
    ptr++;
    (*ptr) = session_result->addressee.ctrl.raw;
    ptr++;
    (*ptr) = session_result->addressee.access_class;
    ptr++;
    memcpy(ptr, session_result->addressee.id, d7ap_addressee_id_length(session_result->addressee.ctrl.id_type));

    return d7_status;
}

static void response_from_d7ap(uint16_t trans_id, uint8_t* payload, uint8_t len, d7ap_session_result_t result) {
    DPRINT("got response from d7 of trans %i with len %i and result linkbudget %i", trans_id, len, result.link_budget);
    DPRINT_DATA(payload, len);
    alp_interface_status_t d7_status = serialize_session_result_to_alp_interface_status(&result);
    DPRINT_DATA(d7_status.itf_status, d7_status.len);
    alp_layer_received_response(trans_id, payload, len, &d7_status);
}

static bool command_from_d7ap(uint8_t* payload, uint8_t len, d7ap_session_result_t result, bool response_expected) {
    DPRINT("command from d7 with len %i result linkbudget %i", len, result.link_budget);
    alp_interface_status_t d7_status = serialize_session_result_to_alp_interface_status(&result);
    alp_command_t* command = alp_layer_command_alloc(false, false);
    if (command == NULL) {
        assert(false); // TODO error handling
    }
    
    command->origin_itf_id = ALP_ITF_ID_D7ASP;
    command->respond_when_completed = response_expected;
    alp_append_interface_status(command, &d7_status);
    fifo_put(&command->alp_command_fifo, payload, len);
    //return alp_layer_process(command->alp_command, fifo_get_size((&command->alp_command_fifo)));
    return alp_layer_process(command);
}

static error_t d7ap_alp_send(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* itf_cfg) {
    DPRINT("sending D7 packet");

    if(itf_cfg != NULL) {
        return d7ap_send(alp_client_id, (d7ap_session_config_t*)&itf_cfg->itf_config, payload, payload_length, expected_response_length, trans_id);
    } else {
        return d7ap_send(alp_client_id, NULL, payload, payload_length, expected_response_length, trans_id);
    }
}

static alp_interface_status_t empty_itf_status = {
    .itf_id = ALP_ITF_ID_D7ASP,
    .len = 0
};

static void d7ap_command_completed(uint16_t trans_id, error_t error) {
    // TODO D7ASP ALP status maps to D7ASP Result, which is only relevant for responses and not for the command (dialog as a whole)
    // TBD in D7 PAG, for now supply 'empty' D7 status
    alp_layer_forwarded_command_completed(trans_id, &error, &empty_itf_status, true);
}

void d7ap_interface_register()
{
    d7_alp_interface = (alp_interface_t) {
        .itf_id = 0xD7,
        .itf_cfg_len = sizeof(d7ap_session_config_t),
        .itf_status_len = sizeof(d7ap_session_result_t),
        .send_command = d7ap_alp_send,
        .init = d7ap_interface_init,
        .deinit = d7ap_interface_stop,
        .unique = true
    };
    
    alp_layer_register_interface(&d7_alp_interface);
}
