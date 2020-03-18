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

#include "alp.h"
#include "dae.h"
#include "fifo.h"
#include "d7ap.h"
#include "log.h"
#include "lorawan_stack.h"

#include "modules_defs.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_ALP_LOG_ENABLED)
  #define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
  #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINT_DATA(...)
#endif

alp_interface_t* interfaces[MODULE_ALP_INTERFACE_SIZE];

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

uint32_t alp_parse_length_operand(fifo_t* cmd_fifo)
{
    // TODO error handling
    uint8_t len = 0;
    fifo_pop(cmd_fifo, (uint8_t*)&len, 1);
    uint8_t field_len = len >> 6;
    if(field_len == 0)
        return (uint32_t)len;
    
    uint32_t full_length = (len & 0x3F) << ( 8 * field_len); // mask field length specificier bits and shift before adding other length bytes
    fifo_pop(cmd_fifo, (uint8_t*)&full_length, field_len);
    return full_length;
}

void alp_append_length_operand(alp_command_t* command, uint32_t length) {
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    if (length < 64) {
        // can be coded in one byte
        assert(fifo_put(cmd_fifo, (uint8_t*)&length, 1) == SUCCESS);
        return;
    }

    uint8_t size = 1;
    if (length > 0x3FFF)
        size = 2;
    if (length > 0x3FFFFF)
        size = 3;

    uint8_t byte = 0;
    byte += (size << 6); // length specifier bits
    byte += ((uint8_t*)(&length))[size];
    assert(fifo_put(cmd_fifo, &byte, 1) == SUCCESS);
    do {
        size--;
        assert(fifo_put(cmd_fifo, (uint8_t*)&length + size, 1) == SUCCESS);
    } while (size > 0);
}

alp_operand_file_offset_t alp_parse_file_offset_operand(fifo_t* cmd_fifo)
{
    alp_operand_file_offset_t operand;
    error_t err = fifo_pop(cmd_fifo, &operand.file_id, 1); assert(err == SUCCESS);
    operand.offset = alp_parse_length_operand(cmd_fifo);
    return operand;
}

void alp_append_file_offset_operand(alp_command_t* command, uint8_t file_id, uint32_t offset) {
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    assert(fifo_put_byte(cmd_fifo, file_id) == SUCCESS);
    alp_append_length_operand(command, offset);
}

void alp_append_indirect_forward_action(
    alp_command_t* command, uint8_t file_id, bool overload, uint8_t* overload_config, uint8_t overload_config_len)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    assert(fifo_put_byte(cmd_fifo, ALP_OP_INDIRECT_FORWARD | (overload << 7)) == SUCCESS);
    assert(fifo_put_byte(cmd_fifo, file_id) == SUCCESS);

    if (overload) {
        assert(fifo_put(cmd_fifo, overload_config, overload_config_len) == SUCCESS);
    }

    DPRINT("INDIRECT FORWARD");
}

