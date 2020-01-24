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

#include "alp.h"
#include "dae.h"
#include "fifo.h"
#include "d7ap.h"
#include "log.h"
#include "lorawan_stack.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_ALP_LOG_ENABLED)
  #define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#else
  #define DPRINT(...)
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

alp_operation_t alp_get_operation(uint8_t* alp_command)
{
    alp_control_t alp_ctrl;
    alp_ctrl.raw = (*alp_command);
    return alp_ctrl.operation;
}

uint32_t alp_parse_length_operand(fifo_t* cmd_fifo) {
  uint8_t len = 0;
  fifo_pop(cmd_fifo, (uint8_t*)&len, 1);
  uint8_t field_len = len >> 6;
  if(field_len == 0)
    return (uint32_t)len;

  uint32_t full_length = (len & 0x3F) << ( 8 * field_len); // mask field length specificier bits and shift before adding other length bytes
  fifo_pop(cmd_fifo, (uint8_t*)&full_length, field_len);
  return full_length;
}

void alp_append_length_operand(fifo_t* fifo, uint32_t length) {
  if(length < 64) {
    // can be coded in one byte
    assert(fifo_put(fifo, (uint8_t*)&length, 1) == SUCCESS);
    return;
  }

  uint8_t size = 1;
  if(length > 0x3FFF)
    size = 2;
  if(length > 0x3FFFFF)
    size = 3;

  uint8_t byte = 0;
  byte += (size << 6); // length specifier bits
  byte += ((uint8_t*)(&length))[size];
  assert(fifo_put(fifo, &byte, 1) == SUCCESS);
  do {
    size--;
    assert(fifo_put(fifo, (uint8_t*)&length + size, 1) == SUCCESS);
  } while(size > 0);
}

alp_operand_file_offset_t alp_parse_file_offset_operand(fifo_t* cmd_fifo) {
  alp_operand_file_offset_t operand;
  error_t err = fifo_pop(cmd_fifo, &operand.file_id, 1); assert(err == SUCCESS);
  operand.offset = alp_parse_length_operand(cmd_fifo);
  return operand;
}

alp_operand_file_header_t alp_parse_file_header_operand(fifo_t* cmd_fifo) {
  alp_operand_file_header_t operand;
  error_t err = fifo_pop(cmd_fifo, &operand.file_id, 1); assert(err == SUCCESS);
  err = fifo_pop(cmd_fifo, (uint8_t*)&operand.file_header, sizeof(d7ap_fs_file_header_t)); assert(err == SUCCESS);

  // convert to little endian (native)
  operand.file_header.length = __builtin_bswap32(operand.file_header.length);
  operand.file_header.allocated_length = __builtin_bswap32(operand.file_header.allocated_length);
  return operand;
}

void alp_append_file_offset_operand(fifo_t* fifo, uint8_t file_id, uint32_t offset) {
  assert(fifo_put_byte(fifo, file_id) == SUCCESS);
  alp_append_length_operand(fifo, offset);
}

void alp_append_indirect_forward_action(fifo_t* fifo, uint8_t file_id, bool overload, uint8_t *overload_config, uint8_t overload_config_len) {
  assert(fifo_put_byte(fifo, ALP_OP_INDIRECT_FORWARD | (overload << 7)) == SUCCESS);
  assert(fifo_put_byte(fifo, file_id) == SUCCESS);

  if (overload) {
    assert(fifo_put(fifo, overload_config, overload_config_len) == SUCCESS);
  }

  DPRINT("INDIRECT FORWARD");
}

