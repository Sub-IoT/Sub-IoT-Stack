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
#include "debug.h"
#include "d7ap.h"
#include "d7ap_fs.h"
#include "ng.h"
#include "hwsystem.h"
#include "version.h"
#include "key.h"
#include "log.h"

#define DPRINT(...) log_print_string(__VA_ARGS__)
#define DPRINT_DATA(p, len) log_print_data(p, len)
//#define DPRINT(...)


#define D7A_PROTOCOL_VERSION_MAJOR 1
#define D7A_PROTOCOL_VERSION_MINOR 1

#define FS_ENABLE_SSR_FILTER 0 // TODO always enabled? cmake param?

#define IS_SYSTEM_FILE(file_id) (file_id <= 0x3F)

static bool NGDEF(_is_fs_init_completed) = false;
#define is_fs_init_completed NG(_is_fs_init_completed)

static fs_modified_file_callback_t file_modified_callbacks[FRAMEWORK_FS_FILE_COUNT] = { NULL };

static fs_d7aactp_callback_t d7aactp_callback = NULL;


static const fs_file_header_t* systemfiles_headers = (const fs_file_header_t*)fs_systemfiles_header_data;

// the offset in blockdevice where the file data section starts
static uint32_t systemfiles_file_data_offset;

// the offset in blockdevice where the file header section starts
static uint32_t systemfiles_header_offset = 0;


static blockdevice_t* bd_systemfiles;

static inline bool is_file_defined(uint8_t file_id)
{
    return systemfiles_headers[file_id].length != 0;
}

static void execute_d7a_action_protocol(uint8_t command_file_id, uint8_t interface_file_id)
{
// TODO
//    assert(is_file_defined(command_file_id));
//    // TODO interface_file_id is optional, how do we code this in file header?
//    // for now we assume it's always used
//    assert(is_file_defined(interface_file_id));

//    uint8_t* data_ptr = (uint8_t*)(data + file_offsets[interface_file_id]);

//    d7ap_session_config_t fifo_config;
//    assert((*data_ptr) == ALP_ITF_ID_D7ASP); // only D7ASP supported for now
//    data_ptr++;
//    // TODO add length field according to spec
//    fifo_config.qos.raw = (*data_ptr); data_ptr++;
//    fifo_config.dormant_timeout = (*data_ptr); data_ptr++;;
//    // TODO add Te field according to spec
//    fifo_config.addressee.ctrl.raw = (*data_ptr); data_ptr++;
//    fifo_config.addressee.access_class = (*data_ptr); data_ptr++;
//    memcpy(&(fifo_config.addressee.id), data_ptr, 8); data_ptr += 8; // TODO assume 8 for now

//    if(d7aactp_callback)
//      d7aactp_callback(&fifo_config, (uint8_t*)(data + file_offsets[command_file_id]), file_headers[command_file_id].length);
}


void d7ap_fs_init(blockdevice_t* blockdevice_systemfiles)
{
  assert(blockdevice_systemfiles);

  bd_systemfiles = blockdevice_systemfiles;
  systemfiles_file_data_offset = (uint32_t)(fs_systemfiles_file_data - fs_systemfiles_header_data);

  // TODO platform specific

  // TODO sanity check
  // TODO set FW version


  uint8_t uid[8];
  uint8_t uid_not_set[8] = { 0 };
  d7ap_fs_read_uid(uid);
  if(memcmp(uid, uid_not_set, 8) == 0) {
    // initializing UID
    uint64_t id = hw_get_unique_id();
    uint64_t id_be = __builtin_bswap64(id);
    d7ap_fs_write_file(D7A_FILE_UID_FILE_ID, 0, (const uint8_t*)&id_be, D7A_FILE_UID_SIZE);
  }

  // always update firmware version file upon boot
  uint8_t firmware_version[D7A_FILE_FIRMWARE_VERSION_SIZE] = {
    D7A_PROTOCOL_VERSION_MAJOR, D7A_PROTOCOL_VERSION_MINOR,
  };

  memcpy(firmware_version + 2, _APP_NAME, D7A_FILE_FIRMWARE_VERSION_APP_NAME_SIZE);
  memcpy(firmware_version + 2 + D7A_FILE_FIRMWARE_VERSION_APP_NAME_SIZE, _GIT_SHA1, D7A_FILE_FIRMWARE_VERSION_GIT_SHA1_SIZE);
  d7ap_fs_write_file(D7A_FILE_FIRMWARE_VERSION_FILE_ID, 0, firmware_version, D7A_FILE_FIRMWARE_VERSION_SIZE);

//    // 0x0A - DLL Configuration
//    file_offsets[D7A_FILE_DLL_CONF_FILE_ID] = current_data_offset;
//    file_headers[D7A_FILE_DLL_CONF_FILE_ID] = (fs_file_header_t){
//        .file_properties.action_protocol_enabled = 0,
//        .file_properties.storage_class = FS_STORAGE_RESTORABLE,
//        .file_permissions = 0, // TODO
//        .length = D7A_FILE_DLL_CONF_SIZE,
//        .allocated_length = D7A_FILE_DLL_CONF_SIZE
//    };

//    data[current_data_offset] = init_args->access_class; current_data_offset += 1; // active access class
//    memset(data + current_data_offset, 0xFF, 2); current_data_offset += 2; // VID; 0xFFFF means not valid

//    d7aactp_callback = init_args->fs_d7aactp_cb;
//    is_fs_init_completed = true;
}

