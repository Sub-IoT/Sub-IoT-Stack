/*! \file d7atp.h
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
 * \author maarten.weyn@uantwerpen.be
 *
 */

#ifndef D7ATP_H_
#define D7ATP_H_

#include "stdint.h"
#include "stdbool.h"

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
          bool ctrl_is_ack_template_present : 1;
          bool ctrl_ack_record : 1;
          bool ctrl_ack_not_void : 1;
          bool ctrl_is_ack_requested : 1;
          uint8_t _rfu : 1;
          bool ctrl_is_timeout_template_present : 1;
          bool ctrl_is_stop : 1;
          bool ctrl_is_start : 1;
      };
    };
} d7atp_ctrl_t;


typedef struct {
    union {
        uint8_t addressee_ctrl;
        struct {
            uint8_t addressee_ctrl_access_class : 4;
            bool addressee_ctrl_virtual_id : 1;
            bool addressee_ctrl_has_id : 1; // TODO: in spec v1.0 this is defined as UCAST but will be changed in next revision
            uint8_t _rfu : 2;
        };
    };
    uint8_t addressee_id[8]; // TODO assuming 8 byte id for now
} d7atp_addressee_t;

void d7atp_start_dialog(uint8_t dialog_id, uint8_t transaction_id, packet_t* packet);
uint8_t d7atp_assemble_packet_header(packet_t* packet, uint8_t* data_ptr);
bool d7atp_disassemble_packet_header(packet_t* packet, uint8_t* data_idx);

#endif /* D7ATP_H_ */
