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

#include "stdlib.h"
#include "debug.h"
#include "errors.h"
#include "string.h"
#include "modules_defs.h"

#include "alp.h"
#include "dae.h"
#include "fifo.h"
#include "d7ap.h"
#include "log.h"
#include "lorawan_stack.h"


#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_ALP_LOG_ENABLED)
  #define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
  #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINT_DATA(...)
#endif

alp_interface_t* interfaces[MODULE_ALP_INTERFACE_SIZE];

alp_status_codes_t alp_handle_error(int rc, alp_operation_t operation, error_source_t source) {
    switch(rc) {
        case -ENOENT:
            log_print_error_string("%i: action %i: file id not found", source, operation);
            return ALP_STATUS_FILE_ID_NOT_EXISTS;
        case -EINVAL:
            log_print_error_string("%i: action %i: length and offset out of bounds", source, operation);
            return ALP_STATUS_LENGTH_OUT_OF_BOUNDS;
        case -ESIZE:
            log_print_error_string("%i: action %i: parameter was outside the expected range", source, operation);
            return ALP_STATUS_WRONG_OPERAND_FORMAT;
        case -ENOBUFS:
            log_print_error_string("%i: action %i: supplied data goes beyond file allocation", source, operation);
            return ALP_STATUS_DATA_OVERFLOW;
        case -EFBIG:
            log_print_error_string("%i: action %i: file format too large", source, operation);
            return ALP_STATUS_DATA_OVERFLOW;
        case -EBADF:
            log_print_error_string("%i: action %i: bad file id, probably above %i", source, operation, FRAMEWORK_FS_FILE_COUNT);
            return ALP_STATUS_FILE_ID_OUT_OF_BOUNDS;
        case -ENOEXEC:
            log_print_error_string("%i: action %i: can't execute because command is wrong", source, operation);
            return ALP_STATUS_UNKNOWN_OPERATION;
        case -EPERM:
            log_print_error_string("%i: action %i: operation not permitted, probably calling a function that's not implemented", source, operation);
            return ALP_STATUS_NOT_YET_IMPLEMENTED;
        case -EFAULT:
            log_print_error_string("%i: action %i: packet was not the expected length", source , operation);
            return ALP_STATUS_FIFO_OUT_OF_BOUNDS;
        case -EEXIST:
            log_print_error_string("%i: action %i: File already exists", source, operation);
            return ALP_STATUS_FILE_ID_ALREADY_EXISTS;
        case ALP_STATUS_FIFO_OUT_OF_BOUNDS:
            log_print_error_string("%i: action %i: fifo put byte failed", source, operation);
            return ALP_STATUS_FIFO_OUT_OF_BOUNDS;
        case ALP_STATUS_EXCEEDS_MAX_ALP_SIZE:
            log_print_error_string("%i: action %i: exceeds max alp size of %i", source, operation, ALP_PAYLOAD_MAX_SIZE);
            return ALP_STATUS_EXCEEDS_MAX_ALP_SIZE;
        case ALP_STATUS_NOT_YET_IMPLEMENTED:
            log_print_error_string("%i: action %i: trying to access not implemented actions", source, operation);
            return ALP_STATUS_NOT_YET_IMPLEMENTED;
        case ALP_STATUS_WRONG_OPERAND_FORMAT:
            log_print_error_string("%i: action %i: operand had wrong format or impossible values", source, operation);
            return ALP_STATUS_WRONG_OPERAND_FORMAT;
        case ALP_STATUS_EMPTY_ITF_STATUS:
            log_print_error_string("%i: action %i: expected interface status", source, operation);
            return ALP_STATUS_EMPTY_ITF_STATUS;
        case ALP_STATUS_COMMAND_NOT_FOUND:
            log_print_error_string("%i: action %i: no command found with given transid and itf-id", source, operation);
            return ALP_STATUS_COMMAND_NOT_FOUND;
        case ALP_STATUS_UNKNOWN_OPERATION:
            log_print_error_string("%i: action %i: unknown operation", source, operation);
            return ALP_STATUS_UNKNOWN_OPERATION;
        default:
            log_print_error_string("%i: action %i: unknown error %i", source, operation, rc);
            return ALP_STATUS_UNKNOWN_ERROR;
    }
}

