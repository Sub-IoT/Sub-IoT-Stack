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
 * \author glenn.ergeerts@uantwerpen.be
 * \author maarten.weyn@uantwerpen.be
 *
 */

/*! \file alp.h
 * \addtogroup ALP
 * \ingroup D7AP
 * @{
 * \brief Application Layer Protocol APIs
 * \author	glenn.ergeerts@aloxy.be
 */

#ifndef ALP_H_
#define ALP_H_

#include "stdint.h"
#include "stdbool.h"

#include "d7ap_fs.h"
#include "dae.h"

#include "fifo.h"

#include "modules_defs.h"

#define MODULE_ALP_INTERFACE_CNT 6

#define ALP_PAYLOAD_MAX_SIZE 255 // TODO configurable?
#define ALP_QUERY_COMPARE_BODY_MAX_SIZE 100
#define ALP_ITF_STATUS_MAX_SIZE 40
#define ALP_INDIRECT_ITF_OVERLOAD_MAX_SIZE 10

#define ALP_OP_CUSTOM 53

#define ALP_AUTH_KEY_FILE_LENGTH 40

typedef enum
{
    ALP_ITF_ID_HOST = 0x00,
    ALP_ITF_ID_SERIAL = 0x01, // not part of the spec
    ALP_ITF_ID_LORAWAN_OTAA = 0x03, // not part of the spec
    ALP_ITF_ID_NFC = 0x04, // not part of the spec
    ALP_ITF_ID_BLE = 0x05, // not part of the spec
    ALP_ITF_ID_D7ASP = 0xD7,
} alp_itf_id_t;

typedef enum {
    ALP_FILE_ID_ROOT_AUTH_KEY = 0x18,
    ALP_FILE_ID_USER_AUTH_KEY = 0x19,
} alp_file_id_t;


typedef enum {
    ALP_OP_NOP = 0,
    ALP_OP_READ_FILE_DATA = 1,
    ALP_OP_READ_FILE_PROPERTIES = 2,
    ALP_OP_WRITE_FILE_DATA = 4,
    ALP_OP_WRITE_FILE_DATA_FLUSH = 5,
    ALP_OP_WRITE_FILE_PROPERTIES = 6,
    ALP_OP_ACTION_QUERY = 8,
    ALP_OP_BREAK_QUERY = 9,
    ALP_OP_PERMISSION_REQUEST = 10,
    ALP_OP_VERIFY_CHECKSUM = 11,
    ALP_OP_EXIST_FILE = 16,
    ALP_OP_CREATE_FILE = 17,
    ALP_OP_DELETE_FILE = 18,
    ALP_OP_RESTORE_FILE = 19,
    ALP_OP_FLUSH_FILE = 20,
    ALP_OP_OPEN_FILE = 21,
    ALP_OP_CLOSE_FILE = 22,
    ALP_OP_COPY_FILE = 23,
    ALP_OP_EXECUTE_FILE = 31,
    ALP_OP_RETURN_FILE_DATA = 32,
    ALP_OP_RETURN_FILE_PROPERTIES = 33,
    ALP_OP_STATUS = 34,
    ALP_OP_RESPONSE_TAG = 35,
    ALP_OP_CHUNK = 48,
    ALP_OP_LOGIC = 49,
    ALP_OP_FORWARD = 50,
    ALP_OP_INDIRECT_FORWARD = 51,
    ALP_OP_REQUEST_TAG = 52,
    ALP_OP_START_ITF = ALP_OP_CUSTOM + 0,
    ALP_OP_STOP_ITF = ALP_OP_CUSTOM + 1,
} alp_operation_t;

// define the (max) size for all ALP operation types
#define ALP_OP_SIZE_REQUEST_TAG (1 + 1)
#define ALP_OP_SIZE_READ_FILE_DATA (1 + 5 + 4)