void alp_append_forward_action(fifo_t* fifo, alp_interface_config_t *itf_config, uint8_t config_len) {
  assert(itf_config!=NULL);
  assert(fifo_put_byte(fifo, ALP_OP_FORWARD) == SUCCESS);
  assert(fifo_put_byte(fifo, itf_config->itf_id) == SUCCESS);

  if (itf_config->itf_id == ALP_ITF_ID_SERIAL) // TODO make optional?
  {
    // empty interface config
  }
  else if (itf_config->itf_id == ALP_ITF_ID_D7ASP)
  {
    assert(fifo_put_byte(fifo, ((d7ap_session_config_t*)itf_config->itf_config)->qos.raw) == SUCCESS);
    assert(fifo_put_byte(fifo, ((d7ap_session_config_t*)itf_config->itf_config)->dormant_timeout) == SUCCESS);
    assert(fifo_put_byte(fifo, ((d7ap_session_config_t*)itf_config->itf_config)->addressee.ctrl.raw) == SUCCESS);
    uint8_t id_length = d7ap_addressee_id_length(((d7ap_session_config_t*)itf_config->itf_config)->addressee.ctrl.id_type);
    assert(fifo_put_byte(fifo, ((d7ap_session_config_t*)itf_config->itf_config)->addressee.access_class) == SUCCESS);
    assert(fifo_put(fifo, ((d7ap_session_config_t*)itf_config->itf_config)->addressee.id, id_length) == SUCCESS);
  }
  else if(itf_config->itf_id == ALP_ITF_ID_LORAWAN_ABP)
  {
    uint8_t control_byte = ((lorawan_session_config_abp_t*)itf_config->itf_config)->request_ack << 1;
    control_byte += ((lorawan_session_config_abp_t*)itf_config->itf_config)->adr_enabled << 2;
    assert(fifo_put_byte(fifo, control_byte) == SUCCESS);
    assert(fifo_put_byte(fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->application_port) == SUCCESS);
    assert(fifo_put_byte(fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->data_rate) == SUCCESS);
    assert(fifo_put(fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->nwkSKey, 16) == SUCCESS);
    assert(fifo_put(fifo, ((lorawan_session_config_abp_t*)itf_config->itf_config)->appSKey, 16) == SUCCESS);
    uint32_t dev_addr = __builtin_bswap32(((lorawan_session_config_abp_t*)itf_config->itf_config)->devAddr);
    assert(fifo_put(fifo, (uint8_t*)&dev_addr, 4) == SUCCESS);
    uint32_t network_id = __builtin_bswap32(((lorawan_session_config_abp_t*)itf_config->itf_config)->network_id);

    assert(fifo_put(fifo, (uint8_t*)&network_id, 4) == SUCCESS);
  }
  else if(itf_config->itf_id == ALP_ITF_ID_LORAWAN_OTAA)
  {
    uint8_t control_byte = ((lorawan_session_config_otaa_t*)itf_config->itf_config)->request_ack << 1;
    control_byte += ((lorawan_session_config_otaa_t*)itf_config->itf_config)->adr_enabled << 2;
    assert(fifo_put_byte(fifo, control_byte) == SUCCESS);
    assert(fifo_put_byte(fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->application_port) == SUCCESS);
    assert(fifo_put_byte(fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->data_rate) == SUCCESS);
    assert(fifo_put(fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->devEUI, 8) == SUCCESS);
    assert(fifo_put(fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->appEUI, 8) == SUCCESS);
    assert(fifo_put(fifo, ((lorawan_session_config_otaa_t*)itf_config->itf_config)->appKey, 16) == SUCCESS);
  }
  else
  {
    assert(fifo_put(fifo, itf_config->itf_config, config_len) == SUCCESS);
  }

  DPRINT("FORWARD");
}

void alp_append_return_file_data_action(fifo_t* fifo, uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data) {
  assert(fifo_put_byte(fifo, ALP_OP_RETURN_FILE_DATA) == SUCCESS);
  assert(fifo_put_byte(fifo, file_id) == SUCCESS);
  alp_append_length_operand(fifo, offset);
  alp_append_length_operand(fifo, length);
  assert(fifo_put(fifo, data, length) == SUCCESS);
}

static void parse_operand_file_data(fifo_t* fifo, alp_action_t* action) {
  action->file_data_operand.file_offset = alp_parse_file_offset_operand(fifo);
  action->file_data_operand.provided_data_length = alp_parse_length_operand(fifo);
  assert(action->file_data_operand.provided_data_length <= sizeof(action->file_data_operand.data));
  fifo_pop(fifo, action->file_data_operand.data, action->file_data_operand.provided_data_length);
}

static void parse_op_write_file_data(fifo_t* fifo, alp_action_t* action) {
  parse_operand_file_data(fifo, action);
  DPRINT("parsed write file data file %i, len %i", action->file_data_operand.file_offset.file_id, action->file_data_operand.provided_data_length);
}


static void parse_op_return_file_data(fifo_t* fifo, alp_action_t* action) {
  parse_operand_file_data(fifo, action);
  DPRINT("parsed return file data file %i, len %i", action->file_data_operand.file_offset.file_id, action->file_data_operand.provided_data_length);
}

static void parse_op_return_tag(fifo_t* fifo, alp_action_t* action, bool b6, bool b7) {
  action->tag_response.completed = b7;
  action->tag_response.error = b6;
  assert(fifo_pop(fifo, &action->tag_response.tag_id, 1) == SUCCESS);
  DPRINT("parsed return tag %i, eop %i, err %i", action->tag_response.tag_id, action->tag_response.completed, action->tag_response.error);
}