void alp_append_forward_action(alp_command_t* command, alp_interface_config_t* itf_config, uint8_t config_len)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    assert(itf_config != NULL);
    assert(fifo_put_byte(cmd_fifo, ALP_OP_FORWARD) == SUCCESS);
    assert(fifo_put_byte(cmd_fifo, itf_config->itf_id) == SUCCESS);

    if (itf_config->itf_id == ALP_ITF_ID_SERIAL) // TODO make optional?
    {
        // empty interface config
    } else if (itf_config->itf_id == ALP_ITF_ID_D7ASP) {
        assert(fifo_put_byte(cmd_fifo, ((d7ap_session_config_t*)itf_config->itf_config)->qos.raw) == SUCCESS);
        assert(fifo_put_byte(cmd_fifo, ((d7ap_session_config_t*)itf_config->itf_config)->dormant_timeout) == SUCCESS);
        assert(
            fifo_put_byte(cmd_fifo, ((d7ap_session_config_t*)itf_config->itf_config)->addressee.ctrl.raw) == SUCCESS);
        uint8_t id_length
            = d7ap_addressee_id_length(((d7ap_session_config_t*)itf_config->itf_config)->addressee.ctrl.id_type);
        assert(fifo_put_byte(cmd_fifo, ((d7ap_session_config_t*)itf_config->itf_config)->addressee.access_class)
            == SUCCESS);
        assert(
            fifo_put(cmd_fifo, ((d7ap_session_config_t*)itf_config->itf_config)->addressee.id, id_length) == SUCCESS);
    } else if (itf_config->itf_id == ALP_ITF_ID_LORAWAN_ABP) {
        uint8_t control_byte = ((lorawan_session_config_abp_t*)itf_config->itf_config)->request_ack << 1;
        control_byte += ((lorawan_session_config_abp_t*)itf_config->itf_config)->adr_enabled << 2;
        assert(fifo_put_byte(cmd_fifo, control_byte) == SUCCESS);
        assert(fifo_put_byte(cmd_fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->application_port)
            == SUCCESS);
        assert(fifo_put_byte(cmd_fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->data_rate) == SUCCESS);
        assert(fifo_put(cmd_fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->nwkSKey, 16) == SUCCESS);
        assert(fifo_put(cmd_fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->appSKey, 16) == SUCCESS);
        uint32_t dev_addr = __builtin_bswap32(((lorawan_session_config_abp_t*)itf_config->itf_config)->devAddr);
        assert(fifo_put(cmd_fifo, (uint8_t*)&dev_addr, 4) == SUCCESS);
        uint32_t network_id = __builtin_bswap32(((lorawan_session_config_abp_t*)itf_config->itf_config)->network_id);

        assert(fifo_put(cmd_fifo, (uint8_t*)&network_id, 4) == SUCCESS);
    } else if (itf_config->itf_id == ALP_ITF_ID_LORAWAN_OTAA) {
        uint8_t control_byte = ((lorawan_session_config_otaa_t*)itf_config->itf_config)->request_ack << 1;
        control_byte += ((lorawan_session_config_otaa_t*)itf_config->itf_config)->adr_enabled << 2;
        assert(fifo_put_byte(cmd_fifo, control_byte) == SUCCESS);
        assert(fifo_put_byte(cmd_fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->application_port)
            == SUCCESS);
        assert(fifo_put_byte(cmd_fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->data_rate) == SUCCESS);
        assert(fifo_put(cmd_fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->devEUI, 8) == SUCCESS);
        assert(fifo_put(cmd_fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->appEUI, 8) == SUCCESS);
        assert(fifo_put(cmd_fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->appKey, 16) == SUCCESS);
    } else {
        assert(fifo_put(cmd_fifo, itf_config->itf_config, config_len) == SUCCESS);
    }

    DPRINT("FORWARD");
}

void alp_append_return_file_data_action(alp_command_t* command, uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data) {
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    assert(fifo_put_byte(cmd_fifo, ALP_OP_RETURN_FILE_DATA) == SUCCESS);
    assert(fifo_put_byte(cmd_fifo, file_id) == SUCCESS);
    alp_append_length_operand(command, offset);
    alp_append_length_operand(command, length);
    assert(fifo_put(cmd_fifo, data, length) == SUCCESS);
}

static void parse_operand_file_data(alp_command_t* command, alp_action_t* action)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    action->file_data_operand.file_offset = alp_parse_file_offset_operand(cmd_fifo);
    action->file_data_operand.provided_data_length = alp_parse_length_operand(cmd_fifo);
    assert(action->file_data_operand.provided_data_length <= sizeof(action->file_data_operand.data));
    fifo_pop(cmd_fifo, action->file_data_operand.data, action->file_data_operand.provided_data_length);
    DPRINT("parsed write file data file %i, len %i", action->file_data_operand.file_offset.file_id, action->file_data_operand.provided_data_length);
}

static void parse_operand_file_data_request(alp_command_t* command, alp_action_t* action)
{
    action->file_data_request_operand.file_offset = alp_parse_file_offset_operand(&command->alp_command_fifo);
    action->file_data_request_operand.requested_data_length = alp_parse_length_operand(&command->alp_command_fifo);
    DPRINT("parsed read file data file %i, len %i", action->file_data_operand.file_offset.file_id, action->file_data_operand.provided_data_length);
}

