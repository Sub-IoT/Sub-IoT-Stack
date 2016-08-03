/*! \file d7anp.c
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
 *
 */

#include "debug.h"
#include "d7anp.h"
#include "packet.h"
#include "fs.h"
#include "ng.h"
#include "log.h"
#include "math.h"
#include "hwdebug.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_NP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_NWL, __VA_ARGS__)
#else
#define DPRINT(...)
#endif


typedef enum {
    D7ANP_STATE_IDLE,
    D7ANP_STATE_TRANSMIT,
    D7ANP_STATE_FOREGROUND_SCAN,
} state_t;

static state_t NGDEF(_d7anp_state);
#define d7anp_state NG(_d7anp_state)

static dae_access_profile_t NGDEF(_own_access_profile);
#define own_access_profile NG(_own_access_profile)

static void switch_state(state_t next_state)
{
    switch(next_state)
    {
        case D7ANP_STATE_TRANSMIT:
          assert(d7anp_state == D7ANP_STATE_IDLE ||
                 d7anp_state == D7ANP_STATE_FOREGROUND_SCAN);
          d7anp_state = next_state;
          DPRINT("Switched to D7ANP_STATE_TRANSMIT");
          break;
        case D7ANP_STATE_IDLE:
          assert(d7anp_state == D7ANP_STATE_TRANSMIT ||
                 d7anp_state == D7ANP_STATE_FOREGROUND_SCAN);
          d7anp_state = next_state;
          DPRINT("Switched to D7ANP_STATE_IDLE");
          break;
        case D7ANP_STATE_FOREGROUND_SCAN:
          assert(d7anp_state == D7ANP_STATE_TRANSMIT ||
                 d7anp_state == D7ANP_STATE_IDLE);

          d7anp_state = next_state;
          DPRINT("Switched to D7ANP_STATE_FOREGROUND_SCAN");
          break;
    }

    // output state on debug pins
    d7anp_state == D7ANP_STATE_FOREGROUND_SCAN? DEBUG_PIN_SET(3) : DEBUG_PIN_CLR(3);
}

static void foreground_scan_expired()
{
    assert(d7anp_state == D7ANP_STATE_FOREGROUND_SCAN);
    DPRINT("Foreground scan expired");

    switch_state(D7ANP_STATE_IDLE);
    dll_stop_foreground_scan();
    d7atp_signal_foreground_scan_expired();
}

static void schedule_foreground_scan_expired_timer(uint8_t timeout_ct)
{
    uint32_t timeout_ticks = pow(4, timeout_ct >> 5) * (timeout_ct & 0b11111);
    DPRINT("starting foreground scan expiration timer (%i ticks)", timeout_ticks);
    assert(timer_post_task_delay(&foreground_scan_expired, timeout_ticks) == SUCCESS);
}

static void start_foreground_scan(uint8_t timeout_ct)
{
    switch_state(D7ANP_STATE_FOREGROUND_SCAN);
    schedule_foreground_scan_expired_timer(timeout_ct); // TODO ensure > own Tc
    dll_start_foreground_scan();
}

static void cancel_foreground_scan_task()
{
    // task can be scheduled now or in the future, try to cancel both // TODO refactor scheduler API
    timer_cancel_task(&foreground_scan_expired);
    sched_cancel_task(&foreground_scan_expired);
}

static void resume_foreground_scan()
{
    if (!sched_is_scheduled(&foreground_scan_expired))
    {
        switch_state(D7ANP_STATE_FOREGROUND_SCAN);
        dll_start_foreground_scan();
    }
}

static void suspend_foreground_scan()
{
    // Tl timer is still ongoing but the foreground scan is disabled
    switch_state(D7ANP_STATE_IDLE);
    dll_stop_foreground_scan();
}

void d7anp_init()
{
    d7anp_state = D7ANP_STATE_IDLE;
    sched_register_task(&foreground_scan_expired);
}

bool is_foreground_scan_suspended()
{
  return timer_is_task_scheduled(&foreground_scan_expired);
}

void d7anp_tx_foreground_frame(packet_t* packet, bool should_include_origin_template, dae_access_profile_t* access_profile)
{
    if(d7anp_state == D7ANP_STATE_FOREGROUND_SCAN)
    {
        // Stop the FG scan and the corresponding timer if the packet to transmit is a request
        if (packet->d7atp_ctrl.ctrl_tc)
            cancel_foreground_scan_task();
        // Suspend only the FG scan and keep FG scan timer ongoing during the response transmission
        else
            suspend_foreground_scan();
    }

    packet->d7anp_ctrl.origin_addressee_ctrl_nls_enabled = false;
    packet->d7anp_ctrl.origin_addressee_ctrl_hop_enabled = false;
    if(!should_include_origin_template)
        packet->d7anp_ctrl.origin_addressee_ctrl_id_type = ID_TYPE_BCAST;
    else
    {
        uint8_t vid[2];
        fs_read_vid(vid);
        if(memcmp(vid, (uint8_t[2]){ 0xFF, 0xFF }, 2) == 0)
            packet->d7anp_ctrl.origin_addressee_ctrl_id_type = ID_TYPE_UID;
        else
            packet->d7anp_ctrl.origin_addressee_ctrl_id_type = ID_TYPE_VID;
    }

    packet->d7anp_ctrl.origin_addressee_ctrl_access_class = packet->d7anp_addressee->ctrl.access_class; // TODO validate

    switch_state(D7ANP_STATE_TRANSMIT);
    dll_tx_frame(packet, access_profile);
}