//void fs_init_file(uint8_t file_id, const fs_file_header_t* file_header, const uint8_t* initial_data)
//{
//    assert(!is_fs_init_completed); // initing files not allowed after fs_init() completed (for now?)
//    assert(file_id < FRAMEWORK_FS_FILE_COUNT);
//    assert(file_id >= 0x40); // system files may not be inited
//    assert(current_data_offset + file_header->length <= FRAMEWORK_FS_FILESYSTEM_SIZE);

//    file_offsets[file_id] = current_data_offset;
//    memcpy(file_headers + file_id, file_header, sizeof(fs_file_header_t));
//    memset(data + current_data_offset, 0, file_header->length);
//    current_data_offset += file_header->length;
//    if(initial_data != NULL)
//        fs_write_file(file_id, 0, initial_data, file_header->length);
//}

//void fs_init_file_with_d7asp_interface_config(uint8_t file_id, const d7ap_session_config_t* fifo_config)
//{
//    // TODO check file not already defined

//    uint8_t alp_command_buffer[40] = { 0 };
//    uint8_t* ptr = alp_command_buffer;
//    (*ptr) = ALP_ITF_ID_D7ASP; ptr++;
//    (*ptr) = fifo_config->qos.raw; ptr++;
//    (*ptr) = fifo_config->dormant_timeout; ptr++;
//    (*ptr) = fifo_config->addressee.ctrl.raw; ptr++;
//    (*ptr) = fifo_config->addressee.access_class; ptr++;
//    memcpy(ptr, &(fifo_config->addressee.id), 8); ptr += 8; // TODO assume 8 for now

//    uint32_t len = ptr - alp_command_buffer;
//    // TODO fixed header implemented here, or should this be configurable by app?
//    fs_file_header_t file_header = (fs_file_header_t){
//        .file_properties.action_protocol_enabled = 0,
//        .file_properties.storage_class = FS_STORAGE_PERMANENT,
//        .file_permissions = 0, // TODO
//        .length = len,
//        .allocated_length = len,
//    };

//    fs_init_file(file_id, &file_header, alp_command_buffer);
//}

//void fs_init_file_with_D7AActP(uint8_t file_id, const d7ap_session_config_t* fifo_config, const uint8_t* alp_command, const uint8_t alp_command_len)
//{
//    uint8_t alp_command_buffer[40] = { 0 };
//    uint8_t* ptr = alp_command_buffer;
//    (*ptr) = ALP_ITF_ID_D7ASP; ptr++;
//    (*ptr) = fifo_config->qos.raw; ptr++;
//    (*ptr) = fifo_config->dormant_timeout; ptr++;
//    (*ptr) = fifo_config->addressee.ctrl.raw; ptr++;
//    (*ptr) = fifo_config->addressee.access_class; ptr++;
//    memcpy(ptr, &(fifo_config->addressee.id), 8); ptr += 8; // TODO assume 8 for now

//    memcpy(ptr, alp_command, alp_command_len); ptr += alp_command_len;

//    uint32_t len = ptr - alp_command_buffer;
//    // TODO fixed header implemented here, or should this be configurable by app?
//    fs_file_header_t action_file_header = (fs_file_header_t){
//        .file_properties.action_protocol_enabled = 0,
//        .file_properties.storage_class = FS_STORAGE_PERMANENT,
//        .file_permissions = 0, // TODO
//        .length = len,
//        .allocated_length = len,
//    };

//    fs_init_file(file_id, &action_file_header, alp_command_buffer);
//}

alp_status_codes_t d7ap_fs_read_file(uint8_t file_id, uint8_t offset, uint8_t* buffer, uint8_t length)
{
  assert(IS_SYSTEM_FILE(file_id)); // TODO user files not implemented
  if(!is_file_defined(file_id)) return ALP_STATUS_FILE_ID_NOT_EXISTS;
  if(systemfiles_headers[file_id].length < offset + length) return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error (wait for spec discussion)

  DPRINT("RD %i + %i = %i\n", systemfiles_file_data_offset, fs_systemfiles_file_offsets[file_id], systemfiles_file_data_offset + fs_systemfiles_file_offsets[file_id]);
  blockdevice_read(bd_systemfiles, buffer, systemfiles_file_data_offset + fs_systemfiles_file_offsets[file_id] + offset, length);
  return ALP_STATUS_OK;
}

