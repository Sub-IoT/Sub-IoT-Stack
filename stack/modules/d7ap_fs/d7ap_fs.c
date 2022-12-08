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
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include "modules_defs.h"
#include "MODULE_D7AP_FS_defs.h"
#include "framework_defs.h"
#include "string.h"
#include "debug.h"
#include "fs.h"
#include "d7ap_fs.h"
#include "version.h"
#include "log.h"

///////////////////////////////////////
// The d7a file header is concatenated with the file data.
// filebody is [$header][$data]
// where length(header) = 12
//     length($data) == $header.length == $header.allocated_length
///////////////////////////////////////



#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_FS_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_NWL, __VA_ARGS__)
#define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINT_DATA(...)
#endif

#define D7A_PROTOCOL_VERSION_MAJOR 1
#define D7A_PROTOCOL_VERSION_MINOR 1

#define FS_STORAGE_COUNT (sizeof(d7ap_fs)/sizeof(d7ap_fs[0]))

#define FS_ENABLE_SSR_FILTER 0 // TODO always enabled? cmake param?

#define IS_SYSTEM_FILE(file_id) (file_id <= 0x3F)

#define FILE_SIZE_MAX (MODULE_D7AP_FS_FILE_SIZE_MAX + sizeof(d7ap_fs_file_header_t))
static uint8_t file_buffer[FILE_SIZE_MAX]; // statically allocated buffer used during file operations, to prevent stack overflow at runtime

static d7ap_fs_modified_file_callback_t file_modified_callbacks[FRAMEWORK_FS_FILE_COUNT] = { NULL }; // TODO limit to lower number so save RAM?
static d7ap_fs_modifying_file_callback_t file_modifying_callbacks[FRAMEWORK_FS_FILE_COUNT] = { NULL };

#if defined(MODULE_ALP) && defined(MODULE_D7AP)
process_d7aactp_callback_t process_d7aactp_callback;

static int execute_d7a_action_protocol(uint8_t action_file_id, uint8_t interface_file_id)
{
  // TODO interface_file_id is optional, how do we code this in file header?
  // for now we assume it's always used
  if(!fs_is_file_defined(action_file_id) || !fs_is_file_defined(interface_file_id))
    return -ECHILD;
    
  uint8_t itf_cfg[MAX_ITF_CONFIG_SIZE];
  uint32_t length = MAX_ITF_CONFIG_SIZE;
  int rc = d7ap_fs_read_file(interface_file_id, 0, itf_cfg, &length, ROOT_AUTH);
  if(rc != SUCCESS)
    return rc;
  uint32_t action_len = d7ap_fs_get_file_length(action_file_id);
  if(action_len > FILE_SIZE_MAX)
    return -EFBIG;
  rc = fs_read_file(action_file_id, sizeof(d7ap_fs_file_header_t), file_buffer, action_len);
  if(rc != SUCCESS)
    return rc;
  if(process_d7aactp_callback != NULL)
  {
    process_d7aactp_callback(itf_cfg, file_buffer, action_len);
  }
  return SUCCESS;
}

void d7ap_fs_set_process_d7aactp_callback(process_d7aactp_callback_t callback)
{
  process_d7aactp_callback = callback;
}
#endif // defined(MODULE_ALP) && defined(MODULE_D7AP)

void d7ap_fs_init()
{
  //init fs with the D7A specific system files
  fs_init();

  // TODO platform specific
  // TODO set FW version

  uint8_t uid[8] = {0};
  uint8_t uid_not_set[8] = { 0 };
  d7ap_fs_read_uid(uid);

  if(memcmp(uid, uid_not_set, 8) == 0) {
    // initializing UID
    uint64_t id = hw_get_unique_id();
    uint64_t id_be = __builtin_bswap64(id);
    d7ap_fs_write_file(D7A_FILE_UID_FILE_ID, 0, (const uint8_t*)&id_be, D7A_FILE_UID_SIZE, ROOT_AUTH);
  }

  // always update firmware version file upon boot
  uint8_t firmware_version[D7A_FILE_FIRMWARE_VERSION_APP_NAME_SIZE + D7A_FILE_FIRMWARE_VERSION_GIT_SHA1_SIZE];

  firmware_version[0] = D7A_PROTOCOL_VERSION_MAJOR;
  firmware_version[1] = D7A_PROTOCOL_VERSION_MINOR;

  d7ap_fs_write_file(D7A_FILE_FIRMWARE_VERSION_FILE_ID, 0, firmware_version, 2, ROOT_AUTH);

  memcpy(firmware_version, _APP_NAME, D7A_FILE_FIRMWARE_VERSION_APP_NAME_SIZE);
  memcpy(firmware_version + D7A_FILE_FIRMWARE_VERSION_APP_NAME_SIZE, _GIT_SHA1, D7A_FILE_FIRMWARE_VERSION_GIT_SHA1_SIZE);
  d7ap_fs_write_file(D7A_FILE_FIRMWARE_VERSION_FILE_ID, 4, firmware_version, D7A_FILE_FIRMWARE_VERSION_APP_NAME_SIZE + D7A_FILE_FIRMWARE_VERSION_GIT_SHA1_SIZE, ROOT_AUTH);
}

