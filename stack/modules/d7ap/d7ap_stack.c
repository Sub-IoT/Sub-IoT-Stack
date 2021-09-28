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
 *
 * \author philippe.nunes@cortus.com
 */

#include "framework_defs.h"

#include "bitmap.h"
#include "errors.h"
#include "debug.h"

#include "packet_queue.h"
#include "d7ap_stack.h"
#include "d7asp.h"
#include "d7atp.h"
#include "d7anp.h"
#include "dll.h"
#include "d7ap_fs.h"



#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_LOG_ENABLED)
#include "log.h"
#define DPRINT(...) log_print_stack_string(LOG_STACK_D7AP, __VA_ARGS__)
#define DPRINT_DATA(ptr, len) log_print_data(ptr, len)
#else
#define DPRINT(...)
#define DPRINT_DATA(ptr, len)
#endif

#define INVALID_CLIENT_ID 0xFF

typedef struct {
    bool active;
    uint8_t token;
    uint8_t client_id;
    uint8_t request_nb;
    uint16_t trans_id[MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT];
} session_t;

static session_t sessions[MODULE_D7AP_MAX_SESSION_COUNT];

typedef struct {
    bool active;
    bool expected_response;
    uint8_t token;
} slave_session_t;

static slave_session_t slave_session = {
    .active = false,
    .expected_response = false,
    .token = 0
};

extern d7ap_resource_desc_t registered_client[MODULE_D7AP_MAX_CLIENT_COUNT];
extern uint8_t registered_client_nb;

typedef enum {
    D7AP_STACK_STATE_STOPPED,
    D7AP_STACK_STATE_IDLE,
    D7AP_STACK_STATE_TRANSMITTING,
    D7AP_STACK_STATE_RECEIVING,
    D7AP_STACK_STATE_WAIT_APP_ANSWER
} state_t;

static state_t d7ap_stack_state = D7AP_STACK_STATE_STOPPED;

// TODO document state diagram
static void switch_state(state_t new_state)
{
    switch(new_state)
    {
        case D7AP_STACK_STATE_TRANSMITTING:
            switch(d7ap_stack_state)
            {
                case D7AP_STACK_STATE_STOPPED:
                case D7AP_STACK_STATE_IDLE:
                    d7ap_stack_state = new_state;
                    DPRINT("[D7AP] Switching to state D7AP_STATE_TRANSMITTING");
                    break;
                case D7AP_STACK_STATE_TRANSMITTING:
                    // new requests in fifo, reschedule for later flushing
                    // TODO sched_post_task(&flush_fifos);
                    break;
                default:
                    assert(false);
            }

            break;
        case D7AP_STACK_STATE_RECEIVING:
            switch(d7ap_stack_state)
            {
                case D7AP_STACK_STATE_STOPPED:
                case D7AP_STACK_STATE_IDLE:
                case D7AP_STACK_STATE_WAIT_APP_ANSWER:
                case D7AP_STACK_STATE_RECEIVING:
                    d7ap_stack_state = new_state;
                    DPRINT("[D7AP] Switching to state D7AP_STATE_RECEIVING");
                    break;
                default:
                    assert(false);
            }
            break;

        case D7AP_STACK_STATE_WAIT_APP_ANSWER:
            switch(d7ap_stack_state)
            {
                case D7AP_STACK_STATE_STOPPED:
                case D7AP_STACK_STATE_IDLE:
                    d7ap_stack_state = new_state;
                    DPRINT("[D7AP] Switching to state D7AP_STACK_STATE_WAIT_APP_ANSWER");
                    break;
                default:
                    assert(false);
            }
            break;

        case D7AP_STACK_STATE_IDLE:
            switch(d7ap_stack_state)
            {
                case D7AP_STACK_STATE_RECEIVING:
                case D7AP_STACK_STATE_TRANSMITTING:
                case D7AP_STACK_STATE_WAIT_APP_ANSWER:
                case D7AP_STACK_STATE_IDLE:
                    d7ap_stack_state = new_state;
                    DPRINT("[D7AP] Switching to state D7AP_STACK_STATE_IDLE");
                    break;
                default:
                    assert(false);
            }
            break;

        default:
            assert(false);
    }
}