static void parse_op_return_status(fifo_t* fifo, alp_action_t* action, bool b6, bool b7) {
  assert(b6 && !b7); // TODO implement action status
  uint8_t itf_id;
  assert(fifo_pop(fifo, &itf_id, 1) == SUCCESS);
  // TODO uint32_t itf_len = parse_length_operand(fifo);
  // assert(itf_len == sizeof(d7ap_session_result_t));
  action->status.itf_id=itf_id;
  if (itf_id == ALP_ITF_ID_D7ASP)
  {
    d7ap_session_result_t* interface_status =  ((d7ap_session_result_t*)action->status.itf_status);
    fifo_skip(fifo, 1); //size
    fifo_pop(fifo, &interface_status->channel.channel_header, 1);
    fifo_pop(fifo, (uint8_t*)&interface_status->channel.center_freq_index, 2);
    interface_status->channel.center_freq_index = __builtin_bswap16(interface_status->channel.center_freq_index);
    fifo_pop(fifo, &interface_status->rx_level, 1);
    fifo_pop(fifo, &interface_status->link_budget, 1);
    fifo_pop(fifo, &interface_status->target_rx_level, 1);
    fifo_pop(fifo, &interface_status->status.raw, 1);
    fifo_pop(fifo, &interface_status->fifo_token, 1);
    fifo_pop(fifo, &interface_status->seqnr, 1);
    fifo_pop(fifo, &interface_status->response_to, 1);
    fifo_pop(fifo, &interface_status->addressee.ctrl.raw, 1);
    fifo_pop(fifo, &interface_status->addressee.access_class, 1);
    uint8_t addressee_len = d7ap_addressee_id_length(interface_status->addressee.ctrl.id_type);
    assert(fifo_pop(fifo, (uint8_t*)&interface_status->addressee.id, addressee_len) == SUCCESS);
  }
  else if ( (itf_id == ALP_ITF_ID_LORAWAN_OTAA) || (itf_id == ALP_ITF_ID_LORAWAN_ABP))
  {
    lorawan_session_result_t* interface_status = ((lorawan_session_result_t*)action->status.itf_status);
    fifo_skip(fifo,1); //size
    fifo_pop(fifo, &interface_status->attempts, 1);
    fifo_pop(fifo, (uint8_t*)&interface_status->error_state, 1);
    fifo_pop(fifo, (uint8_t*)&interface_status->duty_cycle_wait_time,2);
    interface_status->duty_cycle_wait_time= __builtin_bswap16(interface_status->duty_cycle_wait_time);
  }

  DPRINT("parsed interface status");
}

void alp_parse_action(fifo_t* fifo, alp_action_t* action) {
  uint8_t op;
  assert(fifo_pop(fifo, &op, 1) == SUCCESS);
  bool b6 = (op >> 6) & 1;
  bool b7 = op >> 7;
  op &= 0x3F; // op is in b5-b0
  action->operation = op;
  switch(op) {
    case ALP_OP_WRITE_FILE_DATA:
      parse_op_write_file_data(fifo, action);
      break;
    case ALP_OP_RETURN_FILE_DATA:
      parse_op_return_file_data(fifo, action);
      break;
    case ALP_OP_RESPONSE_TAG:
      parse_op_return_tag(fifo, action, b6, b7);
      break;
    case ALP_OP_STATUS:
      parse_op_return_status(fifo, action, b6, b7);
      break;
    default:
      DPRINT("op %x not implemented", op);
      assert(false);
  }

  DPRINT("parsed action");
}

