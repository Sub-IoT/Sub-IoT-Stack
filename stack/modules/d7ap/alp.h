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

#include "d7asp.h"

#define ALP_ITF_ID_D7ASP  0xD7
#define ALP_ITF_ID_FS     0x00 // not part of the spec
#define ALP_ITF_ID_APP    0x01 // not part of the spec

#define ALP_PAYLOAD_MAX_SIZE 32 // TODO configurable?

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
    ALP_OP_CHUNK = 48,
    ALP_OP_LOGIC = 49,
    ALP_OP_FORWARD = 50,
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
            bool response_requested : 1;
            bool group : 1;
        };
    };
} alp_control_t;

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

/*!
 * \brief Returns the ALP operation type contained in alp_command
 * \param alp_command
 * \return the ALP operation type
 */
alp_operation_t alp_get_operation(uint8_t* alp_command);

/*!
 * \brief Process the ALP command against the local host FS
 *
 * Note: alp_command and alp_response may point to the same buffer
 * \param alp_command   The raw command
 * \param alp_command_length The length of the command
 * \param alp_response Pointer to a buffer where a possible response will be written
 * \param alp_response_length The length of the response
 * \return If the ALP command was processed correctly or not
 */
bool alp_process_command_host(uint8_t* alp_command, uint8_t alp_command_length, uint8_t* alp_response, uint8_t* alp_response_length);

/*! \brief Process a received request and replaces the packet's payload with the response payload.
 *  ALP commands which cannot be handled by the stack are vectored to the application layer
 */
bool alp_process_received_request(d7asp_result_t d7asp_result, packet_t* packet);

#endif /* ALP_H_ */