static void free_session(session_t* session) {
    DPRINT("[D7AP] Free session %i", session->token);
    session->active = false;
    session->request_nb = 0;
    session->client_id = INVALID_CLIENT_ID;
    session->token = 0;
};

static session_t* alloc_session(uint8_t client_id) {
    for(uint8_t i = 0; i < MODULE_D7AP_MAX_SESSION_COUNT; i++) {
        if(sessions[i].client_id == INVALID_CLIENT_ID) {
            sessions[i].client_id = client_id;
        return &(sessions[i]);
        }
    }

    DPRINT("[D7AP] Could not allocate session, all %i reserved slots active", MODULE_D7AP_MAX_SESSION_COUNT);
    return NULL;
}

static void init_session_list()
{
    for(uint8_t i = 0; i < MODULE_D7AP_MAX_SESSION_COUNT; i++) {
        free_session(&sessions[i]);
    }
}

static void on_access_profile_file_changed(uint8_t file_id) {
  DPRINT("invalidate cached APs\n");
  d7atp_notify_access_profile_file_changed(file_id);
  dll_notify_access_profile_file_changed(file_id);
}

void d7ap_stack_init(void)
{
    assert(d7ap_stack_state == D7AP_STACK_STATE_STOPPED);
    d7ap_stack_state = D7AP_STACK_STATE_IDLE;

    d7asp_init();
    d7atp_init();
    d7anp_init();
    packet_queue_init();
    dll_init();
    init_session_list();

    for(int i = 0; i < 15; i++)
      d7ap_fs_register_file_modified_callback(D7A_FILE_ACCESS_PROFILE_ID + i, &on_access_profile_file_changed);
}

void d7ap_stack_stop()
{
    d7asp_stop();
    d7atp_stop();
    d7anp_stop();
    dll_stop();
    hw_radio_stop();

    d7ap_stack_state = D7AP_STACK_STATE_STOPPED;
}

static session_t* get_session_by_session_token(uint8_t session_token)
{
    for(uint8_t i = 0; i < MODULE_D7AP_MAX_SESSION_COUNT; i++) {
        if(sessions[i].token == session_token)
            return &(sessions[i]);
    }

    return NULL;
}

/*static session_t* get_active_session_by_client_id(uint8_t client_id)
{
    for(uint8_t i = 0; i < MODULE_D7AP_MAX_SESSION_COUNT; i++) {
        if(sessions[i].client_id == client_id && sessions[i].active)
            return &(sessions[i]);
    }

    return NULL;
}*/

error_t d7ap_stack_send(uint8_t client_id, d7ap_session_config_t* config, uint8_t* payload,
                        uint8_t len, uint8_t expected_response_length, uint16_t *trans_id)
{

    // When an application response is expected, forward the payload directly to the current D7A session
    // TODO how to filter by client Id since we don't know to which client the request is addressed?
    if (d7ap_stack_state == D7AP_STACK_STATE_WAIT_APP_ANSWER)
    {
        DPRINT("[D7AP] sending response");
        DPRINT_DATA(payload, len);
        return (d7asp_send_response(payload, len));
    }

    // Create or return the master session if the current one is compatible with the given session configuration.
    uint8_t session_token = d7asp_master_session_create(config);

    if(session_token == 0)
        return ERETRY;

    // Check if a session already exists
    session_t* session = get_session_by_session_token(session_token);

    if (session == NULL) {
        session = alloc_session(client_id);

        if (session == NULL)
            return -ESIZE;

        session->token = session_token;
    }

    if (session->request_nb == MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT)
        return -ESIZE;

    //TODO handle here the fragmentation if needed?

    uint8_t request_id = d7asp_queue_request(session->token,
                                             payload, len,
                                             expected_response_length);

    session->trans_id[session->request_nb] = ((uint16_t)session->token << 8) | (request_id & 0x00FF);

    if (trans_id != NULL) {
        *trans_id = session->trans_id[session->request_nb];
        DPRINT("[D7AP] request posted with trans_id %02X and request_nb %d", *trans_id, session->request_nb);
    } else {
        DPRINT("[D7AP] request posted with request_nb %d", session->request_nb);
    }
    
    DPRINT_DATA(payload, len);

    session->request_nb++;
    return SUCCESS;
}