static bool check_fifo_succeeded(int rc, alp_operation_t operation) {
    if(rc != SUCCESS) {
        alp_handle_error(ALP_STATUS_FIFO_OUT_OF_BOUNDS, operation, ERROR_ALP);
        return false;
    }
    return true;
}

alp_status_codes_t alp_register_interface(alp_interface_t* itf)
{
  for(uint8_t i=0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
    if(interfaces[i] == NULL) {                 //interface empty, add new one
      interfaces[i] = itf;
      return ALP_STATUS_OK;
    } else if(interfaces[i]->itf_id == itf->itf_id) { //interface already present, only update
      interfaces[i] = itf;
      return ALP_STATUS_PARTIALLY_COMPLETED; 
    }
  }
  return ALP_STATUS_UNKNOWN_ERROR;                  //all slots are taken, return error
}

bool alp_parse_length_operand(fifo_t* cmd_fifo, uint32_t* length)
{
    uint8_t len = 0;
    // TODO error handling
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, (uint8_t*)&len, 1), ALP_OP_READ_FILE_DATA))
        return false;
    uint8_t field_len = len >> 6;
    if(field_len == 0) {
        *length = (uint32_t)len;
        return true;
    }

    *length = (len & 0x3F) << ( 8 * field_len); // mask field length specificier bits and shift before adding other length bytes
    for(; field_len > 0; field_len--) {
        if(!check_fifo_succeeded(fifo_pop(cmd_fifo, &len, 1), ALP_OP_READ_FILE_DATA))
            return false;
        *length += len << (8 * (field_len - 1));
    }
    return true;
}

bool alp_append_length_operand(alp_command_t* command, uint32_t length) {
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    if (length < 64) {
        // can be coded in one byte
        return (fifo_put(cmd_fifo, (uint8_t*)&length, 1) == SUCCESS);
    }

    uint8_t size = 1;
    if (length > 0x3FFF)
        size = 2;
    if (length > 0x3FFFFF)
        size = 3;

    uint8_t byte = 0;
    byte += (size << 6); // length specifier bits
    byte += ((uint8_t*)(&length))[size];
    int rc;
    rc = fifo_put(cmd_fifo, &byte, 1);
    do {
        size--;
        rc += fifo_put(cmd_fifo, (uint8_t*)&length + size, 1);
    } while (size > 0);
    return rc == SUCCESS;
}

bool alp_parse_file_offset_operand(fifo_t* cmd_fifo, alp_operand_file_offset_t* operand)
{
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, &operand->file_id, 1), ALP_OP_CREATE_FILE))
        return false;
    if(!alp_parse_length_operand(cmd_fifo, &operand->offset))
        return false;
    return true;
}

bool alp_append_file_offset_operand(alp_command_t* command, uint8_t file_id, uint32_t offset) {
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    error_t rc;
    rc = fifo_put_byte(cmd_fifo, file_id);
    rc += !alp_append_length_operand(command, offset);

    if(rc != SUCCESS) {
        alp_handle_error(ALP_STATUS_FIFO_OUT_OF_BOUNDS, ALP_OP_CREATE_FILE, ERROR_ALP);
        return false;
    }
    return true;
}

bool alp_append_indirect_forward_action(
    alp_command_t* command, uint8_t file_id, bool overload, uint8_t* overload_config, uint8_t overload_config_len)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    error_t rc = SUCCESS;
    rc = fifo_put_byte(cmd_fifo, ALP_OP_INDIRECT_FORWARD | (overload << 7));
    rc += fifo_put_byte(cmd_fifo, file_id);

    if (overload) {
        rc += fifo_put(cmd_fifo, overload_config, overload_config_len);
    }

    DPRINT("INDIRECT FORWARD");
    if(rc != SUCCESS) {
        alp_handle_error(ALP_STATUS_FIFO_OUT_OF_BOUNDS, ALP_OP_INDIRECT_FORWARD, ERROR_ALP);
        return false;
    }
    return true;
}