typedef enum {
  ALP_STATUS_OK = 0x00,
  ALP_STATUS_PARTIALLY_COMPLETED = 0x01,
  ALP_STATUS_FILE_ID_NOT_EXISTS = 0xFF,         //access file
  ALP_STATUS_INSUFFICIENT_PERMISSIONS = 0xFC,   //access file
  ALP_STATUS_OFFSET_OUT_OF_BOUNDS = 0xF9,       //write file
  ALP_STATUS_DATA_OVERFLOW = 0xF8,              //write file
  ALP_STATUS_WRITE_LOCATION_ERROR = 0xF7,       //write file
  ALP_STATUS_FILE_ID_ALREADY_EXISTS = 0xFE,     //create file
  ALP_STATUS_LENGTH_OUT_OF_BOUNDS = 0xFB,       //create file
  ALP_STATUS_ALLOCATION_OUT_OF_BOUNDS = 0xFA,   //create file
  ALP_STATUS_NOT_RESTORABLE = 0xFD,             //restore file
  ALP_STATUS_UNKNOWN_OPERATION = 0xF6,
  ALP_STATUS_INCOMPLETE_OPERAND = 0xF5,         
  ALP_STATUS_WRONG_OPERAND_FORMAT = 0xF4,
  ALP_STATUS_UNKNOWN_ERROR = 0x80,
  //custom alp status commands that are not in the current spec:
  ALP_STATUS_FIFO_OUT_OF_BOUNDS = 0xE0,
  ALP_STATUS_EXCEEDS_MAX_ALP_SIZE = 0xE1,
  ALP_STATUS_NOT_YET_IMPLEMENTED = 0xE2,
  ALP_STATUS_BREAK_QUERY_FAILED = 0xE3,
  ALP_STATUS_FILE_ID_OUT_OF_BOUNDS = 0xE4,
  ALP_STATUS_EMPTY_ITF_STATUS = 0xE5,
  ALP_STATUS_COMMAND_NOT_FOUND = 0xE6,
  ALP_STATUS_NO_COMMAND_LEFT = 0xE7,
  ALP_STATUS_PARSING_FAILED = 0xE8,
  ALP_STATUS_ITF_STOPPED = 0xE9,
} alp_status_codes_t;

typedef enum {
  ARITH_COMP_TYPE_INEQUALITY = 0,
  ARITH_COMP_TYPE_EQUALITY = 1,
  ARITH_COMP_TYPE_LESS_THAN = 2,
  ARITH_COMP_TYPE_LESS_THAN_OR_EQUAL_TO = 3,
  ARITH_COMP_TYPE_GREATER_THAN = 4,
  ARITH_COMP_TYPE_GREATER_THAN_OR_EQUAL_TO = 5
} alp_query_arithmetic_comparison_type_t;

typedef enum {
    ERROR_ALP = 0,
    ERROR_ALP_LAYER = 1,
} error_source_t;

/*! \brief The ALP CTRL header
 *
 * note: bit order is important here since this is send over the air. We explicitly reverse the order to ensure BE.
 * Although bit fields can cause portability problems it seems fine for now using gcc and the current platforms.
 * If this poses problems in the future we must resort to bit arithmetics and flags.
 */
typedef struct {
    union {
        uint8_t raw;
        struct {
            alp_operation_t operation : 6;
            bool b6 : 1;
            bool b7 : 1;
        };
    };
} alp_control_t;

/*! \brief The ALP CTRL header, for 'regular' operations, where b6 and b7 are overloaded with response_requested and group flags respectively
 */
typedef struct {
    union {
        uint8_t raw;
        struct {
            alp_operation_t operation : 6;
            bool response_requested : 1;
            bool group : 1;
        };
    };
} alp_control_regular_t;

/*! \brief The ALP CTRL header, for tag request operation
 */
typedef struct {
    union {
        uint8_t raw;
        struct {
            alp_operation_t operation : 6;
            bool _rfu : 1;
            bool respond_when_completed : 1;
        };
    };
} alp_control_tag_request_t;

/*! \brief The ALP CTRL header, for tag response operation
 */
typedef struct {
    union {
        uint8_t raw;
        struct {
            alp_operation_t operation : 6;
            bool error : 1;
            bool _rfu : 1;
        };
    };
} alp_control_tag_response_t;

typedef struct {
    uint8_t file_id;
} alp_operand_file_id_t;

typedef struct {
    uint8_t file_id;
    uint32_t offset;
} alp_operand_file_offset_t;

typedef struct {
    alp_operand_file_offset_t file_offset;
    uint32_t requested_data_length;
} alp_operand_file_data_request_t;

typedef struct {
    alp_operand_file_offset_t file_offset;
    uint32_t provided_data_length;
    uint8_t data[255]; // TODO fixed size?
} alp_operand_file_data_t;

typedef struct {
    uint8_t file_id;
    d7ap_fs_file_header_t file_header;
} alp_operand_file_header_t;

typedef struct __attribute__ ((__packed__)) {
    uint8_t itf_id;
    uint8_t itf_config[MAX_ITF_CONFIG_SIZE-1];
} alp_interface_config_t;

typedef struct __attribute__((packed)) {
    uint8_t itf_id;
    uint8_t len; //this length is actually a length operand (it can vary from 1 byte to 4 byte). As the ALP_ITF_STATUS_MAX_SIZE is now set to 40, it is not necessary to foresee multiple bytes for this
    uint8_t itf_status[ALP_ITF_STATUS_MAX_SIZE];
} alp_interface_status_t;

typedef enum {
    QUERY_CODE_TYPE_NON_VOID_CHECK = 0,
    QUERY_CODE_TYPE_ARITHM_COMP_WITH_ZERO = 1,
    QUERY_CODE_TYPE_ARITHM_COMP_WITH_VALUE_IN_QUERY = 2,
    QUERY_CODE_TYPE_ARITHM_COMP_WITH_FILES = 3,
    QUERY_CODE_TYPE_RANGE_COMP_WITH_BITMAP = 4,
    // 5-6 RFU
    QUERY_CODE_TYPE_STRING_TOKEN_SEARCH = 7,
} alp_query_code_type_t;

