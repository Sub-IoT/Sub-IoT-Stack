/*! \file fs.c
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
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include "string.h"
#include "assert.h"
#include "fs.h"
#include "ng.h"
#include "hwsystem.h"


#define FILE_COUNT 0x21 // TODO define from cmake
#define FILE_DATA_SIZE 22 // TODO define from cmake (per platform?)

static fs_file_header_t NGDEF(_file_headers)[FILE_COUNT] = { 0 };
#define file_headers NG(_file_headers)

static uint8_t NGDEF(_data)[FILE_DATA_SIZE] = { 0 };
#define data NG(_data)

static uint16_t NGDEF(_file_offsets)[FILE_COUNT] = { 0 };
#define file_offsets NG(_file_offsets)

#define D7A_FILE_UID_FILE_ID 0x00
#define D7A_FILE_UID_SIZE 8

#define D7A_FILE_ACCESS_PROFILE_ID 0x20 // the first access class file
#define D7A_FILE_ACCESS_PROFILE_SIZE 12 // TODO assuming 1 subband

#define USER_FILE_COUNTER_ID 0x40
#define USER_FILE_COUNTER_SIZE 2

#define ACTION_FILE_BROADCAST_ID 0x41

static inline bool is_file_defined(uint8_t file_id)
{
    return file_headers[file_id].length != 0;
}

void fs_init()
{
    uint8_t* data_ptr = data;

    // UID
    file_offsets[D7A_FILE_UID_FILE_ID] = data_ptr - data;
    file_headers[D7A_FILE_UID_FILE_ID] = (fs_file_header_t){
        .file_properties.action_protocol_enabled = 0,
        .file_properties.storage_class = FS_STORAGE_PERMANENT,
        .file_properties.permissions = 0, // TODO
        .length = D7A_FILE_UID_SIZE
    };
    uint64_t id = hw_get_unique_id();
    memcpy(data_ptr, &id, D7A_FILE_UID_SIZE);
    data_ptr += D7A_FILE_UID_SIZE;

    // Access class 0
    dae_access_profile_t default_access_class = (dae_access_profile_t){
        .control_scan_type_is_foreground = true,
        .control_csma_ca_mode = CSMA_CA_MODE_UNC,
        .control_number_of_subbands = 1,
        .subnet = 0x05,
        .scan_automation_period = 0,
        .transmission_timeout_period = 0,
        .subbands[0] = (subband_t){
            .channel_header = {
                .ch_coding = PHY_CODING_PN9,
                .ch_class = PHY_CLASS_NORMAL_RATE,
                .ch_freq_band = PHY_BAND_433
            },
            .channel_index_start = 0,
            .channel_index_end = 0,
            .eirp = 0,
            .ccao = 0
        }
    };

    file_offsets[D7A_FILE_ACCESS_PROFILE_ID] = data_ptr - data;
    fs_write_access_class(0, &default_access_class);
    file_headers[D7A_FILE_ACCESS_PROFILE_ID] = (fs_file_header_t){
        .file_properties.action_protocol_enabled = 0,
        .file_properties.storage_class = FS_STORAGE_PERMANENT,
        .file_properties.permissions = 0, // TODO
        .length = D7A_FILE_ACCESS_PROFILE_SIZE
    };

    data_ptr += D7A_FILE_ACCESS_PROFILE_SIZE;

    // User file counter
    file_offsets[USER_FILE_COUNTER_ID] = data_ptr - data;
    file_headers[USER_FILE_COUNTER_ID] = (fs_file_header_t){
        .file_properties.action_protocol_enabled = 1,
        .file_properties.action_file_id = ACTION_FILE_BROADCAST_ID,
        .file_properties.action_condition = ALP_ACT_COND_WRITE,
        .file_properties.storage_class = FS_STORAGE_VOLATILE,
        .file_properties.permissions = 0, // TODO
        .length = USER_FILE_COUNTER_SIZE
    };

    memset(data_ptr, 0, USER_FILE_COUNTER_SIZE);
    data_ptr += USER_FILE_COUNTER_SIZE;


    assert(data_ptr - data <= FILE_DATA_SIZE);
}

void fs_write_access_class(uint8_t access_class_index, dae_access_profile_t* access_class)
{
    assert(access_class_index < 16);
    assert(access_class->control_number_of_subbands == 1); // TODO only one supported for now
    uint8_t* data_ptr = data + file_offsets[D7A_FILE_ACCESS_PROFILE_ID + access_class_index];
    (*data_ptr) = access_class->control; data_ptr++;
    (*data_ptr) = access_class->subnet; data_ptr++;
    (*data_ptr) = access_class->scan_automation_period; data_ptr++;
    (*data_ptr) = access_class->transmission_timeout_period; data_ptr++;
    (*data_ptr) = 0x00; data_ptr++; // RFU
    // subbands, only 1 supported for now
    memcpy(data_ptr, &(access_class->subbands[0].channel_header), 1); data_ptr++;
    memcpy(data_ptr, &(access_class->subbands[0].channel_index_start), 2); data_ptr += 2;
    memcpy(data_ptr, &(access_class->subbands[0].channel_index_end), 2); data_ptr += 2;
    (*data_ptr) = access_class->subbands[0].eirp; data_ptr++;
    (*data_ptr) = access_class->subbands[0].ccao; data_ptr++;
}

void fs_read_access_class(uint8_t access_class_index, dae_access_profile_t *access_class)
{
    assert(access_class_index < 16);
    assert(is_file_defined(D7A_FILE_ACCESS_PROFILE_ID + access_class_index));
    uint8_t* data_ptr = data + file_offsets[D7A_FILE_ACCESS_PROFILE_ID + access_class_index];
    access_class->control = (*data_ptr); data_ptr++;
    access_class->subnet = (*data_ptr); data_ptr++;
    access_class->scan_automation_period = (*data_ptr); data_ptr++;
    access_class->transmission_timeout_period = (*data_ptr); data_ptr++;
    data_ptr++; // RFU
    // subbands, only 1 supported for now
    memcpy(&(access_class->subbands[0].channel_header), data_ptr, 1); data_ptr++;
    memcpy(&(access_class->subbands[0].channel_index_start), data_ptr, 2); data_ptr += 2;
    memcpy(&(access_class->subbands[0].channel_index_end), data_ptr, 2); data_ptr += 2;
    access_class->subbands[0].eirp = (*data_ptr); data_ptr++;
    access_class->subbands[0].ccao = (*data_ptr); data_ptr++;
}