bool alp_append_forward_action(alp_command_t* command, alp_interface_config_t* itf_config, uint8_t config_len)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    if(!itf_config) {
        alp_handle_error(ALP_STATUS_EMPTY_ITF_STATUS, ALP_OP_FORWARD, ERROR_ALP);
        return false;
    }
    error_t rc = SUCCESS;
    rc = fifo_put_byte(cmd_fifo, ALP_OP_FORWARD);
    rc += fifo_put_byte(cmd_fifo, itf_config->itf_id);

    if (itf_config->itf_id == ALP_ITF_ID_SERIAL) // TODO make optional?
    {
        // empty interface config
    } else if (itf_config->itf_id == ALP_ITF_ID_D7ASP) {
        rc += fifo_put_byte(cmd_fifo, ((d7ap_session_config_t*)itf_config->itf_config)->qos.raw);
        rc += fifo_put_byte(cmd_fifo, ((d7ap_session_config_t*)itf_config->itf_config)->dormant_timeout);
        rc += fifo_put_byte(cmd_fifo, ((d7ap_session_config_t*)itf_config->itf_config)->addressee.ctrl.raw);
        uint8_t id_length
            = d7ap_addressee_id_length(((d7ap_session_config_t*)itf_config->itf_config)->addressee.ctrl.id_type);
        rc += fifo_put_byte(cmd_fifo, ((d7ap_session_config_t*)itf_config->itf_config)->addressee.access_class);
        rc += fifo_put(cmd_fifo, ((d7ap_session_config_t*)itf_config->itf_config)->addressee.id, id_length);
    } else if (itf_config->itf_id == ALP_ITF_ID_LORAWAN_ABP) {
        uint8_t control_byte = ((lorawan_session_config_abp_t*)itf_config->itf_config)->request_ack << 1;
        control_byte += ((lorawan_session_config_abp_t*)itf_config->itf_config)->adr_enabled << 2;
        rc += fifo_put_byte(cmd_fifo, control_byte);
        rc += fifo_put_byte(cmd_fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->application_port);
        rc += fifo_put_byte(cmd_fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->data_rate);
        rc += fifo_put(cmd_fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->nwkSKey, 16);
        rc += fifo_put(cmd_fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->appSKey, 16);
        uint32_t dev_addr = __builtin_bswap32(((lorawan_session_config_abp_t*)itf_config->itf_config)->devAddr);
        rc += fifo_put(cmd_fifo, (uint8_t*)&dev_addr, 4);
        uint32_t network_id = __builtin_bswap32(((lorawan_session_config_abp_t*)itf_config->itf_config)->network_id);
        rc += fifo_put(cmd_fifo, (uint8_t*)&network_id, 4);
    } else if (itf_config->itf_id == ALP_ITF_ID_LORAWAN_OTAA) {
        uint8_t control_byte = ((lorawan_session_config_otaa_t*)itf_config->itf_config)->request_ack << 1;
        control_byte += ((lorawan_session_config_otaa_t*)itf_config->itf_config)->adr_enabled << 2;
        rc += fifo_put_byte(cmd_fifo, control_byte);
        rc += fifo_put_byte(cmd_fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->application_port);
        rc += fifo_put_byte(cmd_fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->data_rate);
        rc += fifo_put(cmd_fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->devEUI, 8);
        rc += fifo_put(cmd_fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->appEUI, 8);
        rc += fifo_put(cmd_fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->appKey, 16);
    } else {
        rc += fifo_put(cmd_fifo, itf_config->itf_config, config_len);
    }

    if(rc != SUCCESS) {
        alp_handle_error(ALP_STATUS_FIFO_OUT_OF_BOUNDS, ALP_OP_FORWARD, ERROR_ALP);
        return false;
    }

    DPRINT("FORWARD");
    return true;
}

bool alp_append_return_file_data_action(alp_command_t* command, uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data) {
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    int rc;
    rc = fifo_put_byte(cmd_fifo, ALP_OP_RETURN_FILE_DATA);
    rc += fifo_put_byte(cmd_fifo, file_id);
    rc += !alp_append_length_operand(command, offset);
    rc += !alp_append_length_operand(command, length);
    rc += fifo_put(cmd_fifo, data, length);
    return rc == SUCCESS;
}