static void parse_operand_status(alp_command_t* command, alp_action_t* action, bool b6, bool b7)
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
        assert(fifo_pop(cmd_fifo, &action->interface_status.itf_id, 1) == SUCCESS);
        DPRINT("itf status (%i)", action->interface_status.itf_id);
        action->interface_status.len = (uint8_t)alp_parse_length_operand(cmd_fifo);
        assert(fifo_pop(cmd_fifo, action->interface_status.itf_status, action->interface_status.len) == SUCCESS);
    } else {
        DPRINT("op_status ext not defined b6=%i, b7=%i", b6, b7);
        assert(false); // TODO
    }

    DPRINT("parsed interface status");
}

static void parse_op_forward(alp_command_t* command, alp_action_t* action)
{
    bool found = false;
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    fifo_pop(cmd_fifo, &action->interface_config.itf_id, 1);

    if (action->interface_config.itf_id == ALP_ITF_ID_D7ASP) {
        uint8_t min_size = sizeof (d7ap_session_config_t) - 8; // substract max size of responder ID
        fifo_pop(cmd_fifo, action->interface_config.itf_config, min_size);
        uint8_t id_len = d7ap_addressee_id_length(((alp_interface_config_d7ap_t*)&(action->interface_config))->d7ap_session_config.addressee.ctrl.id_type);
        fifo_pop(cmd_fifo, action->interface_config.itf_config + min_size, id_len);
        return;
    }

    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if (action->interface_config.itf_id == interfaces[i]->itf_id) {
            fifo_pop(cmd_fifo, action->interface_config.itf_config, interfaces[i]->itf_cfg_len);
            found = true;
            DPRINT("FORWARD %02X", action->interface_config.itf_id);
            break;
        }
    }

    if (!found) {
        DPRINT("FORWARD interface %02X not found", action->interface_config.itf_id);
        assert(false);
    }
}

static void parse_operand_file_id(alp_command_t* command, alp_action_t* action)
{
    fifo_pop(&command->alp_command_fifo, &action->file_id_operand.file_id, 1);
    DPRINT("READ FILE PROPERTIES %i", action->file_id_operand.file_id);
}

static void parse_operand_file_header(alp_command_t* command, alp_action_t* action)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    error_t err = fifo_pop(cmd_fifo, &action->file_header_operand.file_id, 1);
    assert(err == SUCCESS);
    err = fifo_pop(cmd_fifo, (uint8_t*)&action->file_header_operand.file_header, sizeof(d7ap_fs_file_header_t));
    assert(err == SUCCESS);

    // convert to little endian (native)
    action->file_header_operand.file_header.length = __builtin_bswap32(action->file_header_operand.file_header.length);
    action->file_header_operand.file_header.allocated_length
        = __builtin_bswap32(action->file_header_operand.file_header.allocated_length);

    DPRINT("WRITE FILE PROPERTIES %i", action->file_header_operand.file_id);
}

static void parse_operand_query(alp_command_t* command, alp_action_t* action)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    error_t err;
    DPRINT("BREAK QUERY");
    err = fifo_pop(cmd_fifo, &action->query_operand.code.raw, 1);
    assert(err == SUCCESS);

    assert(action->query_operand.code.type
        == QUERY_CODE_TYPE_ARITHM_COMP_WITH_VALUE_IN_QUERY); // TODO only arithm comp with value type is implemented for

    action->query_operand.compare_operand_length = alp_parse_length_operand(cmd_fifo);
    if (action->query_operand.compare_operand_length > ALP_QUERY_COMPARE_BODY_MAX_SIZE)
        assert(false); // TODO error handling

    // TODO assuming no compare mask for now + assume compare value present + only 1 file offset operand
    assert(!action->query_operand.code.mask);

    err = fifo_pop(
        cmd_fifo, action->query_operand.compare_body, (uint16_t)action->query_operand.compare_operand_length);
    assert(err == SUCCESS);
    
    // parse the file offset operand(s), using a temp fifo
    fifo_t temp_fifo;
    fifo_init_filled(&temp_fifo, action->query_operand.compare_body + action->query_operand.compare_operand_length,
        2 * sizeof(alp_operand_file_offset_t), 2 * sizeof(alp_operand_file_offset_t));
    alp_parse_file_offset_operand(&temp_fifo); // TODO assuming only 1 file offset operant
    err = fifo_pop(cmd_fifo, action->query_operand.compare_body + action->query_operand.compare_operand_length,
        temp_fifo.head_idx);
    assert(err == SUCCESS);
}