int d7ap_fs_init_file(uint8_t file_id, const d7ap_fs_file_header_t* file_header, const uint8_t* initial_data)
{
    // init on default permanent or volatile blockdevice based on requested storage class
    fs_blockdevice_types_t bd_type = (file_header->file_properties.storage_class == FS_STORAGE_VOLATILE)
        ? FS_BLOCKDEVICE_TYPE_VOLATILE
        : FS_BLOCKDEVICE_TYPE_PERMANENT;
    return d7ap_fs_init_file_on_blockdevice(file_id, bd_type, file_header, initial_data);
}

int d7ap_fs_init_file_on_blockdevice(
    uint8_t file_id, uint8_t blockdevice_index, const d7ap_fs_file_header_t* file_header, const uint8_t* initial_data)
{
    DPRINT("FS init %i, alloc %i", file_id, file_header->allocated_length);
    
    d7ap_fs_file_header_t file_header_big_endian;
    memcpy(&file_header_big_endian, file_header, sizeof (d7ap_fs_file_header_t));
    file_header_big_endian.length = __builtin_bswap32(file_header_big_endian.length);
    file_header_big_endian.allocated_length = __builtin_bswap32(file_header_big_endian.allocated_length);
    
    memcpy(file_buffer, (uint8_t *)&file_header_big_endian, sizeof (d7ap_fs_file_header_t));
    uint32_t length = sizeof(d7ap_fs_file_header_t);
    if(initial_data != NULL) {
        length += file_header->length;
        if(length > FILE_SIZE_MAX)
          return -EFBIG;
        memcpy(file_buffer + sizeof(d7ap_fs_file_header_t), initial_data, file_header->length);
    }
       
    return fs_init_file(file_id, blockdevice_index, (const uint8_t *)file_buffer, length, sizeof(d7ap_fs_file_header_t) + file_header->allocated_length);
}

int d7ap_fs_read_file(uint8_t file_id, uint32_t offset, uint8_t* buffer, uint32_t* length, authentication_t auth)
{
  int rtc;
  d7ap_fs_file_header_t header;

  DPRINT("FS RD %i\n", file_id);

  if(!fs_is_file_defined(file_id)) return -ENOENT;

  rtc = d7ap_fs_read_file_header(file_id, &header);
  if (rtc != 0)
    return rtc;

  if(header.length < offset + *length)
  {
    if(header.length < offset)
      return -EINVAL;
    else
      *length = header.length - offset;
  }

#ifndef MODULE_D7AP_FS_DISABLE_PERMISSIONS
  if(((auth == USER_AUTH) && (!header.file_permissions.user_read)) || ((auth == GUEST_AUTH) && (!header.file_permissions.guest_read)))
    return -EACCES;
#endif

  rtc = fs_read_file(file_id, sizeof(d7ap_fs_file_header_t) + offset, buffer, *length);
  if (rtc != 0)
    return rtc;

#if defined(MODULE_ALP) && defined(MODULE_D7AP)
  if(header.file_properties.action_protocol_enabled == true
     && header.file_properties.action_condition == D7A_ACT_COND_READ)
  {
    rtc = execute_d7a_action_protocol(header.action_file_id, header.interface_file_id);
    if(rtc != SUCCESS)
      return rtc;
  }
#endif // defined(MODULE_ALP) && defined(MODULE_D7AP)

  return 0;
}

