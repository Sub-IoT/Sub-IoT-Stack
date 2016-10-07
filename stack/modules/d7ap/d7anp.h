/*! \file d7anp.h
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
 * \author glenn.ergeerts@uantwerpen.be
 *
 */

#ifndef D7ANP_H_
#define D7ANP_H_

#include "stdint.h"
#include "stdbool.h"

#include "dae.h"

typedef struct packet packet_t;

typedef enum {
    ID_TYPE_NBID = 0,
    ID_TYPE_NOID = 1,
    ID_TYPE_UID = 2,
    ID_TYPE_VID = 3
} id_type_t;

#define ID_TYPE_NBID_ID_LENGTH 1
#define ID_TYPE_NOID_ID_LENGTH 0
#define ID_TYPE_UID_ID_LENGTH   8
#define ID_TYPE_VID_LENGTH      2

#define ID_TYPE_IS_BROADCAST(id_type) (id_type == ID_TYPE_NBID || id_type == ID_TYPE_NOID)

#define GET_NLS_METHOD(VAL) (uint8_t)(VAL & 0x0F)
#define SET_NLS_METHOD(VAL) (uint8_t)(VAL << 4 & 0xF0)

enum
{
    AES_NONE = 0, /* No security */
    AES_CTR = 0x01, /* data confidentiality */
    AES_CBC_MAC_128 = 0x02, /* data authenticity */
    AES_CBC_MAC_64 = 0x03, /* data authenticity */
    AES_CBC_MAC_32 = 0x04, /* data authenticity */
    AES_CCM_128 = 0x05, /* data confidentiality and authenticity*/
    AES_CCM_64 = 0x06, /* data confidentiality and authenticity*/
    AES_CCM_32 = 0x07, /* data confidentiality and authenticity*/
};

typedef struct {
    union {
        uint8_t raw;
        struct {
            uint8_t nls_method : 4;
            id_type_t id_type : 2;
            uint8_t _rfu : 2;
        };
    };
} d7anp_addressee_ctrl;

typedef struct {
    d7anp_addressee_ctrl ctrl;
    union {
        uint8_t access_class;
        struct {
            uint8_t access_mask : 4;
            uint8_t access_index : 4;
        };
    };
    uint8_t id[8]; // TODO assuming 8 byte id for now
} d7anp_addressee_t;

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
            id_type_t origin_id_type : 2;
            bool hop_enabled : 1;
            bool origin_void : 1;
        };
    };
} d7anp_ctrl_t;

typedef struct {
    uint8_t key_counter;
    uint32_t frame_counter;
} d7anp_security_t;

void d7anp_init();
void d7anp_tx_foreground_frame(packet_t* packet, bool should_include_origin_template, uint8_t slave_listen_timeout_ct);
uint8_t d7anp_assemble_packet_header(packet_t* packet, uint8_t* data_ptr);
bool d7anp_disassemble_packet_header(packet_t* packet, uint8_t* packet_idx);
void d7anp_signal_transmission_failure();
void d7anp_signal_packet_transmitted(packet_t* packet);
void d7anp_process_received_packet(packet_t* packet, bool background_frame);
uint8_t d7anp_addressee_id_length(id_type_t);
void d7anp_set_foreground_scan_timeout(timer_tick_t timeout);
void d7anp_start_foreground_scan();
void d7anp_stop_foreground_scan(bool auto_scan);
uint8_t d7anp_secure_payload(packet_t* packet, uint8_t* payload, uint8_t payload_len);

#endif /* D7ANP_H_ */