static void parse_operand_tag_id(alp_command_t* command, alp_action_t* action)
{
    assert(fifo_pop(&command->alp_command_fifo, &action->tag_id_operand.tag_id, 1) == SUCCESS);
}

static void parse_operand_interface_config(alp_command_t* command, alp_action_t* action)
{
    error_t err;
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    bool found = false;
    err = fifo_pop(cmd_fifo, &action->interface_config.itf_id, 1);
    assert(err == SUCCESS);
    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if (action->interface_config.itf_id == interfaces[i]->itf_id) {
            // session_config->itf_id = *itf_id;
            if (action->interface_config.itf_id == ALP_ITF_ID_D7ASP) {
#ifdef MODULE_D7AP
                uint8_t min_size = interfaces[i]->itf_cfg_len - 8; // substract max size of responder ID
                err = fifo_pop(cmd_fifo, (uint8_t*)&action->interface_config.itf_config, min_size);
                assert(err == SUCCESS);
                uint8_t id_len
                    = d7ap_addressee_id_length(((d7ap_session_config_t*)action->interface_config.itf_config)
                                                   ->addressee.ctrl.id_type);
                err = fifo_pop(
                    &command->alp_command_fifo, (uint8_t*)&action->interface_config.itf_config + min_size, id_len);
                assert(err == SUCCESS);
#endif
            } else {
                err = fifo_pop(&command->alp_command_fifo, (uint8_t*)&action->interface_config.itf_config,
                    interfaces[i]->itf_cfg_len);
                assert(err == SUCCESS);
            }

            found = true;
            DPRINT("FORWARD %02X", action->interface_config.itf_id);
            break;
        }
    }
    
    if (!found) {
        DPRINT("FORWARD interface %02X not found", action->interface_config.itf_id);
        assert(false);
    }
}

static void parse_operand_indirect_interface(alp_command_t* command, alp_action_t* action)
{
    DPRINT("indirect fwd");
    error_t err;
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    err = fifo_pop(cmd_fifo, &action->indirect_interface_operand.interface_file_id, 1);
    assert(err == SUCCESS);

    if (action->ctrl.b7) {
        // overload bit set. To determine the overload length currently we need to read the itf_id from the file
        // since it is not coded in the ALP. // TODO discuss in protocol action group
        uint8_t itf_id;
        err = d7ap_fs_read_file(action->indirect_interface_operand.interface_file_id, 0, &itf_id, 1);
        assert(err == SUCCESS);
        bool found = false;
        for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
            if (itf_id == interfaces[i]->itf_id) {
#ifdef MODULE_D7AP
                err = fifo_pop(cmd_fifo, (uint8_t*)&action->indirect_interface_operand.overload_data, 2);
                assert(err == SUCCESS);
                uint8_t id_len = d7ap_addressee_id_length(
                    ((alp_interface_config_d7ap_t*)action->indirect_interface_operand.overload_data)
                        ->d7ap_session_config.addressee.ctrl.id_type);
                err = fifo_pop(cmd_fifo, (uint8_t*)&action->indirect_interface_operand.overload_data + 2, id_len);
                assert(err == SUCCESS);
#endif
                found = true;
                DPRINT("indirect forward %02X", action->indirect_interface_operand.interface_file_id);
                break;
            }
        }
        if (!found) {
            DPRINT("interface %02X is not registered", action->indirect_interface_operand.interface_file_id);
            assert(false);
        }
    }
}

