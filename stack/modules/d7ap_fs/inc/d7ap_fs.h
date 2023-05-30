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

/*! \file d7ap_fs.h
 * \addtogroup Fs
 * \ingroup D7AP
 * @{
 * \brief Filesystem APIs
 * \author maarten.weyn@uantwerpen.be
 * \author	glenn.ergeerts@aloxy.be
 * \author	philippe.nunes@cortus.com
 */

#ifndef D7AP_FS_H_
#define D7AP_FS_H_

#include "stdint.h"

#include "dae.h"
#include "modules_defs.h"

#define D7A_FILE_UID_FILE_ID 0x00
#define D7A_FILE_UID_SIZE 8

#define D7A_FILE_FIRMWARE_VERSION_FILE_ID 0x02
#define D7A_FILE_FIRMWARE_VERSION_APP_NAME_SIZE 6
#define D7A_FILE_FIRMWARE_VERSION_GIT_SHA1_SIZE 7
#define D7A_FILE_FIRMWARE_VERSION_SIZE (2 + 2 + D7A_FILE_FIRMWARE_VERSION_APP_NAME_SIZE + D7A_FILE_FIRMWARE_VERSION_GIT_SHA1_SIZE)

#define D7A_FILE_FACTORY_SETTINGS_FILE_ID 0x01
#define D7A_FILE_FACTORY_SETTINGS_SIZE 53

#define D7A_FILE_ENGINEERING_MODE_FILE_ID 0x05
#define D7A_FILE_ENGINEERING_MODE_SIZE  9

#define D7A_FILE_VID_FILE_ID 0x06
#define D7A_FILE_VID_SIZE 3

#define D7A_FILE_PHY_STATUS_FILE_ID 0x09
#define D7A_FILE_PHY_STATUS_MINIMUM_SIZE 15
#define D7A_FILE_PHY_STATUS_SIZE 45 // maximum size for now is 45 which supports 10 channels
#define D7A_FILE_PHY_STATUS_CHANNEL_COUNT_SIZE 1
#define D7A_FILE_PHY_STATUS_CHANNEL_COUNT 10 
#define D7A_FILE_PHY_STATUS_CHANNEL_SIZE 3

#define D7A_FILE_DLL_CONF_FILE_ID	0x0A
#define D7A_FILE_DLL_CONF_SIZE		7
#define D7A_FILE_DLL_CONF_NF_CTRL_OFFSET 4
#define D7A_FILE_DLL_CONF_NF_CTRL_SIZE 1

#define D7A_FILE_DLL_STATUS_FILE_ID 0x0B
#define D7A_FILE_DLL_STATUS_SIZE    12

#define D7A_FILE_SEL_CONF_FILE_ID 0x12
#define D7A_FILE_SEL_CONF_SIZE    6
#define D7A_FILE_SEL_CONF_SEGMENT_FILTER_OFFSET 5
#define D7A_FILE_SEL_CONF_SEGMENT_FILTER_SIZE 1

#define D7A_FILE_ACCESS_PROFILE_ID 0x20 // the first access class file
#define D7A_FILE_ACCESS_PROFILE_SIZE 65
#define D7A_FILE_ACCESS_PROFILE_COUNT 15
#define D7A_FILE_ACCESS_PROFILE_CHANNEL_HEADER_IDX 0

#define D7A_FILE_NWL_SECURITY		0x0D
#define D7A_FILE_NWL_SECURITY_SIZE	5

#define D7A_FILE_NWL_SECURITY_KEY		0x0E
#define D7A_FILE_NWL_SECURITY_KEY_SIZE	16

#define D7A_FILE_NWL_SECURITY_STATE_REG			0x0F
#define D7A_FILE_NWL_SECURITY_STATE_REG_SIZE	2 + (FRAMEWORK_FS_TRUSTED_NODE_TABLE_SIZE)*(D7A_FILE_NWL_SECURITY_SIZE + D7A_FILE_UID_SIZE)

#define D7AP_FS_SYSTEMFILES_COUNT 0x2F // reserved up until 0x3F but used only until 0x2F so use this for limiting memory usage
#define D7AP_FS_USERFILES_COUNT (FRAMEWORK_FS_FILE_COUNT - D7AP_FS_SYSTEMFILES_COUNT)

#define USER_FILE_ALP_CTRL_FILE_ID 0x40
#define USER_FILE_ALP_CTRL_SIZE    2

#define USER_FILE_LORAWAN_KEYS_FILE_ID           0x41
#define USER_FILE_LORAWAN_KEYS_SIZE              24
#define USER_FILE_LORAWAN_KEYS_ALLOCATED_SIZE    40