int d7ap_fs_read_file_header(uint8_t file_id, d7ap_fs_file_header_t* file_header)
{
  int rtc;
  if(!fs_is_file_defined(file_id)) return -ENOENT;

  rtc = fs_read_file(file_id, 0, (uint8_t *)file_header, sizeof(d7ap_fs_file_header_t));
  if (rtc != 0)
    return rtc;

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
  // convert to little endian (native)
  file_header->length = __builtin_bswap32(file_header->length);
  file_header->allocated_length = __builtin_bswap32(file_header->allocated_length);
#endif

  return 0;
}

int d7ap_fs_write_file_header(uint8_t file_id, d7ap_fs_file_header_t* file_header, authentication_t auth)
{
  d7ap_fs_file_header_t header;

  if(!fs_is_file_defined(file_id)) return -ENOENT;

  d7ap_fs_read_file_header(file_id, &header);

#ifndef MODULE_D7AP_FS_DISABLE_PERMISSIONS
  if(((auth == USER_AUTH) && (!header.file_permissions.user_write)) || ((auth == GUEST_AUTH) && (!header.file_permissions.guest_write)))
    return -EACCES;
#endif

  // Input of data shall be in big-endian ordering
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
  file_header->length = __builtin_bswap32(file_header->length);
  file_header->allocated_length = __builtin_bswap32(file_header->allocated_length);
#endif

  return (fs_write_file(file_id, 0, (const uint8_t*)file_header, sizeof(d7ap_fs_file_header_t)));
}

int d7ap_fs_write_file(uint8_t file_id, uint32_t offset, const uint8_t* buffer, uint32_t length, authentication_t auth)
{
    return d7ap_fs_write_file_with_callback(file_id, offset, buffer, length, auth, true);
}

int d7ap_fs_write_file_with_callback(uint8_t file_id, uint32_t offset, const uint8_t* buffer, uint32_t length, authentication_t auth, bool trigger_modified_cb)
{
  int rtc;
  d7ap_fs_file_header_t header;

  DPRINT("FS WR %i\n", file_id);

  if(!fs_is_file_defined(file_id)) return -ENOENT;

  rtc = d7ap_fs_read_file_header(file_id, &header);
  if (rtc != 0)
    return rtc;

  if(header.length < offset + length)
    return -EINVAL;
  
#ifndef MODULE_D7AP_FS_DISABLE_PERMISSIONS
  if(((auth == USER_AUTH) && (!header.file_permissions.user_write)) || ((auth == GUEST_AUTH) && (!header.file_permissions.guest_write)))
    return -EACCES;
#endif
    
  if (file_modifying_callbacks[file_id])
      if (!file_modifying_callbacks[file_id](file_id, offset, buffer, length))
          return -EILSEQ;

  rtc = fs_write_file(file_id, sizeof(d7ap_fs_file_header_t) + offset, buffer, length);
  if (rtc != 0)
    return rtc;


#if defined(MODULE_ALP) && defined(MODULE_D7AP)
  if(header.file_properties.action_protocol_enabled == true
    && header.file_properties.action_condition == D7A_ACT_COND_WRITE) // TODO ALP_ACT_COND_WRITEFLUSH?
  {
    rtc = execute_d7a_action_protocol(header.action_file_id, header.interface_file_id);
    if(rtc != SUCCESS)
      return rtc;
  }
#endif // defined(MODULE_ALP) && defined(MODULE_D7AP)

  if (file_modified_callbacks[file_id] && trigger_modified_cb)
      file_modified_callbacks[file_id](file_id);

  return 0;
}

int d7ap_fs_update_permissions(uint8_t file_id, bool guest_read, bool guest_write, bool user_read, bool user_write)
{
    d7ap_fs_file_header_t file_header;
    error_t err = d7ap_fs_read_file_header(file_id, &file_header);
    if(err)
        return err;
    file_header.file_permissions.guest_read = guest_read;
    file_header.file_permissions.guest_write = guest_write;
    file_header.file_permissions.user_read  = user_read;
    file_header.file_permissions.user_write  = user_write;
    return d7ap_fs_write_file_header(file_id, &file_header, ROOT_AUTH);
}

int d7ap_fs_read_uid(uint8_t *buffer)
{
    uint32_t length = D7A_FILE_UID_SIZE;
    return (d7ap_fs_read_file(D7A_FILE_UID_FILE_ID, 0, buffer, &length, ROOT_AUTH));
}

