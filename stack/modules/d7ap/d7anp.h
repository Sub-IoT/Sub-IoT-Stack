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

/*! \file d7anp.h
 * \addtogroup D7ANP
 * \ingroup D7AP
 * @{
 * \brief Network Layer Protocol APIs
 * \author	glenn.ergeerts@uantwerpen.be
 * \author	philippe.nunes@cortus.com
 */

#ifndef D7ANP_H_
#define D7ANP_H_

#include "stdint.h"
#include "stdbool.h"

#include "MODULE_D7AP_defs.h"
#include "timer.h"
#include "packet.h"

typedef struct packet packet_t;

#define ID_TYPE_IS_BROADCAST(id_type) (id_type == ID_TYPE_NBID || id_type == ID_TYPE_NOID)

#define GET_NLS_METHOD(VAL) (uint8_t)(VAL & 0x0F)
#define SET_NLS_METHOD(VAL) (uint8_t)(VAL << 4 & 0xF0)

#define ENABLE_SSR_FILTER 0x01
#define ALLOW_NEW_SSR_ENTRY_IN_BCAST 0x02

#define FG_SCAN_TIMEOUT    200   // expressed in Ti, to be adjusted
#define FG_SCAN_STARTUP_TIME 3   // to be adjusted per platform
#define CLK_ACCURACY_100_MS 1 // safety margin to start the FG scan before the designated ETA, this represents the amount of ticks the used clock can stray in 100 ms (in this case an accuracy of 1%)


/*! \brief The D7ANP CTRL header
 *
 * note: bit order is important here since this is send over the air. We explicitly reverse the order to ensure BE.
 * Although bit fields can cause portability problems it seems fine for now using gcc and the current platforms.
 * If this poses problems in the future we must resort to bit arithmetics and flags.
 */
typedef struct {
    union {
        uint8_t raw;
        struct {
            uint8_t nls_method : 4;
            d7ap_addressee_id_type_t origin_id_type : 2;
            bool hop_enabled : 1;
            bool origin_void : 1;
        };
    };
} d7anp_ctrl_t;



void d7anp_init();
void d7anp_stop();
error_t d7anp_tx_foreground_frame(packet_t* packet, bool should_include_origin_template);
uint8_t d7anp_assemble_packet_header(packet_t* packet, uint8_t* data_ptr);
bool d7anp_disassemble_packet_header(packet_t* packet, uint8_t* packet_idx);
void d7anp_signal_transmission_failure();
void d7anp_signal_packet_transmitted(packet_t* packet);
void d7anp_process_received_packet(packet_t* packet);
void d7anp_set_foreground_scan_timeout(timer_tick_t timeout);
void d7anp_start_foreground_scan();
void d7anp_stop_foreground_scan();
uint8_t d7anp_secure_payload(packet_t* packet, uint8_t* payload, uint8_t payload_len);

#endif /* D7ANP_H_ */

/** @}*/