alp_status_codes_t d7ap_fs_read_file_header(uint8_t file_id, fs_file_header_t* file_header)
{
  if(!is_file_defined(file_id)) return ALP_STATUS_FILE_ID_NOT_EXISTS;

  blockdevice_read(bd_systemfiles, (uint8_t*)file_header, systemfiles_header_offset + (file_id * sizeof(fs_file_header_t)), sizeof(fs_file_header_t));
  return ALP_STATUS_OK;
}

alp_status_codes_t d7ap_fs_write_file_header(uint8_t file_id, fs_file_header_t* file_header)
{
  if(!is_file_defined(file_id)) return ALP_STATUS_FILE_ID_NOT_EXISTS;

  blockdevice_program(bd_systemfiles, (uint8_t*)file_header, systemfiles_header_offset + (file_id * sizeof(fs_file_header_t)), sizeof(fs_file_header_t));
  return ALP_STATUS_OK;
}

alp_status_codes_t d7ap_fs_write_file(uint8_t file_id, uint8_t offset, const uint8_t* buffer, uint8_t length)
{
    assert(IS_SYSTEM_FILE(file_id)); // TODO user files not implemented
    if(!is_file_defined(file_id)) return ALP_STATUS_FILE_ID_NOT_EXISTS;
    if(systemfiles_headers[file_id].length < offset + length) return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error (wait for spec discussion)

    blockdevice_program(bd_systemfiles, buffer, systemfiles_file_data_offset + fs_systemfiles_file_offsets[file_id] + offset, length);

    if(systemfiles_headers[file_id].file_properties.action_protocol_enabled == true
            && systemfiles_headers[file_id].file_properties.action_condition == ALP_ACT_COND_WRITE) // TODO ALP_ACT_COND_WRITEFLUSH?
    {
        execute_d7a_action_protocol(systemfiles_headers[file_id].alp_cmd_file_id, systemfiles_headers[file_id].interface_file_id);
    }


    if(file_modified_callbacks[file_id])
      file_modified_callbacks[file_id](file_id);

    return ALP_STATUS_OK;
}

void d7ap_fs_read_uid(uint8_t *buffer)
{
    d7ap_fs_read_file(D7A_FILE_UID_FILE_ID, 0, buffer, D7A_FILE_UID_SIZE);
}

void d7ap_fs_read_vid(uint8_t *buffer)
{
//    fs_read_file(D7A_FILE_DLL_CONF_FILE_ID, 1, buffer, 2);
}


alp_status_codes_t d7ap_fs_read_nwl_security_key(uint8_t *buffer)
{
//    return fs_read_file(D7A_FILE_NWL_SECURITY_KEY, 0, buffer, D7A_FILE_NWL_SECURITY_KEY_SIZE);
}

alp_status_codes_t d7ap_fs_read_nwl_security(dae_nwl_security_t *nwl_security)
{
//    uint8_t* data_ptr = data + file_offsets[D7A_FILE_NWL_SECURITY];
//    uint32_t frame_counter;

//    if(!is_file_defined(D7A_FILE_NWL_SECURITY)) return ALP_STATUS_FILE_ID_NOT_EXISTS;

//    nwl_security->key_counter = (*data_ptr); data_ptr++;
//    memcpy(&frame_counter, data_ptr, sizeof(uint32_t));
//    nwl_security->frame_counter = (uint32_t)__builtin_bswap32(frame_counter);
//    return ALP_STATUS_OK;
}

alp_status_codes_t d7ap_fs_write_nwl_security(dae_nwl_security_t *nwl_security)
{
//    uint8_t* data_ptr = data + file_offsets[D7A_FILE_NWL_SECURITY];
//    uint32_t frame_counter;

//    if(!is_file_defined(D7A_FILE_NWL_SECURITY)) return ALP_STATUS_FILE_ID_NOT_EXISTS;

//    (*data_ptr) = nwl_security->key_counter; data_ptr++;
//    frame_counter = __builtin_bswap32(nwl_security->frame_counter);
//    memcpy(data_ptr, &frame_counter, sizeof(uint32_t));
//    return ALP_STATUS_OK;
}