#define USER_FILE_LORAWAN_ANTENNA_GAIN_FILE_ID 0x46
#define USER_FILE_LORAWAN_ANTENNA_GAIN_SIZE 1

#define MAX_ITF_CONFIG_SIZE 44

#define USER_FILE_LORAWAN_DEVNONCE_FILE_ID           0x47
#define USER_FILE_LORAWAN_DEVNONCE_SIZE              2

typedef enum {
  EM_OFF = 0,
  EM_CONTINUOUS_TX = 1,
  EM_TRANSIENT_TX = 2,
  EM_PER_RX = 3,
  EM_PER_TX = 4,
  EM_CONTINUOUS_STANDBY = 5
} engineering_mode_t;

/* \brief The callback function for when a user file is modified
 *
 * \param file_id		The id of the modified user file
 * **/
typedef void (*d7ap_fs_modified_file_callback_t)(uint8_t file_id);


/* \brief The callback function for when a file is going to be modified, mostly used to validate the data
 *
 * \param file_id		The id of the modifying file
 * \param offset    the offset of the data compared to the d7 file start
 * \param buffer    the buffer which will be written at the given offset with the given length
 * \param length    the length of the data which will be written to the file
 * \return bool     this will decide whether the file write will go through (true) or will get cancelled (false)
 * **/
typedef bool (*d7ap_fs_modifying_file_callback_t)(uint8_t file_id, uint32_t offset, const uint8_t* buffer, uint32_t length);

bool d7ap_fs_register_file_modified_callback(uint8_t file_id, d7ap_fs_modified_file_callback_t callback);
bool d7ap_fs_unregister_file_modified_callback(uint8_t file_id);
bool d7ap_fs_register_file_modifying_callback(uint8_t file_id, d7ap_fs_modifying_file_callback_t callback);
bool d7ap_fs_unregister_file_modifying_callback(uint8_t file_id);

void d7ap_fs_init();
int d7ap_fs_init_file(uint8_t file_id, const d7ap_fs_file_header_t* file_header, const uint8_t* initial_data);
int d7ap_fs_init_file_on_blockdevice(uint8_t file_id, uint8_t blockdevice_index, const d7ap_fs_file_header_t* file_header, const uint8_t* initial_data);

int d7ap_fs_read_file(uint8_t file_id, uint32_t offset, uint8_t* buffer, uint32_t* length, authentication_t auth);
int d7ap_fs_write_file(uint8_t file_id, uint32_t offset, const uint8_t* buffer, uint32_t length, authentication_t auth);
int d7ap_fs_write_file_with_callback(uint8_t file_id, uint32_t offset, const uint8_t* buffer, uint32_t length, authentication_t auth, bool trigger_cb);

int d7ap_fs_read_access_class(uint8_t access_class_index, dae_access_profile_t* access_class);
int d7ap_fs_write_access_class(uint8_t access_class_index, dae_access_profile_t* access_class);

int d7ap_fs_read_file_header(uint8_t file_id, d7ap_fs_file_header_t* file_header);
int d7ap_fs_write_file_header(uint8_t file_id, d7ap_fs_file_header_t* file_header, authentication_t auth);

int d7ap_fs_read_uid(uint8_t* buffer);
int d7ap_fs_write_vid(uint8_t* buffer);

uint8_t d7ap_fs_read_dll_conf_active_access_class(void);
int d7ap_fs_write_dll_conf_active_access_class(uint8_t access_class);

int d7ap_fs_update_permissions(uint8_t file_id, bool guest_read, bool guest_write, bool user_read, bool user_write);

int d7ap_fs_read_nwl_security_key(uint8_t* key);
int d7ap_fs_read_nwl_security(dae_nwl_security_t *nwl_security);
int d7ap_fs_write_nwl_security(dae_nwl_security_t *nwl_security);
int d7ap_fs_read_nwl_security_state_register(dae_nwl_ssr_t *node_security_state);
int d7ap_fs_add_nwl_security_state_register_entry(dae_nwl_trusted_node_t *trusted_node, uint8_t trusted_node_nb);
int d7ap_fs_update_nwl_security_state_register(dae_nwl_trusted_node_t *trusted_node, uint8_t trusted_node_index);

uint32_t d7ap_fs_get_file_length(uint8_t file_id);
int d7ap_fs_change_file_length(uint8_t file_id, uint32_t length);

#if defined(MODULE_ALP) && defined(MODULE_D7AP)

typedef void (*process_d7aactp_callback_t)(uint8_t* interface_config, uint8_t* command, uint32_t command_length);

void d7ap_fs_set_process_d7aactp_callback(process_d7aactp_callback_t callback);

#endif // defined(MODULE_ALP) && defined(MODULE_D7AP)

#endif /* D7AP_FS_H_ */

/** @}*/
