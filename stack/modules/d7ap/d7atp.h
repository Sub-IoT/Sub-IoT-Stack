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

/*! \file d7atp.h
 * \addtogroup D7ATP
 * \ingroup D7AP
 * @{
 * \brief Transport Layer Protocol APIs
 * \author	glenn.ergeerts@uantwerpen.be
 * \author	philippe.nunes@cortus.com
 */

#ifndef D7ATP_H_
#define D7ATP_H_

#include "stdint.h"
#include "stdbool.h"

#include "d7ap.h"

typedef struct packet packet_t;

/*! \brief The D7ATP CTRL header
 *
 * note: bit order is important here since this is send over the air. We explicitly reverse the order to ensure BE.
 * Although bit fields can cause portability problems it seems fine for now using gcc and the current platforms.
 * If this poses problems in the future we must resort to bit arithmetics and flags.
 */
typedef struct {
    union {
      uint8_t ctrl_raw;
      struct {
          uint8_t ctrl_agc : 1;
          bool ctrl_ack_record : 1;
          bool ctrl_ack_not_void : 1;
          bool ctrl_is_ack_requested : 1;
          bool ctrl_te : 1;
          bool ctrl_tl : 1;
          bool ctrl_xoff : 1;
          bool ctrl_is_start : 1;
      };
    };
} d7atp_ctrl_t;

typedef struct {
    union {
        uint8_t raw;
        struct {
            bool filter_segments_retry : 1;
            bool not_cnt_csma_ca_err : 1;
            bool not_cnt_duty_err : 1;
            bool _rfu : 1;
            bool xoff : 1;
            uint8_t _rfu2 : 3;
        };
    };
} d7a_segment_filter_options_t;

typedef struct {
    uint8_t ack_transaction_id_start;
    uint8_t ack_transaction_id_stop;
    // TODO ACK bitmap
} d7atp_ack_template_t;

void d7atp_init();
void d7atp_stop();
error_t  d7atp_send_request(uint8_t dialog_id, uint8_t transaction_id, bool is_last_transaction,
                        packet_t* packet, d7ap_session_qos_t* qos_settings, uint8_t listen_timeout, uint8_t expected_response_length);
error_t d7atp_send_response(packet_t* packet);
uint8_t d7atp_assemble_packet_header(packet_t* packet, uint8_t* data_ptr);
bool d7atp_disassemble_packet_header(packet_t* packet, uint8_t* data_idx);
void d7atp_signal_packet_transmitted(packet_t* packet);
void d7atp_signal_transmission_failure();
void d7atp_signal_foreground_scan_expired();
void d7atp_process_received_packet(packet_t* packet);
void d7atp_signal_dialog_termination();
void d7atp_stop_transaction();
void d7atp_notify_access_profile_file_changed(uint8_t file_id);
#endif /* D7ATP_H_ */

/** @}*/