static bool parse_operand_file_data(alp_command_t* command, alp_action_t* action)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    if(!alp_parse_file_offset_operand(cmd_fifo, &action->file_data_operand.file_offset))
        return false;
    if(!alp_parse_length_operand(cmd_fifo, &action->file_data_operand.provided_data_length))
        return false;
    if(action->file_data_operand.provided_data_length > sizeof(action->file_data_operand.data)) {
        alp_handle_error(ALP_STATUS_LENGTH_OUT_OF_BOUNDS, ALP_OP_WRITE_FILE_DATA, ERROR_ALP);
        return false;
    }
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, action->file_data_operand.data, action->file_data_operand.provided_data_length), ALP_OP_WRITE_FILE_DATA))
        return false;
    DPRINT("parsed file data operand file %i, len %i", action->file_data_operand.file_offset.file_id, action->file_data_operand.provided_data_length);
    return true;
}

static bool parse_operand_file_data_request(alp_command_t* command, alp_action_t* action)
{
    if(!alp_parse_file_offset_operand(&command->alp_command_fifo, &action->file_data_request_operand.file_offset))
        return false;
    if(!alp_parse_length_operand(&command->alp_command_fifo, &action->file_data_request_operand.requested_data_length))
        return false;
    DPRINT("parsed read file data file %i, len %i", action->file_data_operand.file_offset.file_id, action->file_data_operand.provided_data_length);
    return true;
}

static bool parse_operand_status(alp_command_t* command, alp_action_t* action, bool b6, bool b7)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    if (!b7 && !b6) {
        //action status operation
        DPRINT("act status");
        //uint8_t status_code = command[*parse_index];
        fifo_skip(cmd_fifo, 1);
        //TODO implement handling of action status
    } else if (!b7 && b6) {
        //interface status operation
        if(!check_fifo_succeeded(fifo_pop(cmd_fifo, &action->interface_status.itf_id, 1), ALP_OP_STATUS))
            return false;
        DPRINT("itf status (%i)", action->interface_status.itf_id);
        uint32_t temp_len;
        if(!alp_parse_length_operand(cmd_fifo, &temp_len))
            return false;
        action->interface_status.len = (uint8_t) temp_len;
        if(!check_fifo_succeeded(fifo_pop(cmd_fifo, action->interface_status.itf_status, action->interface_status.len), ALP_OP_STATUS))
            return false;
    } else {
        DPRINT("op_status ext not defined b6=%i, b7=%i", b6, b7);
        alp_handle_error(ALP_STATUS_UNKNOWN_OPERATION, ALP_OP_STATUS, ERROR_ALP);
        return false;
    }

    DPRINT("parsed interface status");
    return true;
}

static bool parse_op_forward(alp_command_t* command, alp_action_t* action)
{
    bool found = false;
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, &action->interface_config.itf_id, 1), ALP_OP_FORWARD))
        return false;

    if (action->interface_config.itf_id == ALP_ITF_ID_D7ASP) {
        uint8_t min_size = sizeof (d7ap_session_config_t) - 8; // substract max size of responder ID
        if(!check_fifo_succeeded(fifo_pop(cmd_fifo, action->interface_config.itf_config, min_size), ALP_OP_FORWARD))
            return false;
        uint8_t id_len = d7ap_addressee_id_length(((alp_interface_config_d7ap_t*)&(action->interface_config))->d7ap_session_config.addressee.ctrl.id_type);
        if(!check_fifo_succeeded(fifo_pop(cmd_fifo, action->interface_config.itf_config + min_size, id_len), ALP_OP_FORWARD))
            return false;
        return true;
    }

    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if (action->interface_config.itf_id == interfaces[i]->itf_id) {
            if(!check_fifo_succeeded(fifo_pop(cmd_fifo, action->interface_config.itf_config, interfaces[i]->itf_cfg_len), ALP_OP_FORWARD))
                return false;
            found = true;
            DPRINT("FORWARD %02X", action->interface_config.itf_id);
            break;
        }
    }

    if (!found) {
        DPRINT("FORWARD interface %02X not found", action->interface_config.itf_id);
        alp_handle_error(ALP_STATUS_WRONG_OPERAND_FORMAT, ALP_OP_FORWARD, ERROR_ALP);
        return false;
    }
    return true;
}