uint8_t d7anp_assemble_packet_header(packet_t *packet, uint8_t *data_ptr)
{
    assert(!packet->d7anp_ctrl.origin_addressee_ctrl_nls_enabled); // TODO NLS not yet supported
    assert(!packet->d7anp_ctrl.origin_addressee_ctrl_hop_enabled); // TODO hopping not yet supported

    uint8_t* d7anp_header_start = data_ptr;
    (*data_ptr) = packet->d7anp_listen_timeout; data_ptr++;
    (*data_ptr) = packet->d7anp_ctrl.raw; data_ptr++;

    if(packet->d7anp_ctrl.origin_addressee_ctrl_id_type != ID_TYPE_BCAST)
    {
        if(packet->d7anp_ctrl.origin_addressee_ctrl_id_type == ID_TYPE_UID)
        {
            fs_read_uid(data_ptr); data_ptr += 8;
        }
        else if(packet->d7anp_ctrl.origin_addressee_ctrl_id_type == ID_TYPE_VID)
        {
            fs_read_vid(data_ptr); data_ptr += 2;
        }
        else
        {
            assert(false);
        }
    }

    // TODO hopping ctrl

    return data_ptr - d7anp_header_start;
}

bool d7anp_disassemble_packet_header(packet_t* packet, uint8_t* data_idx)
{
    packet->d7anp_listen_timeout = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    packet->d7anp_ctrl.raw = packet->hw_radio_packet.data[(*data_idx)]; (*data_idx)++;
    assert(!packet->d7anp_ctrl.origin_addressee_ctrl_nls_enabled); // TODO NLS not yet supported
    assert(!packet->d7anp_ctrl.origin_addressee_ctrl_hop_enabled); // TODO hopping not yet supported

    if(packet->d7anp_ctrl.origin_addressee_ctrl_id_type != ID_TYPE_BCAST)
    {
        uint8_t origin_access_id_size = packet->d7anp_ctrl.origin_addressee_ctrl_id_type == ID_TYPE_VID? 2 : 8;
        memcpy(packet->origin_access_id, packet->hw_radio_packet.data + (*data_idx), origin_access_id_size); (*data_idx) += origin_access_id_size;
    }

    // TODO hopping ctrl
    // TODO security

    return true;
}

void d7anp_signal_packet_csma_ca_insertion_completed(bool succeeded)
{
    if(!succeeded)
    {
        DPRINT("CSMA-CA insertion failed");
        // resume the FG scan in order to receive a new request (Retry or New)
        if (is_foreground_scan_suspended())
            resume_foreground_scan();
        else
            switch_state(D7ANP_STATE_IDLE);
    }

    d7atp_signal_packet_csma_ca_insertion_completed(succeeded);
}

void d7anp_signal_packet_transmitted(packet_t* packet)
{
    assert(d7anp_state == D7ANP_STATE_TRANSMIT);

    // TO CONFIRM : can we assume that tc is necessary provided when the packet is a request?
    if (packet->d7atp_ctrl.ctrl_tc)
        start_foreground_scan(packet->d7atp_tc);
    else if (is_foreground_scan_suspended())
        resume_foreground_scan();

    d7atp_signal_packet_transmitted(packet);
}

void d7anp_process_received_packet(packet_t* packet)
{
    if(d7anp_state == D7ANP_STATE_FOREGROUND_SCAN)
    {
        DPRINT("Received packet while in D7ANP_STATE_FOREGROUND_SCAN");

        // update the foreground scan timer if Tl is not null
        if (packet->d7anp_listen_timeout)
        {
            DPRINT("Update the foreground scan timeout with Tl <%d>", packet->d7anp_listen_timeout);
            cancel_foreground_scan_task();
            schedule_foreground_scan_expired_timer(packet->d7anp_listen_timeout);
        }
    }
    else
    {
        DPRINT("Received packet while in D7ANP_STATE_IDLE (scan automation)");
        if (packet->d7anp_listen_timeout)
        {
            DPRINT("Start foreground scan for the duration Tl = %d", packet->d7anp_listen_timeout);
            start_foreground_scan(packet->d7anp_listen_timeout);
        }
    }

#ifdef RESPONDER_USE_FG_SCAN_OUTSIDE_TRANSACTION
    /*
     * In case of broadcast request, it may be reasonable for power saving consideration
     * to perform the FG scan outside the transaction period,
     * For that purpose, we decide to stop the FG scan and resume it when the
     * response period expires. The radio remains idle until expiration of the response
     * period but it means also that we are not able to sniff any other concurrent responses
     */
    if(!packet->dll_header.control_target_address_set)
        suspend_foreground_scan();
#endif

    d7atp_process_received_packet(packet);
}

uint8_t d7anp_addressee_id_length(id_type_t id_type)
{
    switch(id_type)
    {
        case ID_TYPE_BCAST:
          return ID_TYPE_BCAST_ID_LENGTH;
        case ID_TYPE_UID:
          return ID_TYPE_UID_ID_LENGTH;
        case ID_TYPE_VID:
          return ID_TYPE_VID_LENGTH;
        default:
          assert(false);
    }
}