alp_status_codes_t d7ap_fs_read_nwl_security_state_register(dae_nwl_ssr_t *node_security_state)
{
//    uint8_t* data_ptr = data + file_offsets[D7A_FILE_NWL_SECURITY_STATE_REG];
//    uint32_t frame_counter;

//    if(!is_file_defined(D7A_FILE_NWL_SECURITY_STATE_REG)) return ALP_STATUS_FILE_ID_NOT_EXISTS;

//    node_security_state->filter_mode = (*data_ptr); data_ptr++;
//    node_security_state->trusted_node_nb = (*data_ptr); data_ptr++;

//    for(uint8_t i = 0; i < node_security_state->trusted_node_nb; i++)
//    {
//        node_security_state->trusted_node_table[i].key_counter = (*data_ptr); data_ptr++;
//        memcpy(&frame_counter, data_ptr, sizeof(uint32_t)); data_ptr += sizeof(uint32_t);
//        node_security_state->trusted_node_table[i].frame_counter = (uint32_t)__builtin_bswap32(frame_counter);
//        memcpy(node_security_state->trusted_node_table[i].addr, data_ptr, 8); data_ptr += 8;
//    }
//    return ALP_STATUS_OK;
}

alp_status_codes_t d7ap_fs_add_nwl_security_state_register_entry(dae_nwl_trusted_node_t *trusted_node,
                                                            uint8_t trusted_node_nb)
{
//    uint8_t* data_ptr = data + file_offsets[D7A_FILE_NWL_SECURITY_STATE_REG];
//    uint32_t frame_counter;

//    if(!is_file_defined(D7A_FILE_NWL_SECURITY_STATE_REG)) return ALP_STATUS_FILE_ID_NOT_EXISTS;

//    data_ptr[1] = trusted_node_nb;
//    data_ptr += (D7A_FILE_NWL_SECURITY_SIZE + D7A_FILE_UID_SIZE)*(trusted_node_nb - 1) + 2;
//    assert(trusted_node_nb <= FRAMEWORK_FS_TRUSTED_NODE_TABLE_SIZE);

//    (*data_ptr) = trusted_node->key_counter; data_ptr++;
//    frame_counter = __builtin_bswap32(trusted_node->frame_counter);
//    memcpy(data_ptr, &frame_counter, sizeof(uint32_t));
//    data_ptr += sizeof(uint32_t);
//    memcpy(data_ptr, trusted_node->addr, 8);
//    return ALP_STATUS_OK;
}

alp_status_codes_t d7ap_fs_update_nwl_security_state_register(dae_nwl_trusted_node_t *trusted_node,
                                                        uint8_t trusted_node_index)
{
//    uint8_t* data_ptr = data + file_offsets[D7A_FILE_NWL_SECURITY_STATE_REG];
//    uint32_t frame_counter;

//    if(!is_file_defined(D7A_FILE_NWL_SECURITY_STATE_REG)) return ALP_STATUS_FILE_ID_NOT_EXISTS;

//    data_ptr += (D7A_FILE_NWL_SECURITY_SIZE + D7A_FILE_UID_SIZE)*(trusted_node_index - 1) + 2;
//    (*data_ptr) = trusted_node->key_counter; data_ptr++;
//    frame_counter = __builtin_bswap32(trusted_node->frame_counter);
//    memcpy(data_ptr, &frame_counter, sizeof(uint32_t));
//    return ALP_STATUS_OK;
}

void d7ap_fs_read_access_class(uint8_t access_class_index, dae_access_profile_t *access_class)
{
  assert(access_class_index < 15);
  assert(is_file_defined(D7A_FILE_ACCESS_PROFILE_ID + access_class_index));
  d7ap_fs_read_file(D7A_FILE_ACCESS_PROFILE_ID + access_class_index, 0, (uint8_t*)access_class, D7A_FILE_ACCESS_PROFILE_SIZE);
}

void d7ap_fs_write_access_class(uint8_t access_class_index, dae_access_profile_t* access_class)
{
  assert(access_class_index < 15);
  assert(is_file_defined(D7A_FILE_ACCESS_PROFILE_ID + access_class_index));
  d7ap_fs_write_file(D7A_FILE_ACCESS_PROFILE_ID + access_class_index, 0, (uint8_t*)access_class, D7A_FILE_ACCESS_PROFILE_SIZE);
}

uint8_t d7ap_fs_read_dll_conf_active_access_class()
{
  uint8_t access_class;
  d7ap_fs_read_file(D7A_FILE_DLL_CONF_FILE_ID, 0, &access_class, 1);
  return access_class;
}

void d7ap_fs_write_dll_conf_active_access_class(uint8_t access_class)
{
  d7ap_fs_write_file(D7A_FILE_DLL_CONF_FILE_ID, 0, &access_class, 1);
}

uint8_t d7ap_fs_get_file_length(uint8_t file_id)
{
  assert(is_file_defined(file_id));
  return systemfiles_headers[file_id].length;
}

bool d7ap_fs_register_file_modified_callback(uint8_t file_id, fs_modified_file_callback_t callback)
{
//  if(file_modified_callbacks[file_id])
//    return false; // already registered

//  file_modified_callbacks[file_id] = callback;
}
