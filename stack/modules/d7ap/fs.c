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
#include "alp.h"
#include "d7asp.h"

#define FILE_COUNT 0x42 // TODO define from cmake
#define FILE_DATA_SIZE 45 // TODO define from cmake (per platform?)

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
#define USER_FILE_COUNTER_SIZE 4

#define ACTION_FILE_ID_BROADCAST_COUNTER 0x41

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

    // User files
    // TODO define user files from application

    // User file counter
    file_offsets[USER_FILE_COUNTER_ID] = data_ptr - data;
    file_headers[USER_FILE_COUNTER_ID] = (fs_file_header_t){
        .file_properties.action_protocol_enabled = 1,
        .file_properties.action_file_id = ACTION_FILE_ID_BROADCAST_COUNTER,
        .file_properties.action_condition = ALP_ACT_COND_WRITE,
        .file_properties.storage_class = FS_STORAGE_VOLATILE,
        .file_properties.permissions = 0, // TODO
        .length = USER_FILE_COUNTER_SIZE
    };

    memset(data_ptr, 0, USER_FILE_COUNTER_SIZE);
    data_ptr += USER_FILE_COUNTER_SIZE;

    // Action file, broadcast ALP command
    d7asp_fifo_config_t d7asp_fifo_config = {
        .fifo_ctrl_nls = false,
        .fifo_ctrl_stop_on_error = false,
        .fifo_ctrl_preferred = false,
        .fifo_ctrl_state = SESSION_STATE_PENDING,
        .qos = 0, // TODO
        .dormant_timeout = 0,
        .start_id = 0, // TODO
        .addressee = {
            .addressee_ctrl_unicast = false,
            .addressee_ctrl_virtual_id = false,
            .addressee_ctrl_access_class = 0,
            .addressee_id = 0
        }
    };

    alp_control_t alp_ctrl = {
        .group = false,
        .response_requested = false,
        .operation = ALP_OP_READ_FILE_DATA
    };

    alp_operand_file_data_request_t file_data_request_operand = {
        .file_offset = {
            .file_id = USER_FILE_COUNTER_ID,
            .offset = 0
        },
        .requested_data_length = USER_FILE_COUNTER_SIZE,
    };

    uint8_t* alp_command_start = data_ptr;
    (*data_ptr) = ALP_ITF_ID_D7ASP; data_ptr++;
    (*data_ptr) = d7asp_fifo_config.fifo_ctrl; data_ptr++;
    memcpy(data_ptr, &(d7asp_fifo_config.qos), 4); data_ptr += 4;
    (*data_ptr) = d7asp_fifo_config.dormant_timeout; data_ptr++;
    (*data_ptr) = d7asp_fifo_config.start_id; data_ptr++;
    (*data_ptr) = d7asp_fifo_config.addressee.addressee_ctrl; data_ptr++;
    memcpy(data_ptr, &(d7asp_fifo_config.addressee.addressee_id), 8); data_ptr += 8; // TODO assume 8 for now

    (*data_ptr) = alp_ctrl.raw; data_ptr++;

    (*data_ptr) = file_data_request_operand.file_offset.file_id; data_ptr++;
    (*data_ptr) = file_data_request_operand.file_offset.offset; data_ptr++; // TODO can be 1-4 bytes, assume 1 for now
    (*data_ptr) = file_data_request_operand.requested_data_length; data_ptr++;

    file_offsets[ACTION_FILE_ID_BROADCAST_COUNTER] = alp_command_start - data;
    file_headers[ACTION_FILE_ID_BROADCAST_COUNTER] = (fs_file_header_t){
        .file_properties.action_protocol_enabled = 0,
        .file_properties.storage_class = FS_STORAGE_PERMANENT,
        .file_properties.permissions = 0, // TODO
        .length = data_ptr - alp_command_start
    };

    assert(data_ptr - data <= FILE_DATA_SIZE);
}

void execute_alp_command(uint8_t command_file_id)
{
    assert(is_file_defined(command_file_id));
    uint8_t* data_ptr = (uint8_t*)(data + file_offsets[command_file_id]);
    uint8_t* file_start = data_ptr;

    // parse ALP command
    d7asp_fifo_config_t fifo_config;
    assert((*data_ptr) == ALP_ITF_ID_D7ASP); // only D7ASP supported for now
    data_ptr++;
    fifo_config.fifo_ctrl = (*data_ptr); data_ptr++;
    memcpy(&(fifo_config.qos), data_ptr, 4); data_ptr += 4;
    fifo_config.dormant_timeout = (*data_ptr); data_ptr++;
    fifo_config.start_id = (*data_ptr); data_ptr++;
    fifo_config.addressee.addressee_ctrl = (*data_ptr); data_ptr++;
    memcpy(&(fifo_config.addressee.addressee_id), data_ptr, 8); data_ptr += 8; // TODO assume 8 for now

    d7asp_queue_alp_actions(&fifo_config, data_ptr, file_headers[command_file_id].length - (uint8_t)(data_ptr - file_start));
}

void fs_read_file(uint8_t file_id, uint8_t offset, uint8_t* buffer, uint8_t length)
{
    assert(is_file_defined(file_id));
    assert(file_headers[file_id].length >= offset + length);
    memcpy(buffer, data + file_offsets[file_id] + offset, length);
}

void fs_write_file(uint8_t file_id, uint8_t offset, uint8_t* buffer, uint8_t length)
{
    assert(is_file_defined(file_id));
    assert(file_headers[file_id].length >= offset + length);
    memcpy(data + file_offsets[file_id] + offset, buffer, length);

    if(file_headers[file_id].file_properties.action_protocol_enabled == true
            && file_headers[file_id].file_properties.action_condition == ALP_ACT_COND_WRITE) // TODO ALP_ACT_COND_WRITEFLUSH?
    {
        execute_alp_command(file_headers[file_id].file_properties.action_file_id);
    }
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
