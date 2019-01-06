/*! \file fs.h
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
 */

/*! \file fs.h
 * \addtogroup Fs
 * \ingroup D7AP
 * @{
 * \brief Filesystem APIs
 * \author maarten.weyn@uantwerpen.be
 * \author	glenn.ergeerts@uantwerpen.be
 * \author	philippe.nunes@cortus.com
 */

#ifndef FS_H_
#define FS_H_

#include "stdint.h"

#include "dae.h"
#include "alp.h"
#include "d7ap.h"
#include "hwblockdevice.h"

/**
 * \brief Initialize the user files in this callback.
 *
 * The application should call fs_init_file() for all user files so the stack can
 * register these files in the filesystem
 */
typedef void (*fs_user_files_init_callback)(void);


/* \brief The callback function for when a user file is modified
 *
 * \param file_id		The id of the modified user file
 * **/
typedef void (*fs_modified_file_callback_t)(uint8_t file_id);

/**
 * \brief Callback function executed when D7AActP is triggered
 */
typedef void (*fs_d7aactp_callback_t)(d7ap_session_config_t* session_config, uint8_t* alp_command, uint8_t alp_command_length);

/**
 * \brief Arguments used by the stack for filesystem initialization
 */
typedef struct {
    fs_user_files_init_callback fs_user_files_init_cb; /**< Initialize the user files in this callback */
    fs_d7aactp_callback_t fs_d7aactp_cb; /**< Callback function executed when D7AActP is triggered */
    uint8_t access_profiles_count; /**< The number of access profiles passed in the access_profiles member.  */
    dae_access_profile_t* access_profiles; /**< The access profiles to be written to the filesystem (using increasing fileID starting from0x20) during init.  */    
    uint8_t access_class; /* The Active Access Class to be written in the DLL configuration file */
    uint8_t ssr_filter_mode; /* Initialise the SSR filter mode used to maintain the SSR */
} fs_init_args_t; // TODO


// externs defined in d7ap_fs_data.c
extern const uint8_t fs_systemfiles_header_data[];
extern const uint16_t fs_systemfiles_file_offsets[];
extern const uint8_t fs_systemfiles_file_data[];

void d7ap_fs_init(blockdevice_t* blockdevice_systemfiles);
void fs_init_file(uint8_t file_id, const fs_file_header_t* file_header, const uint8_t* initial_data);
void fs_init_file_with_D7AActP(uint8_t file_id, const d7ap_session_config_t* fifo_config, const uint8_t* alp_command, const uint8_t alp_command_len);
void fs_init_file_with_d7asp_interface_config(uint8_t file_id, const d7ap_session_config_t* fifo_config);
alp_status_codes_t d7ap_fs_read_file(uint8_t file_id, uint16_t offset, uint8_t* buffer, uint16_t length);
alp_status_codes_t d7ap_fs_write_file(uint8_t file_id, uint16_t offset, const uint8_t* buffer, uint16_t length);
void d7ap_fs_read_access_class(uint8_t access_class_index, dae_access_profile_t* access_class);
void d7ap_fs_write_access_class(uint8_t access_class_index, dae_access_profile_t* access_class);
alp_status_codes_t d7ap_fs_read_file_header(uint8_t file_id, fs_file_header_t* file_header);
alp_status_codes_t d7ap_fs_write_file_header(uint8_t file_id, fs_file_header_t* file_header);
void d7ap_fs_read_uid(uint8_t* buffer);
void d7ap_fs_read_vid(uint8_t* buffer);
void d7ap_fs_write_vid(uint8_t* buffer);
uint8_t d7ap_fs_read_dll_conf_active_access_class();
void d7ap_fs_write_dll_conf_active_access_class(uint8_t access_class);
alp_status_codes_t d7ap_fs_read_nwl_security_key(uint8_t* key);
alp_status_codes_t d7ap_fs_read_nwl_security(dae_nwl_security_t *nwl_security);
alp_status_codes_t d7ap_fs_write_nwl_security(dae_nwl_security_t *nwl_security);
alp_status_codes_t d7ap_fs_read_nwl_security_state_register(dae_nwl_ssr_t *node_security_state);
alp_status_codes_t d7ap_fs_add_nwl_security_state_register_entry(dae_nwl_trusted_node_t *trusted_node, uint8_t trusted_node_nb);
alp_status_codes_t d7ap_fs_update_nwl_security_state_register(dae_nwl_trusted_node_t *trusted_node, uint8_t trusted_node_index);
uint8_t d7ap_fs_get_file_length(uint8_t file_id);

bool d7ap_fs_register_file_modified_callback(uint8_t file_id, fs_modified_file_callback_t callback);


#endif /* FS_H_ */

/** @}*/