void alp_parse_action(alp_command_t* command, alp_action_t* action)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    assert(fifo_pop(cmd_fifo, &action->ctrl.raw, 1) == SUCCESS);
    DPRINT("ALP op %i", action->ctrl.operation);
    
    switch (action->ctrl.operation) {
    case ALP_OP_WRITE_FILE_DATA:
    case ALP_OP_RETURN_FILE_DATA:
        parse_operand_file_data(command, action);
        break;
    case ALP_OP_READ_FILE_DATA:
        parse_operand_file_data_request(command, action);
        break;
    case ALP_OP_READ_FILE_PROPERTIES:
        parse_operand_file_id(command, action);
        break;
    case ALP_OP_WRITE_FILE_PROPERTIES:
    case ALP_OP_CREATE_FILE:
        parse_operand_file_header(command, action);
        break;
    case ALP_OP_STATUS:
        parse_operand_status(command, action, action->ctrl.b6, action->ctrl.b7);
        break;
    case ALP_OP_BREAK_QUERY:
    case ALP_OP_ACTION_QUERY:
        parse_operand_query(command, action);
        break;
    case ALP_OP_RESPONSE_TAG:
    case ALP_OP_REQUEST_TAG:
        parse_operand_tag_id(command, action);
        break;
    case ALP_OP_FORWARD:
        parse_operand_interface_config(command, action);
        break;
    case ALP_OP_INDIRECT_FORWARD:
        parse_operand_indirect_interface(command, action);
        break;
    default:
        DPRINT("op %i not implemented\n", action->ctrl.operation);
        assert(false); // TODO
    }
}

uint8_t alp_get_expected_response_length(alp_command_t* command)
{
    uint8_t expected_response_length = 0;
    static alp_command_t command_copy;
    memcpy(&command_copy, command, sizeof(alp_command_t)); // use a copy, so we don't pop from the original command
    fifo_t* command_copy_fifo = &command_copy.alp_command_fifo;
    error_t e;
    
    while (fifo_get_size(command_copy_fifo) > 0) {
        alp_control_t control;
        e = fifo_pop(command_copy_fifo, &control.raw, 1);
        assert(e == SUCCESS);
        
        switch (control.operation) {
        case ALP_OP_READ_FILE_DATA:
            fifo_skip(command_copy_fifo, 1); // skip file ID
            uint32_t offset = alp_parse_length_operand(command_copy_fifo); // offset
            expected_response_length += alp_parse_length_operand(command_copy_fifo); // the file length
            expected_response_length += alp_length_operand_coded_length(expected_response_length); // the length of the provided data operand
            expected_response_length += alp_length_operand_coded_length(offset) + 1; // the length of the offset operand
            expected_response_length += 1; // the opcode
            break;
        case ALP_OP_READ_FILE_PROPERTIES:
            fifo_skip(command_copy_fifo, 1); //skip file ID
            break;
        case ALP_OP_REQUEST_TAG:
        case ALP_OP_RESPONSE_TAG:
            fifo_skip(command_copy_fifo, 1); // skip tag ID operand
            break;
        case ALP_OP_RETURN_FILE_DATA:
        case ALP_OP_WRITE_FILE_DATA:
            fifo_skip(command_copy_fifo, 1); // skip file ID
            alp_parse_length_operand(command_copy_fifo); // offset
            uint32_t data_len = alp_parse_length_operand(command_copy_fifo);
            fifo_skip(command_copy_fifo, data_len);
            break;
        case ALP_OP_FORWARD:;
            static alp_action_t action;
            parse_op_forward(&command_copy, &action);
            break;
        case ALP_OP_INDIRECT_FORWARD:;
            fifo_skip(command_copy_fifo, 1); // skip interface file id
            assert(!control.b7); // TODO we do not support overload data for now
            break;
        case ALP_OP_WRITE_FILE_PROPERTIES:
        case ALP_OP_CREATE_FILE:
        case ALP_OP_RETURN_FILE_PROPERTIES:
            fifo_skip(command_copy_fifo, (1 + sizeof(d7ap_fs_file_header_t))); // skip file ID & header
            break;
        case ALP_OP_BREAK_QUERY:
        case ALP_OP_ACTION_QUERY:
            fifo_skip(command_copy_fifo, 1);
            fifo_skip(command_copy_fifo, (uint16_t)alp_parse_length_operand(command_copy_fifo));
            fifo_skip(command_copy_fifo, 1);
            alp_parse_length_operand(command_copy_fifo);
            break;
        case ALP_OP_STATUS:
            if (!control.b6 && !control.b7) {
                // action status
                fifo_skip(command_copy_fifo, 1); // skip status code
            } else if (control.b6 && !control.b7) {
                // interface status
                fifo_skip(command_copy_fifo, 1); // skip status code
                fifo_skip(command_copy_fifo, (uint16_t)alp_parse_length_operand(command_copy_fifo)); // itf_status_len + itf status
            } else {
                assert(false); // TODO
            }

            break;
        // TODO other operations
        default:
            DPRINT("op %i not implemented", control.operation);
            assert(false);
        }
    }

    DPRINT("Expected ALP response length=%i", expected_response_length);
    return expected_response_length;
}

