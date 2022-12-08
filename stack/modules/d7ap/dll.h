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

/*! \file dll.h
 * \addtogroup DLL
 * \ingroup D7AP
 * @{
 * \brief This module implements the DLL layer of the D7AP.
 *
 * \author glenn.ergeerts@uantwerpen.be
 * \author maarten.weyn@uantwerpen.be
 * \author philippe.nunes@cortus.com
 */

#ifndef OSS_7_DLL_H
#define OSS_7_DLL_H

#include "phy.h"

#include "d7anp.h"

typedef enum {
    D7ADLL_FIXED_NOISE_FLOOR,
    D7ADLL_SLOW_RSSI_VARIATION,
    D7ADLL_MEDIAN_OF_THREE
} noise_floor_computation_method_t;

/*
 * Background frames are of fixed length
 * Subnet / CTRL / Payload / CRC16
 * 1 byte /1 byte/ 2 bytes / 2 bytes
 */
#define BACKGROUND_FRAME_LENGTH 6

#define SFc    3 // Collision Avoidance Spreading Factor

#define t_g    5 // Guarding period

#define PHY_STATUS_CHANNEL_BYTES 3
#define PHY_STATUS_MAX_CHANNELS 10

#define NOISEFL_NUMBER_MEASUREMENTS 3

typedef struct packet packet_t;

typedef struct
{
    uint8_t subnet;
    union
    {
        int8_t control_eirp_index;
        int8_t control_identifier_tag;
    };

    d7ap_addressee_id_type_t control_target_id_type;
    //uint8_t target_address[8]; // TODO assuming 8B UID for now
} dll_header_t;

typedef enum
{
    CSMA_CA_MODE_UNC = 0,
    CSMA_CA_MODE_AIND = 1,
    CSMA_CA_MODE_RAIND = 2,
    CSMA_CA_MODE_RIGD = 3
} csma_ca_mode_t;

void dll_init();
void dll_stop();
void dll_tx_frame(packet_t* packet);
void dll_start_foreground_scan();
void dll_stop_foreground_scan();
void dll_stop_background_scan();
void dll_execute_scan_automation();

uint8_t dll_assemble_packet_header(packet_t* packet, uint8_t* data_ptr);
uint8_t dll_assemble_packet_header_bg(packet_t* packet, uint8_t* data_ptr);
bool dll_disassemble_packet_header(packet_t* packet, uint8_t* data_idx);
void dll_signal_packet_transmitted(packet_t* packet);
void dll_signal_packet_received(packet_t* packet);
void dll_notify_access_profile_file_changed(uint8_t file_id);
void dll_notify_dialog_terminated();

#endif //OSS_7_DLL_H

/** @}*/