static bool parse_operand_file_id(alp_command_t* command, alp_action_t* action)
{
    if(!check_fifo_succeeded(fifo_pop(&command->alp_command_fifo, &action->file_id_operand.file_id, 1), ALP_OP_EXIST_FILE))
        return false;
    DPRINT("READ FILE PROPERTIES %i", action->file_id_operand.file_id);
    return true;
}

static bool parse_operand_file_header(alp_command_t* command, alp_action_t* action)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, &action->file_header_operand.file_id, 1), ALP_OP_WRITE_FILE_PROPERTIES))
        return false;
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, (uint8_t*)&action->file_header_operand.file_header, sizeof(d7ap_fs_file_header_t)), ALP_OP_WRITE_FILE_PROPERTIES))
        return false;

    // convert to little endian (native)
    action->file_header_operand.file_header.length = __builtin_bswap32(action->file_header_operand.file_header.length);
    action->file_header_operand.file_header.allocated_length
        = __builtin_bswap32(action->file_header_operand.file_header.allocated_length);

    DPRINT("WRITE FILE PROPERTIES %i", action->file_header_operand.file_id);
    return true;
}

static bool parse_operand_query(alp_command_t* command, alp_action_t* action)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    error_t err;
    DPRINT("BREAK QUERY");
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, &action->query_operand.code.raw, 1), ALP_OP_BREAK_QUERY))
        return false;

    if(action->query_operand.code.type != QUERY_CODE_TYPE_ARITHM_COMP_WITH_VALUE_IN_QUERY)
        return alp_handle_error(ALP_STATUS_NOT_YET_IMPLEMENTED, ALP_OP_BREAK_QUERY, ERROR_ALP);
    
    // TODO assuming no compare mask for now + assume compare value present + only 1 file offset operand
    if(action->query_operand.code.mask)
        return alp_handle_error(ALP_STATUS_NOT_YET_IMPLEMENTED, ALP_OP_BREAK_QUERY, ERROR_ALP);

    uint32_t temp;
    if(!alp_parse_length_operand(cmd_fifo, &temp))
        return false;

    action->query_operand.compare_operand_length = temp;

    if (action->query_operand.compare_operand_length > ALP_QUERY_COMPARE_BODY_MAX_SIZE)
        return alp_handle_error(-EINVAL, ALP_OP_BREAK_QUERY, ERROR_ALP);

    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, action->query_operand.compare_body, (uint16_t)action->query_operand.compare_operand_length), ALP_OP_BREAK_QUERY))
        return false;
    
    // parse the file offset operand(s), using a temp fifo
    fifo_t temp_fifo;
    fifo_init_filled(&temp_fifo, action->query_operand.compare_body + action->query_operand.compare_operand_length,
        2 * sizeof(alp_operand_file_offset_t), 2 * sizeof(alp_operand_file_offset_t));
    alp_operand_file_offset_t temp_offset;
    if(!alp_parse_file_offset_operand(&temp_fifo, &temp_offset)) // TODO assuming only 1 file offset operant. Why does this get discarded?
        return false;
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, action->query_operand.compare_body + action->query_operand.compare_operand_length, temp_fifo.head_idx), ALP_OP_BREAK_QUERY))
        return false;

    return true;
}

static bool parse_operand_tag_id(alp_command_t* command, alp_action_t* action)
{
    if(!check_fifo_succeeded(fifo_pop(&command->alp_command_fifo, &action->tag_id_operand.tag_id, 1), ALP_OP_REQUEST_TAG))
        return false;

    return true;
}