uint8_t alp_get_expected_response_length(fifo_t fifo) {
  uint8_t expected_response_length = 0;

  while(fifo_get_size(&fifo) > 0) {
    alp_control_t control;
    fifo_pop(&fifo, (uint8_t*)&control.raw, 1);
    switch(control.operation) {
      case ALP_OP_READ_FILE_DATA:
        fifo_skip(&fifo, 1); // skip file ID
        uint32_t offset = alp_parse_length_operand(&fifo); // offset
        expected_response_length += alp_parse_length_operand(&fifo); // the file length
        expected_response_length += alp_length_operand_coded_length(expected_response_length); // the length of the provided data operand
        expected_response_length += alp_length_operand_coded_length(offset) + 1; // the length of the offset operand
        expected_response_length += 1; // the opcode
        break;
      case ALP_OP_READ_FILE_PROPERTIES:
        fifo_skip(&fifo, 1); //skip file ID
        break;
      case ALP_OP_REQUEST_TAG:
      case ALP_OP_RESPONSE_TAG:
        fifo_skip(&fifo, 1); // skip tag ID operand
        break;
      case ALP_OP_RETURN_FILE_DATA:
      case ALP_OP_WRITE_FILE_DATA:
        fifo_skip(&fifo, 1); // skip file ID
        alp_parse_length_operand(&fifo); // offset
        fifo_skip(&fifo, alp_parse_length_operand(&fifo));
        break;
      case ALP_OP_FORWARD: ;
        uint8_t itf_id;
        fifo_pop(&fifo, &itf_id, 1);
        for(uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
          if((interfaces[i] != NULL) && (itf_id == interfaces[i]->itf_id)) {
            if(interfaces[i]->itf_id == ALP_ITF_ID_D7ASP) {
              fifo_skip(&fifo, 2);
              d7ap_addressee_ctrl_t addressee_ctrl;
              fifo_pop(&fifo, (uint8_t*)&addressee_ctrl.raw, 1);
              fifo_skip(&fifo, 1 + d7ap_addressee_id_length(addressee_ctrl.id_type)); // skip addressee ctrl, access class
            } else 
              fifo_skip(&fifo, interfaces[i]->itf_cfg_len);
          }
        }
        break;
      case ALP_OP_INDIRECT_FORWARD: ;
        fifo_skip(&fifo, 1);
        if(control.b7 && (itf_id == ALP_ITF_ID_D7ASP))
          fifo_skip(&fifo, 10);
        break;
      case ALP_OP_WRITE_FILE_PROPERTIES:
      case ALP_OP_CREATE_FILE:
      case ALP_OP_RETURN_FILE_PROPERTIES:
        fifo_skip(&fifo, 1 + sizeof(d7ap_fs_file_header_t)); // skip file ID & header
        break;
      case ALP_OP_BREAK_QUERY:
        fifo_skip(&fifo, 1);
        fifo_skip(&fifo, (uint16_t)alp_parse_length_operand(&fifo));
        fifo_skip(&fifo, 1);
        alp_parse_length_operand(&fifo);
        break;
      case ALP_OP_STATUS:
          if (!control.b6 && !control.b7) {
              // action status
              fifo_skip(&fifo, 1); // skip status code
          } else if (control.b6 && !control.b7) {
              // interface status
              fifo_skip(&fifo, 1); // skip status code
              fifo_skip(&fifo, (uint16_t)alp_parse_length_operand(&fifo)); // itf_status_len + itf status
          } else {
              assert(false); // TODO
          }

          break;
      // TODO other operations
      default:
          DPRINT("!!op %i not implemented", control.raw);
        DPRINT("op %i not implemented", control.operation);
        assert(false);
    }
  }

  DPRINT("Expected ALP response length=%i", expected_response_length);
  return expected_response_length;
}

void alp_append_tag_request_action(fifo_t* fifo, uint8_t tag_id, bool eop) {
  DPRINT("append tag %i", tag_id);
  uint8_t op = ALP_OP_REQUEST_TAG | (eop << 7);
  assert(fifo_put_byte(fifo, op) == SUCCESS);
  assert(fifo_put_byte(fifo, tag_id) == SUCCESS);
}

void alp_append_read_file_data_action(fifo_t* fifo, uint8_t file_id, uint32_t offset, uint32_t length, bool resp, bool group) {
  uint8_t op = ALP_OP_READ_FILE_DATA | (resp << 6) | (group << 7);
  assert(fifo_put_byte(fifo, op) == SUCCESS);
  alp_append_file_offset_operand(fifo, file_id, offset);
  alp_append_length_operand(fifo, length);
}

void alp_append_write_file_data_action(fifo_t* fifo, uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data, bool resp, bool group) {
  uint8_t op = ALP_OP_WRITE_FILE_DATA | (resp << 6) | (group << 7);
  assert(fifo_put_byte(fifo, op) == SUCCESS);
  alp_append_file_offset_operand(fifo, file_id, offset);
  alp_append_length_operand(fifo, length);
  assert(fifo_put(fifo, data, length) == SUCCESS);
}

void alp_append_interface_status(fifo_t* fifo, alp_interface_status_t* status) {
    fifo_put_byte(fifo, ALP_OP_STATUS + (1 << 6));
    fifo_put(fifo, (uint8_t*)status, status->len + 2);
}

void alp_append_create_new_file_data_action(fifo_t* fifo, uint8_t file_id, uint32_t length, fs_storage_class_t storage_class, bool resp, bool group) {
  uint8_t op = ALP_OP_CREATE_FILE | (resp << 6) | (group << 7);
  assert(fifo_put_byte(fifo, op) == SUCCESS);
  alp_operand_file_header_t header = {
    .file_id = file_id,
    .file_header = {
      .file_permissions = 0,
      .file_properties.action_protocol_enabled = 0,
      .file_properties.storage_class = storage_class,
      .length = __builtin_bswap32(length),
      .allocated_length = __builtin_bswap32(length)
    }
  };
  assert(fifo_put(fifo, (uint8_t*)&header, sizeof(alp_operand_file_header_t)) == SUCCESS);
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
