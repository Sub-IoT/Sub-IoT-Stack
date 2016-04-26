/*! \file alp.c
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

#include "debug.h"
#include "ng.h"

#include "alp.h"
#include "packet.h"
#include "fs.h"

alp_operation_t alp_get_operation(uint8_t* alp_command)
{
    alp_control_t alp_ctrl;
    alp_ctrl.raw = (*alp_command);
    return alp_ctrl.operation;
}

bool alp_process_command_fs_itf(uint8_t* alp_command, uint8_t alp_command_length, uint8_t* alp_response, uint8_t* alp_response_length)
{
    (*alp_response_length) = 0;
    if(alp_command_length ==0)
      return;

    // TODO check response length
    alp_control_t alp_control = { .raw = (*alp_command) }; alp_command++;

    alp_status_codes_t alp_status;

    switch(alp_control.operation)
    {
        case ALP_OP_READ_FILE_DATA:
        {
            alp_operand_file_data_request_t operand;
            operand.file_offset.file_id = (*alp_command); alp_command++;
            operand.file_offset.offset = (*alp_command); alp_command++; // TODO can be 1-4 bytes, assume 1 for now
            operand.requested_data_length = (*alp_command); alp_command++;

            // fill response
            (*alp_response_length) = 0;
            alp_response[(*alp_response_length)] = ALP_OP_RETURN_FILE_DATA; (*alp_response_length)++;
            alp_response[(*alp_response_length)] = operand.file_offset.file_id; (*alp_response_length)++;
            alp_response[(*alp_response_length)] = operand.file_offset.offset; (*alp_response_length)++;
            alp_response[(*alp_response_length)] = operand.requested_data_length; (*alp_response_length)++;
            alp_status = fs_read_file(operand.file_offset.file_id, operand.file_offset.offset, alp_response + (*alp_response_length), operand.requested_data_length);
            (*alp_response_length) += operand.requested_data_length;
            break;
        }
        case ALP_OP_WRITE_FILE_DATA:
        {
            alp_operand_file_data_t operand;
            operand.file_offset.file_id = (*alp_command); alp_command++;
            operand.file_offset.offset = (*alp_command); alp_command++; // TODO can be 1-4 bytes, assume 1 for now
            operand.provided_data_length = (*alp_command); alp_command++;
            alp_status = fs_write_file(operand.file_offset.file_id, operand.file_offset.offset, alp_command, operand.provided_data_length);
            break;
        }
        default:
            alp_status = ALP_STATUS_UNKNOWN_OPERATION;

            // TODO implement other operations
    }

    // TODO multiple commands in one request

    // TODO return ALP status if requested

    if(alp_status != ALP_STATUS_OK)
      return false;

    return true;
}