int d7ap_fs_read_nwl_security_key(uint8_t *buffer)
{
    uint32_t length = D7A_FILE_NWL_SECURITY_KEY_SIZE;
    return d7ap_fs_read_file(D7A_FILE_NWL_SECURITY_KEY, 0, buffer, &length, ROOT_AUTH);
}

int d7ap_fs_read_nwl_security(dae_nwl_security_t *nwl_security)
{
  int rtc;
  uint32_t length = D7A_FILE_NWL_SECURITY_SIZE;

  rtc = d7ap_fs_read_file(D7A_FILE_NWL_SECURITY, 0, (uint8_t*)nwl_security, &length, ROOT_AUTH);
  if (rtc == 0)
    nwl_security->frame_counter = (uint32_t)__builtin_bswap32(nwl_security->frame_counter); // correct endianess

  return rtc;
}

int d7ap_fs_write_nwl_security(dae_nwl_security_t *nwl_security)
{
  if(!fs_is_file_defined(D7A_FILE_NWL_SECURITY)) return -ENOENT;

  dae_nwl_security_t sec;
  memcpy(&sec, nwl_security, sizeof (sec));
  sec.frame_counter = (uint32_t)__builtin_bswap32(nwl_security->frame_counter); // correct endianess
  return (d7ap_fs_write_file(D7A_FILE_NWL_SECURITY, 0, (uint8_t*)&sec, D7A_FILE_NWL_SECURITY_SIZE, ROOT_AUTH));
}

int d7ap_fs_read_nwl_security_state_register(dae_nwl_ssr_t *node_security_state)
{
  int rtc;
  uint32_t length = sizeof(dae_nwl_ssr_t);

  if(!fs_is_file_defined(D7A_FILE_NWL_SECURITY_STATE_REG)) return -ENOENT;

  rtc = (d7ap_fs_read_file(D7A_FILE_NWL_SECURITY_STATE_REG, 0,
                           (uint8_t*)node_security_state, &length, ROOT_AUTH));
  if (rtc != 0)
    return rtc;

   // correct endiannes
  for(uint8_t i = 0; i < node_security_state->trusted_node_nb; i++)
  {
    node_security_state->trusted_node_table[i].frame_counter = (uint32_t)__builtin_bswap32(node_security_state->trusted_node_table[i].frame_counter);
  }

  return rtc;
}

static int write_security_state_register_entry(dae_nwl_trusted_node_t *trusted_node, uint8_t trusted_node_nb)
{
  assert(trusted_node_nb <= FRAMEWORK_FS_TRUSTED_NODE_TABLE_SIZE);

  uint16_t entry_offset = (D7A_FILE_NWL_SECURITY_SIZE + D7A_FILE_UID_SIZE)*(trusted_node_nb - 1) + 2;
  // correct endiannes before writing
  dae_nwl_trusted_node_t node;
  memcpy(&node, trusted_node, sizeof(dae_nwl_trusted_node_t));
  node.frame_counter = __builtin_bswap32(node.frame_counter);
  return (d7ap_fs_write_file(D7A_FILE_NWL_SECURITY, entry_offset, (uint8_t*)&node, sizeof(dae_nwl_trusted_node_t), ROOT_AUTH));
}

int d7ap_fs_add_nwl_security_state_register_entry(dae_nwl_trusted_node_t *trusted_node,
                                                  uint8_t trusted_node_nb)
{
  // TODO test
  assert(trusted_node_nb <= FRAMEWORK_FS_TRUSTED_NODE_TABLE_SIZE);
  if(!fs_is_file_defined(D7A_FILE_NWL_SECURITY_STATE_REG)) return -ENOENT;

  // first add the new entry ...
  write_security_state_register_entry(trusted_node, trusted_node_nb);
  // ... and finally update the node count
  return (d7ap_fs_write_file(D7A_FILE_NWL_SECURITY, 1, &trusted_node_nb, 1, ROOT_AUTH));
}

int d7ap_fs_update_nwl_security_state_register(dae_nwl_trusted_node_t *trusted_node,
                                               uint8_t trusted_node_index)
{
  if(!fs_is_file_defined(D7A_FILE_NWL_SECURITY_STATE_REG)) return -ENOENT;

  return (write_security_state_register_entry(trusted_node, trusted_node_index));
}

