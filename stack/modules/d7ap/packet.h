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

/*! \file packet.h
 * \addtogroup Packet
 * \ingroup D7AP
 * @{
 * \brief This module contains data structures and behaviour for a D7AP 'packet' when moving through all layers of the stack
 * \author glenn.ergeerts@uantwerpen.be
 */

#ifndef OSS_7_PACKET_H
#define OSS_7_PACKET_H

#include "stdint.h"
#include "d7atp.h"
#include "dae.h"
#include "dll.h"
#include "d7anp.h"
#include "phy.h"
#include "hwradio.h"
#include "MODULE_D7AP_defs.h"


typedef enum {
    INITIAL_REQUEST,
    SUBSEQUENT_REQUEST,
    RETRY_REQUEST,
    RESPONSE_TO_UNICAST,
    RESPONSE_TO_BROADCAST,
    BACKGROUND_ADV,
    REQUEST_IN_DIALOG_EXTENSION
} packet_type;

/*! \brief A D7AP 'packet' used over all layers of the stack. Contains both the raw packet data (as transmitted over the air) as well
 * as metadata parsed or generated while moving through the different layers */
struct packet
{
    timer_tick_t request_received_timestamp;
    dll_header_t dll_header;
    d7anp_ctrl_t d7anp_ctrl;
    uint8_t origin_access_class;
    uint8_t origin_access_id[8];
    dae_nwl_security_t d7anp_security;
    d7atp_ctrl_t d7atp_ctrl;
    d7ap_addressee_t* d7anp_addressee;
    d7atp_ack_template_t d7atp_ack_template;
    uint8_t d7atp_dialog_id;
    uint8_t d7atp_transaction_id;
    uint8_t d7atp_tc;
    uint8_t d7atp_tl;
    uint8_t d7atp_te;
    uint8_t d7atp_target_rx_level_i;
    packet_type type;
    uint16_t ETA;
    uint16_t tx_duration;
    // TODO d7atp ack template
    uint8_t payload_length;
    uint8_t payload[MODULE_D7AP_PAYLOAD_SIZE];
                            // TODO store payload here or only pointer to file where we need to fetch it? can we assume data will not be changed in between
    phy_config_t phy_config;
    hw_radio_packet_t hw_radio_packet; // TODO we might not need all metadata included in hw_radio_packet_t. If not copy needed data fields
    uint8_t __data[MODULE_D7AP_RAW_PACKET_SIZE];    // reserves space for hw_radio_packet_t.data flexible array member,
                            // do not use this directly but use hw_radio_packet_t.data instead, which contains the length byte
};


void packet_init(packet_t*);
void packet_assemble(packet_t*);
void packet_disassemble(packet_t*);

#endif //OSS_7_PACKET_H

/** @}*/
