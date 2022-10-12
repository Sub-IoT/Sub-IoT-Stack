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
 *  \author glenn.ergeerts@uantwerpen.be
 */

#include <string.h>

#include "debug.h"
#include "ng.h"
#include "log.h"
#include "bitmap.h"
#include "d7ap_fs.h"
#include "random.h"
#include "errors.h"
#include "compress.h"

#include "hwdebug.h"
#include "hwwatchdog.h"

#include "MODULE_D7AP_defs.h"
#include "d7ap_stack.h"
#include "d7ap.h"
#include "d7asp.h"
#include "d7atp.h"
#include "packet_queue.h"
#include "packet.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_SP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_SESSION, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif

typedef enum {
  D7ASP_MASTER_SESSION_IDLE,
  D7ASP_MASTER_SESSION_DORMANT,
  D7ASP_MASTER_SESSION_PENDING,
  D7ASP_MASTER_SESSION_PENDING_DORMANT_TIMEOUT,
  D7ASP_MASTER_SESSION_PENDING_DORMANT_TRIGGERED,
  D7ASP_MASTER_SESSION_ACTIVE,
} d7asp_master_session_state_t;

struct d7asp_master_session {
    d7ap_session_config_t config;
    d7asp_master_session_state_t state;
    uint8_t token;
    uint8_t progress_bitmap[REQUESTS_BITMAP_BYTE_COUNT];
    uint8_t success_bitmap[REQUESTS_BITMAP_BYTE_COUNT];
    uint8_t next_request_id;
    uint8_t request_buffer_tail_idx;
    uint8_t requests_indices[MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT]; /**< Contains for every request ID the index in command_buffer the index where the request begins */
    uint8_t requests_lengths[MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT]; /**< Contains for every request ID the index in command_buffer the length of the ALP payload in that request */
    uint8_t response_lengths[MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT]; /**< Contains for every request ID the index in command_buffer the expected length of the ALP response for the specific request */
    uint8_t request_buffer[MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE];
    d7ap_addressee_t preferred_addressee;
};

static d7asp_master_session_t NGDEF(_current_master_session); // TODO we only use 1 fifo for now, should be multiple later (1 per on unique addressee and QoS combination)
#define current_master_session NG(_current_master_session)

static uint8_t NGDEF(_current_request_id); // TODO move ?
#define current_request_id NG(_current_request_id)

static uint8_t NGDEF(_current_request_retry_count);
#define current_request_retry_count NG(_current_request_retry_count)

static packet_t* NGDEF(_current_request_packet);
#define current_request_packet NG(_current_request_packet)

static uint8_t NGDEF(_single_request_retry_limit);
#define single_request_retry_limit NG(_single_request_retry_limit)

static packet_t* NGDEF(_current_response_packet);
#define current_response_packet NG(_current_response_packet)

static timer_event current_session_timer;
static timer_event dormant_session_timer;

typedef enum {
    D7ASP_STATE_STOPPED,
    D7ASP_STATE_IDLE,
    D7ASP_STATE_SLAVE,
    D7ASP_STATE_MASTER,
    D7ASP_STATE_SLAVE_PENDING_MASTER,
    D7ASP_STATE_PENDING_MASTER,
    D7ASP_STATE_SLAVE_WAITING_RESPONSE
} state_t;

typedef struct {
  uint8_t lb;
  uint8_t id[8];
} lowest_lb_responder_t;

static lowest_lb_responder_t current_responder_lowest_lb;

#define LB_MAX 140

static state_t NGDEF(_state) = D7ASP_STATE_STOPPED;
#define d7asp_state NG(_state)

static void switch_state(state_t new_state);

static void mark_current_request_done()
{
    bitmap_set(current_master_session.progress_bitmap, current_request_id);
    // current_request_packet will be free-ed in the packet_queue when the transaction is completed
}

static void mark_current_request_successful()
{
    bitmap_set(current_master_session.success_bitmap, current_request_id);
}