void alp_append_tag_request_action(alp_command_t* command, uint8_t tag_id, bool eop)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    DPRINT("append tag req %i", tag_id);
    uint8_t op = ALP_OP_REQUEST_TAG | (eop << 7);
    assert(fifo_put_byte(cmd_fifo, op) == SUCCESS);
    assert(fifo_put_byte(cmd_fifo, tag_id) == SUCCESS);
}

void alp_append_tag_response_action(alp_command_t* command, uint8_t tag_id, bool eop, bool err)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    DPRINT("append tag resp %i err %i", tag_id, err);
    uint8_t op = ALP_OP_RESPONSE_TAG | (eop << 7) | (err << 6);
    assert(fifo_put_byte(cmd_fifo, op) == SUCCESS);
    assert(fifo_put_byte(cmd_fifo, tag_id) == SUCCESS);
}

void alp_append_read_file_data_action(
    alp_command_t* command, uint8_t file_id, uint32_t offset, uint32_t length, bool resp, bool group)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    uint8_t op = ALP_OP_READ_FILE_DATA | (resp << 6) | (group << 7);
    assert(fifo_put_byte(cmd_fifo, op) == SUCCESS);
    alp_append_file_offset_operand(command, file_id, offset);
    alp_append_length_operand(command, length);
}

void alp_append_write_file_data_action(
    alp_command_t* command, uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data, bool resp, bool group)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    uint8_t op = ALP_OP_WRITE_FILE_DATA | (resp << 6) | (group << 7);
    assert(fifo_put_byte(cmd_fifo, op) == SUCCESS);
    alp_append_file_offset_operand(command, file_id, offset);
    alp_append_length_operand(command, length);
    assert(fifo_put(cmd_fifo, data, length) == SUCCESS);
}

void alp_append_interface_status(alp_command_t* command, alp_interface_status_t* status)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    fifo_put_byte(cmd_fifo, ALP_OP_STATUS + (1 << 6));
    fifo_put(cmd_fifo, (uint8_t*)status, status->len + 2);
}

void alp_append_create_new_file_data_action(
    alp_command_t* command, uint8_t file_id, uint32_t length, fs_storage_class_t storage_class, bool resp, bool group)
{
    fifo_t* cmd_fifo = &command->alp_command_fifo;
    uint8_t op = ALP_OP_CREATE_FILE | (resp << 6) | (group << 7);
    assert(fifo_put_byte(cmd_fifo, op) == SUCCESS);
    alp_operand_file_header_t header = { .file_id = file_id,
        .file_header = { .file_permissions = 0,
            .file_properties.action_protocol_enabled = 0,
            .file_properties.storage_class = storage_class,
            .length = __builtin_bswap32(length),
            .allocated_length = __builtin_bswap32(length) } };
    assert(fifo_put(cmd_fifo, (uint8_t*)&header, sizeof(alp_operand_file_header_t)) == SUCCESS);
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