static bool parse_operand_interface_config(alp_command_t* command, alp_action_t* action)
{
    error_t err;
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    bool found = false;
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, &action->interface_config.itf_id, 1), ALP_OP_FORWARD))
        return false;
    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if (action->interface_config.itf_id == interfaces[i]->itf_id) {
            // session_config->itf_id = *itf_id;
            if (action->interface_config.itf_id == ALP_ITF_ID_D7ASP) {
#ifdef MODULE_D7AP
                uint8_t min_size = interfaces[i]->itf_cfg_len - 8; // substract max size of responder ID
                if(!check_fifo_succeeded(fifo_pop(cmd_fifo, (uint8_t*)&action->interface_config.itf_config, min_size), ALP_OP_FORWARD))
                    return false;
                uint8_t id_len
                    = d7ap_addressee_id_length(((d7ap_session_config_t*)action->interface_config.itf_config)
                                                   ->addressee.ctrl.id_type);
                if(!check_fifo_succeeded(fifo_pop(&command->alp_command_fifo, (uint8_t*)&action->interface_config.itf_config + min_size, id_len), ALP_OP_FORWARD))
                        return false;
#endif
            } else {
                if(!check_fifo_succeeded(fifo_pop(&command->alp_command_fifo, (uint8_t*)&action->interface_config.itf_config, interfaces[i]->itf_cfg_len), ALP_OP_FORWARD))
                        return false;
            }

            found = true;
            DPRINT("FORWARD %02X", action->interface_config.itf_id);
            break;
        }
    }
    
    if (!found) {
        DPRINT("FORWARD interface %02X not found", action->interface_config.itf_id);
        alp_handle_error(ALP_STATUS_WRONG_OPERAND_FORMAT, ALP_OP_FORWARD, ERROR_ALP);
        return false;
    }
    return true;
}

static bool parse_operand_indirect_interface(alp_command_t* command, alp_action_t* action)
{
    DPRINT("indirect fwd");
    error_t err;
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, &action->indirect_interface_operand.interface_file_id, 1), ALP_OP_INDIRECT_FORWARD))
        return false;

    if (action->ctrl.b7) {
        // overload bit set. To determine the overload length currently we need to read the itf_id from the file
        // since it is not coded in the ALP. // TODO discuss in protocol action group
        uint8_t itf_id;
        err = d7ap_fs_read_file(action->indirect_interface_operand.interface_file_id, 0, &itf_id, 1);
        if(err != SUCCESS) {
            alp_handle_error(err, ALP_OP_INDIRECT_FORWARD, ERROR_ALP);
            return false;
        }
        bool found = false;
        for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
            if (itf_id == interfaces[i]->itf_id) {
#ifdef MODULE_D7AP
                if(!check_fifo_succeeded(fifo_pop(cmd_fifo, (uint8_t*)&action->indirect_interface_operand.overload_data, 2), ALP_OP_INDIRECT_FORWARD))
                    return false;
                uint8_t id_len = d7ap_addressee_id_length(
                    ((alp_interface_config_d7ap_t*)action->indirect_interface_operand.overload_data)
                        ->d7ap_session_config.addressee.ctrl.id_type);
                if(!check_fifo_succeeded(fifo_pop(cmd_fifo, (uint8_t*)&action->indirect_interface_operand.overload_data + 2, id_len), ALP_OP_INDIRECT_FORWARD))
                    return false;
#endif
                found = true;
                DPRINT("indirect forward %02X", action->indirect_interface_operand.interface_file_id);
                break;
            }
        }
        if (!found) {
            DPRINT("interface %02X is not registered", action->indirect_interface_operand.interface_file_id);
            alp_handle_error(ALP_STATUS_WRONG_OPERAND_FORMAT, ALP_OP_INDIRECT_FORWARD, ERROR_ALP);
            return false;
        }
    }
    return true;
}

bool alp_parse_action(alp_command_t* command, alp_action_t* action)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    if(!check_fifo_succeeded(fifo_pop(cmd_fifo, &action->ctrl.raw, 1), ALP_OP_NOP))
        return false;
    DPRINT("ALP op %i", action->ctrl.operation);
    bool succeeded = false;
    
    switch (action->ctrl.operation) {
    case ALP_OP_WRITE_FILE_DATA:
    case ALP_OP_RETURN_FILE_DATA:
        succeeded = parse_operand_file_data(command, action);
        break;
    case ALP_OP_READ_FILE_DATA:
        succeeded = parse_operand_file_data_request(command, action);
        break;
    case ALP_OP_READ_FILE_PROPERTIES:
        succeeded = parse_operand_file_id(command, action);
        break;
    case ALP_OP_WRITE_FILE_PROPERTIES:
    case ALP_OP_CREATE_FILE:
        succeeded = parse_operand_file_header(command, action);
        break;
    case ALP_OP_STATUS:
        succeeded = parse_operand_status(command, action, action->ctrl.b6, action->ctrl.b7);
        break;
    case ALP_OP_BREAK_QUERY:
    case ALP_OP_ACTION_QUERY:
        succeeded = parse_operand_query(command, action);
        break;
    case ALP_OP_RESPONSE_TAG:
    case ALP_OP_REQUEST_TAG:
        succeeded = parse_operand_tag_id(command, action);
        break;
    case ALP_OP_FORWARD:
        succeeded = parse_operand_interface_config(command, action);
        break;
    case ALP_OP_INDIRECT_FORWARD:
        succeeded = parse_operand_indirect_interface(command, action);
        break;
    default:
        succeeded = false;
        alp_handle_error(ALP_STATUS_UNKNOWN_OPERATION, action->ctrl.operation, ERROR_ALP);
    }
    return succeeded;
}

