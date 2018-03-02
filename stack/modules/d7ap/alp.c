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
#include "ng.h"

#include "alp.h"
#include "packet.h"
#include "fs.h"
#include "fifo.h"
#include "log.h"
#include "alp_cmd_handler.h"
#include "shell.h"
#include "MODULE_D7AP_defs.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_ALP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#else
#define DPRINT(...)
#endif


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

static alp_operand_file_offset_t parse_file_offset_operand(fifo_t* cmd_fifo) {
  alp_operand_file_offset_t operand;
  error_t err = fifo_pop(cmd_fifo, &operand.file_id, 1); assert(err == SUCCESS);
  operand.offset = alp_parse_length_operand(cmd_fifo);
  return operand;
}

void alp_append_file_offset_operand(fifo_t* fifo, uint8_t file_id, uint32_t offset) {
  assert(fifo_put_byte(fifo, file_id) == SUCCESS);
  alp_append_length_operand(fifo, offset);
}


static void append_tag_response(fifo_t* fifo, uint8_t tag_id, bool eop, bool error) {
  // fill response with tag response
  uint8_t op_return_tag = ALP_OP_RETURN_TAG | (eop << 7);
  op_return_tag |= (error << 6);
  error_t err = fifo_put_byte(fifo, op_return_tag); assert(err == SUCCESS);
  err = fifo_put_byte(fifo, tag_id); assert(err == SUCCESS);
}


static void add_interface_status_action(fifo_t* alp_response_fifo, d7asp_result_t* d7asp_result)
{
  fifo_put_byte(alp_response_fifo, ALP_OP_RETURN_STATUS + (1 << 6));
  fifo_put_byte(alp_response_fifo, ALP_ITF_ID_D7ASP);
  fifo_put_byte(alp_response_fifo, d7asp_result->channel.channel_header_raw);
  uint16_t center_freq_index_be = __builtin_bswap16(d7asp_result->channel.center_freq_index);
  fifo_put(alp_response_fifo, (uint8_t*)&center_freq_index_be, 2);
  fifo_put_byte(alp_response_fifo, d7asp_result->rx_level);
  fifo_put_byte(alp_response_fifo, d7asp_result->link_budget);
  fifo_put_byte(alp_response_fifo, d7asp_result->target_rx_level);
  fifo_put_byte(alp_response_fifo, d7asp_result->status.raw);
  fifo_put_byte(alp_response_fifo, d7asp_result->fifo_token);
  fifo_put_byte(alp_response_fifo, d7asp_result->seqnr);
  fifo_put_byte(alp_response_fifo, d7asp_result->response_to);
  fifo_put_byte(alp_response_fifo, d7asp_result->addressee->ctrl.raw);
  fifo_put_byte(alp_response_fifo, d7asp_result->addressee->access_class);
  uint8_t address_len = d7anp_addressee_id_length(d7asp_result->addressee->ctrl.id_type);
  fifo_put(alp_response_fifo, d7asp_result->addressee->id, address_len);
}

uint8_t alp_get_expected_response_length(uint8_t* alp_command, uint8_t alp_command_length) {
  uint8_t expected_response_length = 0;
  uint8_t* ptr = alp_command;

  while(ptr < alp_command + alp_command_length) {
    alp_control_t control;
    control.raw = (*ptr);
    ptr++; // skip control byte
    switch(control.operation) {
      case ALP_OP_READ_FILE_DATA:
        ptr += 2; // skip file offset operand // TODO we assume 2 bytes now but can be 2-5 bytes
        expected_response_length += *ptr; ptr++; // TODO we assume length is coded in 1 byte but can be 4
        // TODO
        break;
      case ALP_OP_REQUEST_TAG:
        ptr += 1; // skip tag ID operand
        break;
      case ALP_OP_RETURN_FILE_DATA:
      case ALP_OP_WRITE_FILE_DATA:
        ptr += 2; // skip file offset operand // TODO we assume 2 bytes now but can be 2-5 bytes
        uint8_t data_length = *ptr;
        ptr += 1; // skip data length field // TODO we assume length is coded in 1 byte but can be 4
        ptr += data_length; // skip data
        break;
      case ALP_OP_FORWARD:
        ptr += 1; // skip interface ID
        ptr += 1; // skip QoS
        ptr += 1; // skip dormant
        d7anp_addressee_ctrl addressee_ctrl;
        addressee_ctrl.raw = *ptr;
        ptr += 1; // skip addressee ctrl
        ptr += 1; // skip access class
        ptr += d7anp_addressee_id_length(addressee_ctrl.id_type); // skip address
        // TODO refactor to reuse same logic for parsing and response length counting
        break;
      case ALP_OP_WRITE_FILE_PROPERTIES:
        ptr += 1 + sizeof(fs_file_header_t); // skip file ID & header
        break;
      // TODO other operations
      default:
        assert(false);
    }
  }

  DPRINT("Expected ALP response length=%i", expected_response_length);
  return expected_response_length;
}

void alp_append_tag_request(fifo_t* fifo, uint8_t tag_id, bool eop) {
  uint8_t op = ALP_OP_REQUEST_TAG | (eop << 7);
  assert(fifo_put_byte(fifo, op) == SUCCESS);
  assert(fifo_put_byte(fifo, tag_id) == SUCCESS);
}

void alp_append_read_file_data(fifo_t* fifo, uint8_t file_id, uint32_t offset, uint32_t length, bool resp, bool group) {
  uint8_t op = ALP_OP_READ_FILE_DATA | (resp << 6) | (group << 7);
  assert(fifo_put_byte(fifo, op) == SUCCESS);
  alp_append_file_offset_operand(fifo, file_id, offset);
  alp_append_length_operand(fifo, length);
}