bool d7ap_stack_process_unsolicited_request(uint8_t *payload, uint8_t length, d7ap_session_result_t result, bool response_expected)
{
    bool expect_upper_layer_resp_payload = false;

    //TODO handle here the re-assembly if needed?
    DPRINT("[D7AP] received an unsolicited request");
    DPRINT_DATA(payload, length);

    slave_session.active = true;
    slave_session.token = result.fifo_token;
    slave_session.expected_response = response_expected;

    // Forward this unsolicited request to all clients
    for(uint8_t i = 0; i < registered_client_nb; i++)
    {
        if (registered_client[i].unsolicited_cb)
            expect_upper_layer_resp_payload = registered_client[i].unsolicited_cb(payload, length, result, response_expected);
    }

    if ((slave_session.expected_response) && (expect_upper_layer_resp_payload))
        switch_state(D7AP_STACK_STATE_WAIT_APP_ANSWER);
    else
        switch_state(D7AP_STACK_STATE_RECEIVING);

    return expect_upper_layer_resp_payload;
}

void d7ap_stack_process_received_response(uint8_t *payload, uint8_t length, d7ap_session_result_t result)
{
    DPRINT("[D7AP] received a response");
    DPRINT_DATA(payload, length);
    session_t* session = get_session_by_session_token(result.fifo_token);

    assert(session != NULL);

    uint16_t trans_id = ((uint16_t)result.fifo_token << 8) | (result.seqnr & 0x00FF);
    uint8_t i = 0;

    DPRINT("[D7AP] received trans_id %02X", trans_id);
    DPRINT("[D7AP] session->request_nb %d", session->request_nb);

    for(i = 0; i < session->request_nb; i++)
    {
    	DPRINT("[D7AP] session->trans_id[i] = %02X", session->trans_id[i]);
    	if (session->trans_id[i] == trans_id)
            break;
    }

    assert(i < session->request_nb);

    if (registered_client[session->client_id].receive_cb)
        registered_client[session->client_id].receive_cb(trans_id, payload, length, result);
}

void d7ap_stack_session_completed(uint8_t session_token, uint8_t* progress_bitmap, uint8_t* success_bitmap, uint8_t bitmap_byte_count)
{
    DPRINT("[D7AP] session is completed");
    error_t error;
    uint8_t request_id;
    session_t* session = get_session_by_session_token(session_token);

    assert(session != NULL);

    if (registered_client[session->client_id].transmitted_cb == NULL)
        goto free_session;

    for(uint8_t i = 0; i < session->request_nb; i++)
    {
        request_id = (uint8_t)(session->trans_id[i] & 0xFF);
        error = bitmap_get(progress_bitmap, request_id) && bitmap_get(success_bitmap, request_id) ? SUCCESS : FAIL;

        registered_client[session->client_id].transmitted_cb(session->trans_id[i], error);
    }

    switch_state(D7AP_STACK_STATE_IDLE);

free_session:
    free_session(session);
}

void d7ap_stack_signal_active_master_session(uint8_t session_token)
{
    switch_state(D7AP_STACK_STATE_TRANSMITTING);
    DPRINT("[D7AP] session[%d] is now active", session_token);
    session_t* session = get_session_by_session_token(session_token);

    assert(session != NULL);
    session->active = true;
}


void d7ap_stack_signal_slave_session_terminated(void)
{
    DPRINT("[D7AP] slave session is terminated");
    slave_session.active = false;
    switch_state(D7AP_STACK_STATE_IDLE);
}

void d7ap_stack_signal_transaction_terminated(void)
{
    DPRINT("[D7AP] transaction is terminated");

    if ( d7ap_stack_state ==  D7AP_STACK_STATE_WAIT_APP_ANSWER)
        switch_state(D7AP_STACK_STATE_RECEIVING);
}

bool d7ap_stack_is_client_session_active(uint8_t client_id)
{
    for(uint8_t i = 0; i < MODULE_D7AP_MAX_SESSION_COUNT; i++) {
        if(sessions[i].client_id == client_id && sessions[i].active)
            return true;
    }

    return false;
}