int alp_get_expected_response_length(alp_command_t* command)
{
    uint8_t expected_response_length = 0;
    static alp_command_t command_copy;
    memcpy(&command_copy, command, sizeof(alp_command_t)); // use a copy, so we don't pop from the original command
    fifo_t* command_copy_fifo = &command_copy.alp_command_fifo;
    error_t e = 0;
    uint32_t length = 0;
    
    while (fifo_get_size(command_copy_fifo) > 0) {
        alp_control_t control;
        if(!check_fifo_succeeded(fifo_pop(command_copy_fifo, &control.raw, 1), ALP_OP_NOP))
            return -EINVAL;
        
        switch (control.operation) {
        case ALP_OP_READ_FILE_DATA:
            e += fifo_skip(command_copy_fifo, 1); // skip file ID
            uint32_t offset;
            e += !alp_parse_length_operand(command_copy_fifo, &offset);
            e += !alp_parse_length_operand(command_copy_fifo, &length);
            expected_response_length += alp_length_operand_coded_length(length); // the length of the provided data operand
            expected_response_length += alp_length_operand_coded_length(offset) + 1; // the length of the offset operand
            expected_response_length += 1; // the opcode
            break;
        case ALP_OP_READ_FILE_PROPERTIES:
            e += fifo_skip(command_copy_fifo, 1); //skip file ID
            break;
        case ALP_OP_REQUEST_TAG:
        case ALP_OP_RESPONSE_TAG:
            e += fifo_skip(command_copy_fifo, 1); // skip tag ID operand
            break;
        case ALP_OP_RETURN_FILE_DATA:
        case ALP_OP_WRITE_FILE_DATA:
            e += fifo_skip(command_copy_fifo, 1); // skip file ID
            uint32_t data_len;
            uint8_t error = 0;
            e += !alp_parse_length_operand(command_copy_fifo, &data_len); // offset
            e += !alp_parse_length_operand(command_copy_fifo, &data_len);
            e += fifo_skip(command_copy_fifo, data_len);
            break;
        case ALP_OP_FORWARD:;
            static alp_action_t action;
            e += !parse_op_forward(&command_copy, &action);
            break;
        case ALP_OP_INDIRECT_FORWARD:;
            e += fifo_skip(command_copy_fifo, 1); // skip interface file id
            if(control.b7) {
                alp_handle_error(ALP_STATUS_NOT_YET_IMPLEMENTED, ALP_OP_INDIRECT_FORWARD, ERROR_ALP);
                return -EPERM;
            }
            break;
        case ALP_OP_WRITE_FILE_PROPERTIES:
        case ALP_OP_CREATE_FILE:
        case ALP_OP_RETURN_FILE_PROPERTIES:
            e += fifo_skip(command_copy_fifo, (1 + sizeof(d7ap_fs_file_header_t))); // skip file ID & header
            break;
        case ALP_OP_BREAK_QUERY:
        case ALP_OP_ACTION_QUERY:
            e += fifo_skip(command_copy_fifo, 1);
            e += !alp_parse_length_operand(command_copy_fifo, &length);
            e += fifo_skip(command_copy_fifo, (uint16_t)length);
            e += fifo_skip(command_copy_fifo, 1);
            e += !alp_parse_length_operand(command_copy_fifo, &length);
            break;
        case ALP_OP_STATUS:
            if (!control.b6 && !control.b7) {
                // action status
                e += fifo_skip(command_copy_fifo, 1); // skip status code
            } else if (control.b6 && !control.b7) {
                // interface status
                e += fifo_skip(command_copy_fifo, 1); // skip status code
                uint32_t length;
                e += !alp_parse_length_operand(command_copy_fifo, &length);
                e += fifo_skip(command_copy_fifo, (uint16_t)length); // itf_status_len + itf status
            } else {
                alp_handle_error(-ENOEXEC, ALP_OP_STATUS, ERROR_ALP);
                return -ENOEXEC; 
            }

            break;
        // TODO other operations
        default:
            alp_handle_error(ALP_STATUS_UNKNOWN_OPERATION, control.operation, ERROR_ALP);
            return -ENOEXEC;
        }
    }
    if(e != SUCCESS) {
        alp_handle_error(ALP_STATUS_FIFO_OUT_OF_BOUNDS, ALP_OP_NOP, ERROR_ALP);
        return -EFAULT;
    }

    DPRINT("Expected ALP response length=%i", expected_response_length);
    return (int)expected_response_length;
}

