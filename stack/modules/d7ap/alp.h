/*! \file alp.h
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

#ifndef ALP_H_
#define ALP_H_

#include "stdint.h"
#include "stdbool.h"

#include "fifo.h"

#include "d7asp.h"

#define ALP_ITF_ID_D7ASP  0xD7
#define ALP_ITF_ID_FS     0x00 // not part of the spec
#define ALP_ITF_ID_APP    0x01 // not part of the spec

#define ALP_PAYLOAD_MAX_SIZE 128 // TODO configurable?

typedef enum
{
    ALP_CMD_ORIGIN_APP,
    ALP_CMD_ORIGIN_SERIAL_CONSOLE,
    ALP_CMD_ORIGIN_D7AACTP,
    ALP_CMD_ORIGIN_D7ASP,
} alp_command_origin_t;

typedef enum
{
    ALP_ACT_COND_LIST = 0,
    ALP_ACT_COND_READ = 1,
    ALP_ACT_COND_WRITE = 2,
    ALP_ACT_COND_WRITEFLUSH = 3
} alp_act_condition_t;

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
    ALP_OP_RETURN_STATUS = 34,
    ALP_OP_RETURN_TAG = 35,
    ALP_OP_CHUNK = 48,
    ALP_OP_LOGIC = 49,
    ALP_OP_FORWARD = 50,
    ALP_OP_REQUEST_TAG = 52
} alp_operation_t;

typedef enum {
  ALP_STATUS_OK = 0x00,
  ALP_STATUS_PARTIALLY_COMPLETED = 0x01,
  ALP_STATUS_UNKNOWN_ERROR = 0x80,
  ALP_STATUS_UNKNOWN_OPERATION = 0xF6,
  ALP_STATUS_INSUFFICIENT_PERMISSIONS = 0xFC,
  // TODO others
  ALP_STATUS_FILE_ID_ALREADY_EXISTS = 0xFE,
  ALP_STATUS_FILE_ID_NOT_EXISTS = 0xFF,
} alp_status_codes_t;


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
    uint8_t offset; // TODO can be 1-4 bytes, assuming 1 byte for now
} alp_operand_file_offset_t;

typedef struct {
    alp_operand_file_offset_t file_offset;
    uint8_t requested_data_length;
} alp_operand_file_data_request_t;

typedef struct {
    alp_operand_file_offset_t file_offset;
    uint8_t provided_data_length;
    // data
} alp_operand_file_data_t;


typedef struct {
  uint8_t fifo_token;
  uint8_t tag_id;
  bool respond_when_completed;
  alp_command_origin_t origin;
  fifo_t alp_command_fifo;
  fifo_t alp_response_fifo;
  uint8_t alp_command[ALP_PAYLOAD_MAX_SIZE];
  uint8_t alp_response[ALP_PAYLOAD_MAX_SIZE];
} alp_command_t;

/*!
 * \brief Returns the ALP operation type contained in alp_command
 * \param alp_command
 * \return the ALP operation type
 */
alp_operation_t alp_get_operation(uint8_t* alp_command);

/*!
 * \brief Process the ALP command.
 * Processing will be done against the local host interface unless explicitely forwarded to another interface using an (indirect) forward action.
 *
 * Note: alp_command and alp_response may point to the same buffer
 * \param alp_command   The raw command
 * \param alp_command_length The length of the command
 * \param alp_response Pointer to a buffer where a possible response will be written
 * \param alp_response_length The length of the response
 * \param origin Where the ALP command originates from, determines where response will go to
 * \return If the ALP command was processed correctly or not
 */
bool alp_process_command(uint8_t* alp_command, uint8_t alp_command_length, uint8_t* alp_response, uint8_t* alp_response_length, alp_command_origin_t origin);

/*!
 * \brief Process the ALP command on the local host interface and output the response to the D7ASP interface
 *
 * \param d7asp_fifo_config The config of the D7ASP fifo to output the ALP response to
 * \param alp_command   The raw command
 * \param alp_command_length The length of the command
 * \param origin Where the ALP command originates from, determines where response will go to
 */
void alp_process_command_result_on_d7asp(d7asp_master_session_config_t* d7asp_fifo_config, uint8_t* alp_command, uint8_t alp_command_length, alp_command_origin_t origin);

/*!
 * \brief Process the ALP command and output the result on the console.
 * Processing will be done against the local host interface unless explicitely forwarded to another interface using an (indirect) forward action.
 *
 * Note: alp_command and alp_response may point to the same buffer
 * \param alp_command   The raw command
 * \param alp_command_length The length of the command
 */
void alp_process_command_console_output(uint8_t* alp_command, uint8_t alp_command_length);

void alp_d7asp_request_completed(d7asp_result_t result, uint8_t* payload, uint8_t payload_length);

void alp_d7asp_fifo_flush_completed(uint8_t fifo_token, uint8_t* progress_bitmap, uint8_t* success_bitmap, uint8_t bitmap_byte_count);

#endif /* ALP_H_ */