static void init_master_session(d7asp_master_session_t* session) {
    session->state = D7ASP_MASTER_SESSION_IDLE;
    do {
        session->token = get_rnd() % 0xFF;
    } while(session->token == 0);
    memset(session->progress_bitmap, 0x00, REQUESTS_BITMAP_BYTE_COUNT);
    memset(session->success_bitmap, 0x00, REQUESTS_BITMAP_BYTE_COUNT);
    session->next_request_id = 0;
    session->request_buffer_tail_idx = 0;
    memset(session->requests_indices, 0x00, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
    memset(session->requests_lengths, 0x00, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
    memset(session->response_lengths, 255, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
    memset(session->request_buffer, 0x00, MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE);

    // TODO we don't reset preferred_addressee field for now
    // for now one ALP command execution mostly results one new session, which
    // would break the preferred addressee mechanism. For now this is cached regardless
    // over the session, until we decide on session lifetime etc
}

static void flush_completed() {
    DPRINT("FIFO flush completed");

    // TODO When a Session does not terminate on success, the Session is automatically re-activated using
    // the RETRY_MODE pattern defined in the Configuration file

    // single flush of the FIFO without retry
    d7ap_stack_session_completed(current_master_session.token, current_master_session.progress_bitmap,
                                   current_master_session.success_bitmap, current_master_session.next_request_id - 1);
    init_master_session(&current_master_session);
    current_master_session.state = D7ASP_MASTER_SESSION_IDLE;
    d7atp_signal_dialog_termination();
    switch_state(D7ASP_STATE_IDLE);
}

static void schedule_current_session() {
    assert(d7asp_state == D7ASP_STATE_MASTER || d7asp_state == D7ASP_STATE_PENDING_MASTER || d7asp_state == D7ASP_STATE_SLAVE);
    assert(current_master_session.state >= D7ASP_MASTER_SESSION_PENDING);

    DPRINT("Re-schedule immediately the current session");
    current_session_timer.next_event = 0;
    int rtc = timer_add_event(&current_session_timer);
    assert(rtc == SUCCESS);
}

static void flush_fifos()
{
    error_t ret;

    if (d7asp_state != D7ASP_STATE_MASTER && d7asp_state != D7ASP_STATE_PENDING_MASTER)
    {
        DPRINT("Flushing FIFOs can't be executed in this state <%d>", d7asp_state);
        return;
    }

    if(current_master_session.state != D7ASP_MASTER_SESSION_PENDING &&
       current_master_session.state != D7ASP_MASTER_SESSION_ACTIVE &&
       current_master_session.state != D7ASP_MASTER_SESSION_PENDING_DORMANT_TIMEOUT &&
       current_master_session.state != D7ASP_MASTER_SESSION_PENDING_DORMANT_TRIGGERED) {
      DPRINT("No sessions in pending or active state, skipping");
      return;
    }

    bool is_triggered_dormant_session = (current_master_session.state == D7ASP_MASTER_SESSION_PENDING_DORMANT_TRIGGERED);
    current_master_session.state = D7ASP_MASTER_SESSION_ACTIVE;
    if (d7asp_state == D7ASP_STATE_PENDING_MASTER)
    {
        switch_state(D7ASP_STATE_MASTER);
        d7ap_stack_signal_active_master_session(current_master_session.token);
    }

    current_responder_lowest_lb.lb = LB_MAX;
    DPRINT("Flushing FIFOs");
    hw_watchdog_feed(); // TODO do here?

    if (current_request_id == NO_ACTIVE_REQUEST_ID)
    {
        // find first request which is not acked or dropped
        int8_t found_next_req_index = bitmap_search(current_master_session.progress_bitmap, false, MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);
        if (found_next_req_index == -1 || found_next_req_index == current_master_session.next_request_id)
        {
            // we handled all requests ...
            flush_completed();
            return;
        }

        current_request_id = found_next_req_index;
        DPRINT("Found request Id %x", current_request_id);
        current_request_retry_count = 0;

        current_request_packet = packet_queue_alloc_packet();
        assert(current_request_packet);
        packet_queue_mark_processing(current_request_packet);
        current_request_packet->d7anp_addressee = &(current_master_session.config.addressee); // TODO explicitly pass addressee down the stack layers?

        if(current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_PREFERRED
           && memcmp(current_master_session.preferred_addressee.id,(uint8_t[8]){ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 8) != 0)
        {
            DPRINT("overriding addressee with preferred one");
            current_master_session.preferred_addressee.access_class = current_master_session.config.addressee.access_class;
            current_master_session.preferred_addressee.ctrl.nls_method = current_master_session.config.addressee.ctrl.nls_method;
            current_master_session.config.addressee.ctrl.id_type = ID_TYPE_UID; // TODO no VID for now
            current_request_packet->d7anp_addressee = &current_master_session.preferred_addressee;
        }

        memcpy(current_request_packet->payload, current_master_session.request_buffer + current_master_session.requests_indices[current_request_id], current_master_session.requests_lengths[current_request_id]);
        current_request_packet->payload_length = current_master_session.requests_lengths[current_request_id];

        if(is_triggered_dormant_session)
        {
            current_request_packet->type = REQUEST_IN_DIALOG_EXTENSION;
        }
        else
        {
            if (current_request_id == 0)
                current_request_packet->type = INITIAL_REQUEST;
            else
                current_request_packet->type =  SUBSEQUENT_REQUEST;
        }

        // TODO calculate Tl
        // Tl should correspond to the maximum time needed to send the remaining requests in the FIFO including the RETRY parameter
    }
    else
    {
        // retrying request ...
        DPRINT("Current request retry count: %i", current_request_retry_count);
        if (current_request_retry_count == single_request_retry_limit)
        {
            // mark request as failed and pop
            mark_current_request_done();
            DPRINT("Request reached single request retry limit (%i), skipping request", single_request_retry_limit);
            packet_queue_free_packet(current_request_packet);
            current_request_id = NO_ACTIVE_REQUEST_ID;
            schedule_current_session(); //reschedule the d7ap stack to continue flushing the session until all request handled ...
            return;
        }

        packet_queue_mark_processing(current_request_packet);
        current_request_packet->type = RETRY_REQUEST;
        // TODO stop on error
    }

    uint8_t listen_timeout = 0; // TODO calculate timeout (and update during transaction lifetime) (based on Tc, channel, cs, payload size, # msgs, # retries)
    ret = d7atp_send_request(current_master_session.token, current_request_id, (current_request_id == current_master_session.next_request_id - 1),
                       current_request_packet, &current_master_session.config.qos, listen_timeout, current_master_session.response_lengths[current_request_id]);
    if (ret == EPERM)
    {
        // this is probably because no further encryption is possible (frame counter reaches the maximum value)
        // TODO return an error code to the application?
        DPRINT("Request sending is not allowed likely because frame counter reaches its maximum value");
    }
}

// TODO document state diagram
static void switch_state(state_t new_state)
{
    switch(new_state)
    {
        case D7ASP_STATE_MASTER:
            switch(d7asp_state)
            {
                case D7ASP_STATE_IDLE:
                    d7asp_state = new_state;
                    schedule_current_session();
                    DPRINT("Switching to state D7ASP_STATE_MASTER");
                    break;
                case D7ASP_STATE_SLAVE_PENDING_MASTER:
                case D7ASP_STATE_PENDING_MASTER:
                    d7asp_state = new_state;
                    DPRINT("Switching to state D7ASP_STATE_MASTER");
                    break;
                case D7ASP_STATE_SLAVE:
                    d7asp_state = D7ASP_STATE_SLAVE_PENDING_MASTER;
                    DPRINT("Switching to state D7ASP_STATE_SLAVE_PENDING_MASTER");
                    break;
                default:
                    assert(false);
            }

            break;
        case D7ASP_STATE_SLAVE:
            switch(d7asp_state)
            {
                case D7ASP_STATE_IDLE:
                case D7ASP_STATE_MASTER:
                case D7ASP_STATE_SLAVE_WAITING_RESPONSE:
                    d7asp_state = new_state;
                    current_request_id = NO_ACTIVE_REQUEST_ID;
                    DPRINT("Switching to state D7ASP_STATE_SLAVE");
                    break;
                default:
                    assert(false);
            }
            break;
        case D7ASP_STATE_SLAVE_PENDING_MASTER:
            assert(d7asp_state == D7ASP_STATE_SLAVE || d7asp_state == D7ASP_STATE_PENDING_MASTER ||
                   d7asp_state == D7ASP_STATE_SLAVE_WAITING_RESPONSE);
            d7asp_state = D7ASP_STATE_SLAVE_PENDING_MASTER;
            DPRINT("Switching to state D7ASP_STATE_SLAVE_PENDING_MASTER");
            break;
        case D7ASP_STATE_PENDING_MASTER:
            assert(d7asp_state == D7ASP_STATE_IDLE || d7asp_state == D7ASP_STATE_SLAVE);
            d7asp_state = D7ASP_STATE_PENDING_MASTER;
            DPRINT("Switching to state D7ASP_STATE_PENDING_MASTER");
            break;
        case D7ASP_STATE_SLAVE_WAITING_RESPONSE:
            assert(d7asp_state == D7ASP_STATE_SLAVE || d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER);
            d7asp_state = D7ASP_STATE_SLAVE_WAITING_RESPONSE;
            DPRINT("Switching to state D7ASP_STATE_SLAVE_WAITING_RESPONSE");
            break;
        case D7ASP_STATE_IDLE:
            d7asp_state = new_state;
            current_request_id = NO_ACTIVE_REQUEST_ID;
            DPRINT("Switching to state D7ASP_STATE_IDLE");
            break;
        default:
            assert(false);
    }
}

static void dormant_session_timeout() {
  // TODO pass session so we can have multiple
  DPRINT("dormant session timeout");
  if(current_master_session.state == D7ASP_MASTER_SESSION_DORMANT) {
    current_master_session.state = D7ASP_MASTER_SESSION_PENDING_DORMANT_TIMEOUT;
    if(d7asp_state == D7ASP_STATE_IDLE)
      d7asp_state = D7ASP_STATE_PENDING_MASTER;

    schedule_current_session();
  }
}

static void schedule_dormant_session(d7asp_master_session_t* dormant_session) {
  assert(dormant_session->state == D7ASP_MASTER_SESSION_DORMANT);
  timer_tick_t timeout = CT_DECOMPRESS(dormant_session->config.dormant_timeout);
  DPRINT("Sched dormant timeout in %i s", timeout);
  dormant_session_timer.next_event = timeout * 1024;
  error_t rtc = timer_add_event(&dormant_session_timer);
  assert(rtc == SUCCESS);
}

void d7asp_init()
{
    assert(d7asp_state == D7ASP_STATE_STOPPED);

    d7asp_state = D7ASP_STATE_IDLE;
    current_request_id = NO_ACTIVE_REQUEST_ID;

    current_master_session.state = D7ASP_MASTER_SESSION_IDLE;
    memcpy(current_responder_lowest_lb.id, (uint8_t[8]){ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 8);
    memcpy(current_master_session.preferred_addressee.id, (uint8_t[8]){ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 8);
    DPRINT("REQUESTS_BITMAP_BYTE_COUNT %d", REQUESTS_BITMAP_BYTE_COUNT);
    DPRINT("FIFO_MAX_REQUESTS_COUNT %d", MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT);

    timer_init_event(&dormant_session_timer, &dormant_session_timeout);
    timer_init_event(&current_session_timer, &flush_fifos);
}

void d7asp_stop()
{
    d7asp_state = D7ASP_STATE_STOPPED;
    timer_cancel_event(&current_session_timer);
    timer_cancel_event(&dormant_session_timer);
}

uint8_t d7asp_master_session_create(d7ap_session_config_t* d7asp_master_session_config) {
    // TODO for now we assume only one concurrent session, in the future we should dynamically allocate (or return from pool) a session

    if (current_master_session.state != D7ASP_MASTER_SESSION_IDLE)
    {
        // Requests can be pushed in the FIFO by upper layer anytime
        if ((current_master_session.config.addressee.access_class == d7asp_master_session_config->addressee.access_class) &&
                (current_master_session.config.addressee.ctrl.nls_method == d7asp_master_session_config->addressee.ctrl.nls_method) &&
                ((d7asp_master_session_config->qos.qos_resp_mode == SESSION_RESP_MODE_PREFERRED && current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_PREFERRED) ||
                (current_master_session.config.addressee.ctrl.id_type == d7asp_master_session_config->addressee.ctrl.id_type && 
                memcmp(current_master_session.config.addressee.id, d7asp_master_session_config->addressee.id, d7ap_addressee_id_length(d7asp_master_session_config->addressee.ctrl.id_type)) == 0)))
            return current_master_session.token;
        else
            return 0;
        // TODO create a pending session or a dormant session if TO (DORM_TIMER) !=0
    }

    DPRINT("current master session state %d", current_master_session.state);


    init_master_session(&current_master_session);

    DPRINT("Create master session %d", current_master_session.token);

    current_master_session.config.qos = d7asp_master_session_config->qos;
    current_master_session.config.dormant_timeout = d7asp_master_session_config->dormant_timeout;
    current_master_session.config.addressee.ctrl = d7asp_master_session_config->addressee.ctrl;
    current_master_session.config.addressee.access_class = d7asp_master_session_config->addressee.access_class;

    if(current_master_session.config.qos.qos_resp_mode != SESSION_RESP_MODE_PREFERRED) {
      memcpy(current_master_session.config.addressee.id, d7asp_master_session_config->addressee.id, sizeof(current_master_session.config.addressee.id));
    } else {
      // in this case we don't reset the preferred addressee.
      // for now one ALP command execution mostly results one new session, which
      // would break the preferred addressee mechanism. For now this is cached regardless
      // over the session, until we decide on session lifetime etc.
      current_master_session.config.addressee.id[0] = d7asp_master_session_config->addressee.id[0];
      assert(d7asp_master_session_config->addressee.ctrl.id_type == ID_TYPE_NBID
             || d7asp_master_session_config->addressee.ctrl.id_type == ID_TYPE_NOID);
    }

    if(current_master_session.config.dormant_timeout) {
      current_master_session.state = D7ASP_MASTER_SESSION_DORMANT;
      schedule_dormant_session(&current_master_session);
    }

    return current_master_session.token;
}

static d7asp_master_session_t* get_master_session_from_token(uint8_t session_token)
{
    //TODO handle a list of sessions
    //for now, return systematically the current master session
    assert(current_master_session.token == session_token);
    return(&current_master_session);
}

error_t d7asp_send_response(uint8_t* payload, uint8_t length)
{
    DPRINT("Send the expected response");
    DPRINT_DATA(payload, length);

    if (d7asp_state != D7ASP_STATE_SLAVE_WAITING_RESPONSE)
    {
        // Response comes too late, the response period is probably expired
        DPRINT("Not waiting for a response, discard it");
        return EINVAL;
    }

    if(length > MODULE_D7AP_PAYLOAD_SIZE)
    {
        log_print_error_string("%s:%s Payload too large, %d > %d", __FILE__, __FUNCTION__, length, MODULE_D7AP_PAYLOAD_SIZE);
        return EFBIG;
    }

    current_response_packet->payload_length = length;
    memcpy(current_response_packet->payload, payload, length);

    // check if there is a pending session
    if (current_master_session.state == D7ASP_MASTER_SESSION_ACTIVE)
        switch_state(D7ASP_STATE_SLAVE_PENDING_MASTER);
    else
        switch_state(D7ASP_STATE_SLAVE);

    /*
     * activate the dialog extension procedure in the unicast response if the dialog is terminated
     * and a master session is pending and this session is
     */
    if ((!ID_TYPE_IS_BROADCAST(current_response_packet->dll_header.control_target_id_type))
            && (d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER))
    {
        current_response_packet->d7atp_ctrl.ctrl_is_start = true;
        // TODO set packet->d7anp_listen_timeout according the time remaining in the current transaction
        // + the maximum time to send the first request of the pending session.
    }
    else
        current_response_packet->d7atp_ctrl.ctrl_is_start = 0;

    return(d7atp_send_response(current_response_packet));
}

uint8_t d7asp_queue_request(uint8_t session_token, uint8_t* alp_payload_buffer, uint8_t alp_payload_length, uint8_t expected_alp_response_length)
{
    DPRINT("Queuing request in the session queue");
    d7asp_master_session_t *session = get_master_session_from_token(session_token);

    // TODO can be called in all session states?
    assert(session != NULL);
    assert(session->request_buffer_tail_idx + alp_payload_length < MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE);
    assert(session->next_request_id < MODULE_D7AP_FIFO_MAX_REQUESTS_COUNT); // TODO do not assert but let upper layer handle this
    assert(!(expected_alp_response_length > 0 &&
             (session->config.qos.qos_resp_mode == SESSION_RESP_MODE_NO || session->config.qos.qos_resp_mode == SESSION_RESP_MODE_NO_RPT))); // TODO return error
    single_request_retry_limit = 1; // TODO read from SEL config file

    // add request to buffer
    // TODO request can contain 1 or more ALP commands, find a way to group commands in requests instead of dumping all requests in one buffer
    uint8_t request_id = session->next_request_id;
    session->requests_indices[request_id] = session->request_buffer_tail_idx;
    session->requests_lengths[request_id] = alp_payload_length;
    session->response_lengths[request_id] = expected_alp_response_length;
    memcpy(session->request_buffer + session->request_buffer_tail_idx, alp_payload_buffer, alp_payload_length);
    session->request_buffer_tail_idx += alp_payload_length + 1;
    session->next_request_id++;

    if(current_master_session.state == D7ASP_MASTER_SESSION_IDLE) {
      current_master_session.state = D7ASP_MASTER_SESSION_PENDING;
      DPRINT("converting IDLE session to PENDING");
    } else if(current_master_session.state == D7ASP_MASTER_SESSION_DORMANT) {
      DPRINT("session is dormant, not activating");
    }

    // TODO for master only set to pending when asked by upper layer (ie new function call)
    if ((d7asp_state == D7ASP_STATE_IDLE) && (current_master_session.state != D7ASP_MASTER_SESSION_DORMANT))
    {
        switch_state(D7ASP_STATE_PENDING_MASTER);
        schedule_current_session();
    }
    else if (d7asp_state == D7ASP_STATE_SLAVE)
        switch_state(D7ASP_STATE_SLAVE_PENDING_MASTER);

    return request_id;
}

void d7asp_process_received_response(packet_t* packet, bool extension)
{
    hw_watchdog_feed(); // TODO do here?
    d7ap_session_result_t result = {
        .channel = {
            .channel_header = packet->phy_config.rx.channel_id.channel_header_raw,
            .center_freq_index = packet->phy_config.rx.channel_id.center_freq_index,
        },
        .rx_level =  - packet->hw_radio_packet.rx_meta.rssi,
        .link_budget = (packet->dll_header.control_eirp_index - 32) - packet->hw_radio_packet.rx_meta.rssi,
        .target_rx_level = 80, // TODO not implemented yet, use default for now
        .status = {
            .ucast = 0, // TODO
            .nls = (packet->d7anp_ctrl.nls_method ? true : false),
            .retry = false, // TODO
            .missed = false, // TODO
        },
        .response_to = packet->d7atp_tc,
        .addressee = *packet->d7anp_addressee
        // .fifo_token and .seqnr filled below
    };

    assert(d7asp_state == D7ASP_STATE_MASTER);
    assert(packet->d7atp_dialog_id == current_master_session.token);
    assert(packet->d7atp_transaction_id == current_request_id);

    // received ack
    DPRINT("Received ACK for request ID %d", current_request_id);
    if (current_master_session.config.qos.qos_resp_mode != SESSION_RESP_MODE_NO
       && current_master_session.config.qos.qos_resp_mode != SESSION_RESP_MODE_NO_RPT)
    {
        // for SESSION_RESP_MODE_NO and SESSION_RESP_MODE_NO_RPT the request was already marked as done
        // upon successfull CSMA insertion. We don't care about response in these cases.

        if((current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_PREFERRED) 
            && (current_master_session.config.addressee.ctrl.id_type == ID_TYPE_UID) 
            && (packet->d7atp_ctrl.ctrl_xoff)) {
            DPRINT("preferred gateway answered that it should not be preferred, this should not count as an ACK");
            memcpy(current_master_session.preferred_addressee.id,
                (uint8_t[8]) { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 8);
            memcpy(current_responder_lowest_lb.id, (uint8_t[8]) { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 8);
            packet_queue_free_packet(packet);
            return;
        }

        result.fifo_token = current_master_session.token;
        result.seqnr = current_request_id;
        mark_current_request_successful();
        mark_current_request_done();
        if(current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_PREFERRED
           && ID_TYPE_IS_BROADCAST(current_master_session.config.addressee.ctrl.id_type))
        {
            if(result.link_budget < current_responder_lowest_lb.lb && (!packet->d7atp_ctrl.ctrl_xoff))
            {
                memcpy(current_responder_lowest_lb.id, result.addressee.id, 8); // TODO assume UID for now
                current_responder_lowest_lb.lb = result.link_budget;
                DPRINT("current responder with lowest LB %i:", current_responder_lowest_lb.lb);
                DPRINT_DATA(current_responder_lowest_lb.id, 8);
            }
        }
        assert(packet != current_request_packet);
    }

    d7ap_stack_process_received_response(packet->payload, packet->payload_length, result);

    packet_queue_free_packet(packet); // ACK can be cleaned

    /* In case of unicast session, it is acceptable to switch to the next request before the expiration of Tc */
    if (!ID_TYPE_IS_BROADCAST(current_master_session.config.addressee.ctrl.id_type))
    {
        DPRINT("Request completed, don't wait end of transaction");
        packet_queue_free_packet(current_request_packet);

        // terminate the dialog if all request handled
        // we need to switch to the state idle otherwise we may receive a new packet before the task flush_fifos is handled
        // in this case, we may assert since the state remains MASTER
        if (current_request_id == current_master_session.next_request_id - 1)
        {
            flush_completed();
            return;
        }
        current_request_id = NO_ACTIVE_REQUEST_ID;
        schedule_current_session(); // continue flushing until all request handled ...
        // stop the current transaction
        // d7atp_stop_transaction(); //TO BE CHECKED THAT COMMENTING THIS OUT HAS NO NEGATIVE EFFECT
    }
    // switch to the state slave when the D7ATP Dialog Extension Procedure is initiated and all request are handled
    else if ((extension) && (current_request_id == current_master_session.next_request_id - 1))
    {
        DPRINT("Dialog Extension Procedure is initiated, mark the FIFO flush "
               "completed before switching to a responder state");
        d7ap_stack_session_completed(current_master_session.token, current_master_session.progress_bitmap,
                                     current_master_session.success_bitmap, current_master_session.next_request_id - 1);
        current_master_session.state = D7ASP_MASTER_SESSION_IDLE;
        switch_state(D7ASP_STATE_SLAVE);
    }
}

bool d7asp_process_received_packet(packet_t* packet)
{
    bool expect_upper_layer_resp_payload = false;

    assert(d7asp_state == D7ASP_STATE_IDLE ||
           d7asp_state == D7ASP_STATE_SLAVE ||
           d7asp_state == D7ASP_STATE_PENDING_MASTER ||
           d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER);

    // received a request, start slave session, process and respond
    if (d7asp_state == D7ASP_STATE_IDLE)
        switch_state(D7ASP_STATE_SLAVE); // don't switch when already in slave state
    else if (d7asp_state == D7ASP_STATE_PENDING_MASTER)
        switch_state(D7ASP_STATE_SLAVE_PENDING_MASTER);

    if (packet->payload_length > 0)
    {
        d7ap_session_result_t result = {
            .channel = {
                .channel_header = packet->phy_config.rx.channel_id.channel_header_raw,
                .center_freq_index = packet->phy_config.rx.channel_id.center_freq_index,
            },
            .rx_level =  - packet->hw_radio_packet.rx_meta.rssi,
            .link_budget = (packet->dll_header.control_eirp_index - 32) - packet->hw_radio_packet.rx_meta.rssi,
            .target_rx_level = 80, // TODO not implemented yet, use default for now
            .status = {
                .ucast = 0, // TODO
                .nls = (packet->d7anp_ctrl.nls_method ? true : false),
                .retry = false, // TODO
                .missed = false, // TODO
            },
            .response_to = packet->d7atp_tc,
            .addressee = *packet->d7anp_addressee,
            .fifo_token =  packet->d7atp_dialog_id,
            .seqnr = packet->d7atp_transaction_id
        };

        expect_upper_layer_resp_payload = d7ap_stack_process_unsolicited_request(packet->payload, packet->payload_length, result, packet->d7atp_ctrl.ctrl_is_ack_requested);
    }

    if (current_master_session.state == D7ASP_MASTER_SESSION_DORMANT &&
        (!ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type)) &&
        memcmp(current_master_session.config.addressee.id, packet->d7anp_addressee->id, d7ap_addressee_id_length(packet->d7anp_addressee->ctrl.id_type)) == 0) {
        DPRINT("pending dormant session for requester");
        current_master_session.state = D7ASP_MASTER_SESSION_PENDING_DORMANT_TRIGGERED;
    }

    /*
     * activate the dialog extension procedure in the unicast response if the dialog is terminated
     * and a master session is pending
     */
    if ((!ID_TYPE_IS_BROADCAST(packet->dll_header.control_target_id_type)) &&
        (d7asp_state == D7ASP_STATE_SLAVE) && (current_master_session.state == D7ASP_MASTER_SESSION_PENDING_DORMANT_TRIGGERED))
    {
        packet->d7atp_ctrl.ctrl_is_start = true;
        packet->d7atp_ctrl.ctrl_tl = true;
        // calculate Tl
        // TODO payload length does not include headers ... + hardcoded subband
        // TODO this length does not include lower layers overhead for now, assume 20 bytes for now ...
        uint16_t len = packet->payload_length;
        if(len < 255 - 20)
            len += 20;

        // TX duration for ack
        uint16_t estimated_tl = phy_calculate_tx_duration(packet->phy_config.rx.channel_id.channel_header.ch_class,
                                                          packet->phy_config.rx.channel_id.channel_header.ch_coding,
                                                          len, false);

        estimated_tl += t_g; // Tt < silent time < Tg ~ in practice 4.26 ms
        // TX duration for dormant session
        estimated_tl += phy_calculate_tx_duration(packet->phy_config.rx.channel_id.channel_header.ch_class,
                                                  packet->phy_config.rx.channel_id.channel_header.ch_coding,
                                                  current_master_session.requests_lengths[0], false); // TODO assuming 1 queued request for now
        DPRINT("Dormant session estimated Tl=%i", estimated_tl);
        packet->d7atp_tl = compress_data(estimated_tl, true);
    }
    else
        packet->d7atp_ctrl.ctrl_is_start = 0;

    // execute slave transaction
    if (packet->d7atp_ctrl.ctrl_is_ack_requested)
    {
        // a response is required, either with payload or just an ack without payload
        current_response_packet = packet;
        if(expect_upper_layer_resp_payload == false) {
          DPRINT("Don't need resp payload from upper layer, send the ACK");
          return false; // don't free the packet here, it will be done in d7asp_signal_packet_transmitted()
        } else {
          DPRINT("Wait for resp from upper layer");
          switch_state(D7ASP_STATE_SLAVE_WAITING_RESPONSE);
          return true;
        }
    }
    else
    {
        // no ack to be transmitted, free packet
        packet_queue_free_packet(packet);
        return false;
    }
}

static void on_request_completed()
{
    assert(d7asp_state == D7ASP_STATE_MASTER);
    DPRINT("request completed");

    if(current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_PREFERRED) {
      memcpy(current_master_session.preferred_addressee.id, current_responder_lowest_lb.id, 8); // TODO assume UID for now
      current_master_session.preferred_addressee.ctrl.id_type = ID_TYPE_UID;

      DPRINT("preferred addressee with LB %i is now:", current_responder_lowest_lb.lb);
      DPRINT_DATA(current_master_session.preferred_addressee.id, 8);
    }

    if (!bitmap_get(current_master_session.progress_bitmap, current_request_id))
    {
        if(current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_PREFERRED
          && memcmp(current_master_session.preferred_addressee.id, (uint8_t[8]){ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 8) != 0)
        {
            DPRINT("No ack from preferred addressee, switching to bcast");
            current_responder_lowest_lb.lb = LB_MAX;
            memcpy(current_master_session.preferred_addressee.id, (uint8_t[8]){ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 8);
        }
        current_request_retry_count++;
        // the request may be retransmitted, don't free yet (this will be done in flush_fifo() when failed)
    }
    else
    {
        // request completed, no retries needed so we can free the packet
        packet_queue_free_packet(current_request_packet);

        // terminate the dialog if all request handled
        // we need to switch to the state idle otherwise we may receive a new packet before the task flush_fifos is handled
        // in this case, we may assert since the state remains MASTER
        if (current_request_id == current_master_session.next_request_id - 1)
        {
            flush_completed();
            return;
        }
        current_request_id = NO_ACTIVE_REQUEST_ID;
    }

    schedule_current_session(); // reschedule the d7ap stack to continue flushing the session until all request handled
}

void d7asp_signal_packet_transmitted(packet_t *packet)
{
    DPRINT("Packet transmitted");
    if (d7asp_state == D7ASP_STATE_MASTER)
    {
        // for the lowest QoS level the packet is ack-ed when CSMA/CA process succeeded
        if (current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_NO ||
           current_master_session.config.qos.qos_resp_mode == SESSION_RESP_MODE_NO_RPT)
        {
            mark_current_request_done();
            mark_current_request_successful();
        }
    }
    else if (d7asp_state == D7ASP_STATE_SLAVE || d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER)
    {
        assert(current_response_packet == packet);

        // when in slave session we can immediately cleanup the transmitted response.
        // requests (in master sessions) will be cleanup upon termination of the dialog.
        current_response_packet = NULL;
        packet_queue_free_packet(packet);
    }
}

void d7asp_signal_transmission_failure()
{
    if (d7asp_state == D7ASP_STATE_MASTER)
        on_request_completed();
    else if (d7asp_state == D7ASP_STATE_SLAVE ||
             d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER)
    {
        packet_queue_free_packet(current_response_packet);
        current_response_packet = NULL;
    }
}

void d7asp_signal_transaction_terminated()
{
    if (d7asp_state == D7ASP_STATE_SLAVE_WAITING_RESPONSE)
    {
        // the time window to respond is expired, so it is not possible to send a response anymore
        if (current_master_session.state == D7ASP_MASTER_SESSION_ACTIVE)
            switch_state(D7ASP_STATE_SLAVE_PENDING_MASTER);
        else
            switch_state(D7ASP_STATE_SLAVE);

        d7ap_stack_signal_transaction_terminated();
    }
    else if (d7asp_state == D7ASP_STATE_MASTER)
        on_request_completed();
}

void d7asp_signal_dialog_terminated()
{
    assert(d7asp_state == D7ASP_STATE_SLAVE ||
           d7asp_state == D7ASP_STATE_SLAVE_PENDING_MASTER);

    if (current_response_packet)
    {
        DPRINT("Discard the response since the dialog is now terminated");
        packet_queue_free_packet(current_response_packet);
        current_response_packet = NULL;
    }

    if (current_master_session.state == D7ASP_MASTER_SESSION_PENDING_DORMANT_TRIGGERED) {
      switch_state(D7ASP_STATE_PENDING_MASTER);
      schedule_current_session();
    } else {
      switch_state(D7ASP_STATE_IDLE);
    }

    d7ap_stack_signal_slave_session_terminated();
}
