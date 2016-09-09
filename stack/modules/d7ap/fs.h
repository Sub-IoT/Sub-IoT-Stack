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
 * \author glenn.ergeerts@uantwerpen.be
 * \author maarten.weyn@uantwerpen.be
 *
 */

#ifndef FS_H_
#define FS_H_

#include "stdint.h"

#include "dae.h"
#include "alp.h"

#define D7A_FILE_UID_FILE_ID 0x00
#define D7A_FILE_UID_SIZE 8

#define D7A_FILE_FIRMWARE_VERSION_FILE_ID 0x02
#define D7A_FILE_FIRMWARE_VERSION_APP_NAME_SIZE 6
#define D7A_FILE_FIRMWARE_VERSION_GIT_SHA1_SIZE 7
#define D7A_FILE_FIRMWARE_VERSION_SIZE (D7A_FILE_FIRMWARE_VERSION_APP_NAME_SIZE + D7A_FILE_FIRMWARE_VERSION_GIT_SHA1_SIZE)

#define D7A_FILE_DLL_CONF_FILE_ID	0x0A
#define D7A_FILE_DLL_CONF_SIZE		6

#define D7A_FILE_ACCESS_PROFILE_ID 0x20 // the first access class file
#define D7A_FILE_ACCESS_PROFILE_SIZE 12 // TODO assuming 1 subband

typedef enum
{
    FS_STORAGE_TRANSIENT = 0,
    FS_STORAGE_VOLATILE = 1,
    FS_STORAGE_RESTORABLE = 2,
    FS_STORAGE_PERMANENT = 3
} fs_storage_class_t;

typedef struct
{
    uint8_t action_file_id;
    union
    {
        uint8_t _flags;
        struct
        {
            bool action_protocol_enabled : 1;
            alp_act_condition_t action_condition : 3;
            uint8_t _rfu : 2;
            fs_storage_class_t storage_class : 2; // TODO
        };
        // TODO save to use bitfields here?
    };
    uint8_t permissions;
} fs_file_properties_t;

typedef struct
{
    fs_file_properties_t file_properties;
    uint32_t length;
    // TODO not used for now uint32_t allocated_length;
} fs_file_header_t;

/**
 * \brief Initialize the user files in this callback.
 *
 * The application should call fs_init_file() for all user files so the stack can
 * register these files in the filesystem
 */
typedef void (*fs_user_files_init_callback)(void);


/**
 * \brief Arguments used by the stack for filesystem initialization
 */
typedef struct {
    fs_user_files_init_callback fs_user_files_init_cb; /**< Initialize the user files in this callback */
    uint8_t access_profiles_count; /**< The number of access profiles used (and passed in the access_profiles member).  */
    dae_access_profile_t* access_profiles; /**< The access profiles to be written to the filesystem (using increasing fileID starting from0x20) during init.  */    
} fs_init_args_t;

void fs_init(fs_init_args_t* init_args);
void fs_init_file(uint8_t file_id, const fs_file_header_t* file_header, const uint8_t* initial_data);
void fs_init_file_with_D7AActP(uint8_t file_id, const d7asp_master_session_config_t* fifo_config, const alp_control_t* alp_control, const uint8_t* alp_operand);
alp_status_codes_t fs_read_file(uint8_t file_id, uint8_t offset, uint8_t* buffer, uint8_t length);
alp_status_codes_t fs_write_file(uint8_t file_id, uint8_t offset, const uint8_t* buffer, uint8_t length);
void fs_read_access_class(uint8_t access_class_index, dae_access_profile_t* access_class);
void fs_read_uid(uint8_t* buffer);
void fs_read_vid(uint8_t* buffer);
void fs_write_vid(uint8_t* buffer);
uint8_t fs_read_dll_conf_active_access_class();
void fs_write_dll_conf_active_access_class(uint8_t access_class);
uint8_t fs_get_file_length(uint8_t file_id);

#endif /* FS_H_ */