typedef struct __attribute__((packed)) {
    union {
        uint8_t raw;
        struct {
            uint8_t param : 4;
            bool mask : 1;
            alp_query_code_type_t type : 3;
        };
    } code;
    uint32_t compare_operand_length;
    uint8_t compare_body[ALP_QUERY_COMPARE_BODY_MAX_SIZE];
} alp_operand_query_t;

typedef struct __attribute__((packed)) {
    uint8_t tag_id;
} alp_operand_tag_id_t;

typedef struct __attribute__((packed)) {
    uint8_t interface_file_id;
    uint8_t overload_data[ALP_INDIRECT_ITF_OVERLOAD_MAX_SIZE];
} alp_operand_indirect_interface_t;


typedef struct {
    alp_control_t ctrl;
    union {
        alp_operand_file_data_t file_data_operand;
        alp_operand_file_data_request_t file_data_request_operand;
        alp_operand_file_id_t file_id_operand;
        alp_operand_file_header_t file_header_operand;
        alp_operand_query_t query_operand;
        struct {
            bool completed;
            bool error;
            uint8_t tag_id;
        } tag_response; // TODO merge with tag_id_operand
        alp_interface_status_t interface_status;
        alp_interface_config_t interface_config;
        alp_operand_tag_id_t tag_id_operand;
        alp_operand_indirect_interface_t indirect_interface_operand;
    };

} alp_action_t;

typedef void (*interface_deinit)();
typedef error_t (*interface_init)();

typedef struct {
    alp_itf_id_t itf_id;
    uint8_t itf_cfg_len;
    uint8_t itf_status_len;
    error_t (*send_command)(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* itf_cfg);
    interface_init init;
    interface_deinit deinit;
    bool unique; // TODO
} alp_interface_t;

// TODO internal struct, only expose opaque struct to users?
typedef struct {
    bool is_active;
    bool is_response;
    bool respond_when_completed;
    bool is_response_completed;
    bool is_response_error;
    bool is_tag_requested;
    bool is_unsollicited;
    uint8_t forward_itf_id;
    uint16_t trans_id;
    uint8_t tag_id;
    alp_interface_status_t origin_itf_status;
    alp_itf_id_t origin_itf_id; // TODO can be removed, itf id stored in origin_itf_status
#ifdef MODULE_D7AP // TODO add separate flag to disable D7AActP?
    bool use_d7aactp;
    alp_interface_config_t d7aactp_interface_config;
#endif
    fifo_t alp_command_fifo;
    uint8_t alp_command[ALP_PAYLOAD_MAX_SIZE];
} alp_command_t;

int alp_get_expected_response_length(alp_command_t* command);

alp_status_codes_t alp_register_interface(alp_interface_t* itf);
bool alp_append_tag_request_action(alp_command_t* command, uint8_t tag_id, bool eop);
bool alp_append_tag_response_action(alp_command_t* command, uint8_t tag_id, bool eop, bool err);
bool alp_append_read_file_data_action(alp_command_t* command, uint8_t file_id, uint32_t offset, uint32_t length, bool resp, bool group);
bool alp_append_write_file_data_action(alp_command_t* command, uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data, bool resp, bool group);
bool alp_append_forward_action(alp_command_t* command, alp_interface_config_t* config, uint8_t config_len);
bool alp_append_return_file_data_action(alp_command_t* command, uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data);
bool alp_fifo_append_return_file_data_action(fifo_t* cmd_fifo, uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data);
bool alp_append_length_operand(alp_command_t* command, uint32_t length);
bool alp_fifo_append_length_operand(fifo_t* cmd_fifo, uint32_t length);
bool alp_append_create_new_file_data_action(alp_command_t* command, uint8_t file_id, uint32_t length, fs_storage_class_t storage_class, bool resp, bool group);
bool alp_append_indirect_forward_action(alp_command_t* command, uint8_t file_id, bool overload, uint8_t *overload_config, uint8_t overload_config_len);
bool alp_append_interface_status(alp_command_t* command, alp_interface_status_t* status);
bool alp_append_start_itf_action(alp_command_t* command);
bool alp_append_stop_itf_action(alp_command_t* command);
bool alp_append_break_query_action(alp_command_t* command, uint8_t file_id, uint32_t offset, alp_operand_query_t* query);

bool alp_parse_action(alp_command_t* command, alp_action_t* action);
bool alp_parse_length_operand(fifo_t* cmd_fifo, uint32_t* length);
bool alp_parse_file_offset_operand(fifo_t* cmd_fifo, alp_operand_file_offset_t* operand);


uint8_t alp_length_operand_coded_length(uint32_t length);

#endif /* ALP_H_ */

/** @}*/