bool alp_append_tag_request_action(alp_command_t* command, uint8_t tag_id, bool eop)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    DPRINT("append tag req %i", tag_id);
    uint8_t op = ALP_OP_REQUEST_TAG | (eop << 7);
    int rc = fifo_put_byte(cmd_fifo, op);
    rc += fifo_put_byte(cmd_fifo, tag_id);
    return (rc == SUCCESS);
}

bool alp_append_tag_response_action(alp_command_t* command, uint8_t tag_id, bool eop, bool err)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    DPRINT("append tag resp %i err %i", tag_id, err);
    uint8_t op = ALP_OP_RESPONSE_TAG | (eop << 7) | (err << 6);
    int rc = fifo_put_byte(cmd_fifo, op);
    rc += fifo_put_byte(cmd_fifo, tag_id);
    return (rc == SUCCESS);
}

bool alp_append_read_file_data_action(
    alp_command_t* command, uint8_t file_id, uint32_t offset, uint32_t length, bool resp, bool group)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    uint8_t op = ALP_OP_READ_FILE_DATA | (resp << 6) | (group << 7);
    int rc = fifo_put_byte(cmd_fifo, op);
    rc += !alp_append_file_offset_operand(command, file_id, offset);
    rc += !alp_append_length_operand(command, length);
    return (rc == SUCCESS);
}

bool alp_append_write_file_data_action(
    alp_command_t* command, uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data, bool resp, bool group)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    uint8_t op = ALP_OP_WRITE_FILE_DATA | (resp << 6) | (group << 7);
    int rc = fifo_put_byte(cmd_fifo, op);
    rc += !alp_append_file_offset_operand(command, file_id, offset);
    rc += !alp_append_length_operand(command, length);
    rc += fifo_put(cmd_fifo, data, length);
    return (rc == SUCCESS);
}

bool alp_append_interface_status(alp_command_t* command, alp_interface_status_t* status)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    int rc = fifo_put_byte(cmd_fifo, ALP_OP_STATUS + (1 << 6));
    rc += fifo_put(cmd_fifo, (uint8_t*)status, status->len + 2);
    return (rc == SUCCESS);
}

bool alp_append_create_new_file_data_action(
    alp_command_t* command, uint8_t file_id, uint32_t length, fs_storage_class_t storage_class, bool resp, bool group)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    uint8_t op = ALP_OP_CREATE_FILE | (resp << 6) | (group << 7);
    int rc = fifo_put_byte(cmd_fifo, op);
    alp_operand_file_header_t header = { .file_id = file_id,
        .file_header = { .file_permissions = 0,
            .file_properties.action_protocol_enabled = 0,
            .file_properties.storage_class = storage_class,
            .length = __builtin_bswap32(length),
            .allocated_length = __builtin_bswap32(length) } };
    rc += fifo_put(cmd_fifo, (uint8_t*)&header, sizeof(alp_operand_file_header_t));
    return (rc == SUCCESS);
}

uint8_t alp_length_operand_coded_length(uint32_t length) {
  uint8_t coded_len = 1;
  if(length > 0x3F)
    coded_len = 2;

  if(length > 0x3FFF)
    coded_len = 3;

  if(length > 0x3FFFFF)
    coded_len = 4;

  return coded_len;
}