int d7ap_fs_read_access_class(uint8_t access_class_index, dae_access_profile_t *access_class)
{
  uint32_t length = D7A_FILE_ACCESS_PROFILE_SIZE;

  if(access_class_index >= 15)
    return -EFAULT;
  if(!fs_is_file_defined(D7A_FILE_ACCESS_PROFILE_ID + access_class_index))
    return -ENOENT;
  int result = d7ap_fs_read_file(D7A_FILE_ACCESS_PROFILE_ID + access_class_index, 0, (uint8_t*)access_class, &length, ROOT_AUTH);
  for(int i=0; i<SUBBANDS_NB; i++) {
    access_class->subbands[i].channel_index_start = __builtin_bswap16(access_class->subbands[i].channel_index_start);
    access_class->subbands[i].channel_index_end = __builtin_bswap16(access_class->subbands[i].channel_index_end);
  }
  return result;
}

int d7ap_fs_write_access_class(uint8_t access_class_index, dae_access_profile_t* access_class)
{
  if(access_class_index >= 15)
    return -EFAULT;
  if(!fs_is_file_defined(D7A_FILE_ACCESS_PROFILE_ID + access_class_index))
    return -ENOENT;
  dae_access_profile_t temp_access_class;
  memcpy(&temp_access_class, access_class, sizeof(dae_access_profile_t));
  for(int i=0; i<SUBBANDS_NB; i++) {
    temp_access_class.subbands[i].channel_index_start = __builtin_bswap16(temp_access_class.subbands[i].channel_index_start);
    temp_access_class.subbands[i].channel_index_end = __builtin_bswap16(temp_access_class.subbands[i].channel_index_end);
  }
  return d7ap_fs_write_file(D7A_FILE_ACCESS_PROFILE_ID + access_class_index, 0, (uint8_t*)&temp_access_class, D7A_FILE_ACCESS_PROFILE_SIZE, ROOT_AUTH);
}

uint8_t d7ap_fs_read_dll_conf_active_access_class()
{
  uint8_t access_class;
  uint32_t length = 1;
  d7ap_fs_read_file(D7A_FILE_DLL_CONF_FILE_ID, 0, &access_class, &length, ROOT_AUTH);
  return access_class;
}

int d7ap_fs_write_dll_conf_active_access_class(uint8_t access_class)
{
  return (d7ap_fs_write_file(D7A_FILE_DLL_CONF_FILE_ID, 0, &access_class, 1, ROOT_AUTH));
}

uint32_t d7ap_fs_get_file_length(uint8_t file_id)
{
  d7ap_fs_file_header_t header;

  d7ap_fs_read_file_header(file_id, &header);
  return header.length;
}

int d7ap_fs_change_file_length(uint8_t file_id, uint32_t length)
{
  int error;
  d7ap_fs_file_header_t header;

  error = d7ap_fs_read_file_header(file_id, &header);
  if(error != 0)
      return error;
  
  header.length = length;

  return d7ap_fs_write_file_header(file_id, &header, ROOT_AUTH);
}

bool d7ap_fs_unregister_file_modified_callback(uint8_t file_id) {
    if(file_modified_callbacks[file_id]) {
        file_modified_callbacks[file_id] = NULL;
        return true;
    } else
        return false;
}

bool d7ap_fs_register_file_modified_callback(uint8_t file_id, d7ap_fs_modified_file_callback_t callback)
{
    if(!fs_is_file_defined(file_id))
        return false;

    if(file_modified_callbacks[file_id])
        return false; // already registered

    file_modified_callbacks[file_id] = callback;
    return true;
}

bool d7ap_fs_unregister_file_modifying_callback(uint8_t file_id) {
    if(file_modifying_callbacks[file_id]) {
        file_modifying_callbacks[file_id] = NULL;
        return true;
    } else
        return false;
}

bool d7ap_fs_register_file_modifying_callback(uint8_t file_id, d7ap_fs_modifying_file_callback_t callback)
{
    if(!fs_is_file_defined(file_id))
        return false;

    if(file_modifying_callbacks[file_id])
        return false; // already registered

    file_modifying_callbacks[file_id] = callback;
    return true;
}
