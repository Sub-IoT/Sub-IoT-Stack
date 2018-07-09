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

#include "alp_layer.h"
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
#define DPRINT_DATA(p, n) log_print_data(p, n)
#else
#define DPRINT(...)
#define DPRINT_DATA(p, n)
#endif

static bool NGDEF(_shell_enabled);
#define shell_enabled NG(_shell_enabled)

typedef struct {
  bool is_active;
  uint8_t fifo_token;
  uint8_t request_id;
  uint8_t tag_id;
  bool respond_when_completed;
  alp_command_origin_t origin;
  fifo_t alp_command_fifo;
  fifo_t alp_response_fifo;
  uint8_t alp_command[ALP_PAYLOAD_MAX_SIZE];
  uint8_t alp_response[ALP_PAYLOAD_MAX_SIZE];
} alp_command_t;

static alp_command_t NGDEF(_commands)[MODULE_D7AP_ALP_MAX_ACTIVE_COMMAND_COUNT];
#define commands NG(_commands)

static d7ap_session_result_t NGDEF(_current_d7asp_result);
#define current_d7asp_result NG(_current_d7asp_result)

static alp_init_args_t* NGDEF(_init_args);
#define init_args NG(_init_args)

static void free_command(alp_command_t* command) {
  DPRINT("Free cmd %i", command->fifo_token);
  command->is_active = false;
  fifo_init(&command->alp_command_fifo, command->alp_command, ALP_PAYLOAD_MAX_SIZE);
  fifo_init(&command->alp_response_fifo, command->alp_response, ALP_PAYLOAD_MAX_SIZE);
  // other fields are initialized on usage
};

static void init_commands()
{
  for(uint8_t i = 0; i < MODULE_D7AP_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
    free_command(&commands[i]);
  }
}

static alp_command_t* alloc_command()
{
  for(uint8_t i = 0; i < MODULE_D7AP_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
    if(commands[i].is_active == false) {
      commands[i].is_active = true;
      DPRINT("alloc cmd %p in slot %i", &commands[i], i);
      return &(commands[i]);
    }
  }

  DPRINT("Could not alloc command, all %i reserved slots active", MODULE_D7AP_ALP_MAX_ACTIVE_COMMAND_COUNT);
  return NULL;
}

static alp_command_t* get_command_by_fifo_token(uint8_t fifo_token) {
  for(uint8_t i = 0; i < MODULE_D7AP_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
    if(commands[i].fifo_token == fifo_token && commands[i].is_active) {
      DPRINT("command %i in slot %i", fifo_token, i);
      return &(commands[i]);
    }
  }

  DPRINT("No active command found with fifo_token = %i", fifo_token);
  return NULL;
}

void alp_layer_init(alp_init_args_t* alp_init_args, bool is_shell_enabled)
{
  init_args = alp_init_args;
  shell_enabled = is_shell_enabled;
  init_commands();

  uint8_t read_firmware_version_alp_command[] = { 0x01, D7A_FILE_FIRMWARE_VERSION_FILE_ID, 0, D7A_FILE_FIRMWARE_VERSION_SIZE };
  if(shell_enabled)
  {
#ifdef FRAMEWORK_SHELL_ENABLED
      shell_init();
      shell_register_handler((cmd_handler_registration_t){ .id = ALP_CMD_HANDLER_ID, .cmd_handler_callback = &alp_cmd_handler });
      // alp_cmd_handler_set_appl_itf_callback(alp_cmd_handler_appl_itf_cb); // TODO

      // notify booted to serial
      alp_layer_process_command_console_output(read_firmware_version_alp_command, sizeof(read_firmware_version_alp_command));
#endif
  }
}

static uint8_t process_action(uint8_t* alp_action, uint8_t* alp_response, uint8_t* alp_response_length)
{

}

//static uint32_t parse_length_operand(fifo_t* cmd_fifo) {
//  uint8_t len = 0;
//  fifo_pop(cmd_fifo, (uint8_t*)&len, 1);
//  uint8_t field_len = len >> 6;
//  if(field_len == 0)
//    return (uint32_t)len;

//  uint32_t full_length = (len & 0x3F) << ( 8 * field_len); // mask field length specificier bits and shoft before adding other length bytes
//  fifo_pop(cmd_fifo, (uint8_t*)&full_length, field_len);
//  return full_length;
//}

static void generate_length_operand(fifo_t* cmd_fifo, uint32_t length) {
  if(length < 64) {
    // can be coded in one byte
    fifo_put(cmd_fifo, (uint8_t*)&length, 1);
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
  fifo_put(cmd_fifo, &byte, 1);
  do {
    size--;
    fifo_put(cmd_fifo, (uint8_t*)&length + size, 1);
  } while(size > 0);
}

//static alp_operand_file_offset_t parse_file_offset_operand(fifo_t* cmd_fifo) {
//  alp_operand_file_offset_t operand;
//  error_t err = fifo_pop(cmd_fifo, &operand.file_id, 1); assert(err == SUCCESS);
//  operand.offset = parse_length_operand(cmd_fifo);
//  return operand;
//}

static alp_status_codes_t process_op_read_file_data(alp_command_t* command) {
  alp_operand_file_data_request_t operand;
  error_t err;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  operand.file_offset = alp_parse_file_offset_operand(&command->alp_command_fifo);
  operand.requested_data_length = alp_parse_length_operand(&command->alp_command_fifo);
  DPRINT("READ FILE %i LEN %i", operand.file_offset.file_id, operand.requested_data_length);

  if(operand.requested_data_length <= 0)
    return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error + move to fs_read_file?

  uint8_t data[operand.requested_data_length];
  alp_status_codes_t alp_status = fs_read_file(operand.file_offset.file_id, operand.file_offset.offset, data, operand.requested_data_length);
  if(alp_status == ALP_STATUS_FILE_ID_NOT_EXISTS) {
    // give the application layer the chance to fullfill this request ...
    if(init_args != NULL && init_args->alp_unhandled_read_action_cb != NULL)
      alp_status = init_args->alp_unhandled_read_action_cb(current_d7asp_result, operand, data);
  }

  if(alp_status == ALP_STATUS_OK) {
    // fill response
    alp_append_return_file_data_action(&command->alp_response_fifo, operand.file_offset.file_id, operand.file_offset.offset,
                                       operand.requested_data_length, data);
  }

  return alp_status;
}

static alp_status_codes_t process_op_read_file_properties(alp_command_t* command) {
  uint8_t file_id;
  error_t err;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(&command->alp_command_fifo, &file_id, 1); assert(err == SUCCESS);
  DPRINT("READ FILE PROPERTIES %i", file_id);

  fs_file_header_t file_header;
  alp_status_codes_t alp_status = fs_read_file_header(file_id, &file_header);

  if(alp_status == ALP_STATUS_OK) {
    // fill response
    err = fifo_put_byte(&command->alp_response_fifo, ALP_OP_RETURN_FILE_PROPERTIES); assert(err == SUCCESS);
    err = fifo_put_byte(&command->alp_response_fifo, file_id); assert(err == SUCCESS);
    err = fifo_put(&command->alp_response_fifo, (uint8_t*)&file_header, sizeof(fs_file_header_t)); assert(err == SUCCESS);
  }

  return alp_status;
}

static alp_status_codes_t process_op_write_file_properties(alp_command_t* command) {
  uint8_t file_id;
  error_t err;
  fs_file_header_t file_header;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(&command->alp_command_fifo, &file_id, 1); assert(err == SUCCESS);
  err = fifo_pop(&command->alp_command_fifo, (uint8_t*)&file_header, sizeof(fs_file_header_t)); assert(err == SUCCESS);
  DPRINT("WRITE FILE PROPERTIES %i", file_id);

  return fs_write_file_header(file_id, &file_header);
}

static alp_status_codes_t process_op_write_file_data(alp_command_t* command) {
  alp_operand_file_data_t operand;
  error_t err;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  operand.file_offset = alp_parse_file_offset_operand(&command->alp_command_fifo);
  operand.provided_data_length = alp_parse_length_operand(&command->alp_command_fifo);
  DPRINT("WRITE FILE %i LEN %i", operand.file_offset.file_id, operand.provided_data_length);

  uint8_t data[operand.provided_data_length];
  err = fifo_pop(&command->alp_command_fifo, data, operand.provided_data_length);
  return fs_write_file(operand.file_offset.file_id, operand.file_offset.offset, data, operand.provided_data_length);
}

bool process_arithm_predicate(uint8_t* value1, uint8_t* value2, uint32_t len, alp_query_arithmetic_comparison_type_t comp_type) {
  // TODO assuming unsigned for now
  DPRINT("ARITH PREDICATE COMP TYPE %i LEN %i", comp_type, len);
  // first check for equality/inequality
  bool is_equal = memcmp(value1, value2, len) == 0;
  if(is_equal) {
    if(comp_type == ARITH_COMP_TYPE_EQUALITY || comp_type == ARITH_COMP_TYPE_GREATER_THAN_OR_EQUAL_TO || comp_type == ARITH_COMP_TYPE_LESS_THAN_OR_EQUAL_TO)
      return true;
    else
      return false;
  } else if(comp_type == ARITH_COMP_TYPE_INEQUALITY) {
    return true;
  }

  // since we don't know length in advance compare byte per byte starting from MSB
  for(uint32_t i = 0; i < len; i++) {
    if(value1[i] == value2[i])
      continue;

    if(value1[i] > value2[i] && (comp_type == ARITH_COMP_TYPE_GREATER_THAN || comp_type == ARITH_COMP_TYPE_GREATER_THAN_OR_EQUAL_TO))
      return true;
    else if(value1[i] < value2[i] && (comp_type == ARITH_COMP_TYPE_LESS_THAN || comp_type == ARITH_COMP_TYPE_LESS_THAN_OR_EQUAL_TO))
      return true;
    else
      return false;
  }

  assert(false); // should not reach here
}

static alp_status_codes_t process_op_break_query(alp_command_t* command) {
  uint8_t query_code;
  error_t err;
  DPRINT("BREAK QUERY");
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(&command->alp_command_fifo, &query_code, 1); assert(err == SUCCESS);
  assert((query_code & 0xE0) == 0x40); // TODO only arithm comp with value type is implemented for now
  assert((query_code & 0x10) == 0); // TODO mask value not implemented for now

  // parse arithm query params
  bool use_signed_comparison = true;
  if((query_code & 0x08) == 0)
    use_signed_comparison = false;

  alp_query_arithmetic_comparison_type_t comp_type = query_code & 0x07;
  uint32_t comp_length = alp_parse_length_operand(&command->alp_command_fifo);
  // TODO assuming no compare mask for now + assume compare value present + only 1 file offset operand

  uint8_t value[comp_length];
  memset(value, 0, comp_length);
  err = fifo_pop(&command->alp_command_fifo, value, comp_length); assert(err == SUCCESS);
  alp_operand_file_offset_t offset_a = alp_parse_file_offset_operand(&command->alp_command_fifo);

  uint8_t file_value[comp_length];
  fs_read_file(offset_a.file_id, offset_a.offset, file_value, comp_length);

  bool success = process_arithm_predicate(file_value, value, comp_length, comp_type);
  DPRINT("predicate result: %i", success);

  if(!success) {
    DPRINT("predicate failed, clearing ALP command to stop further processing");
    fifo_clear(&command->alp_command_fifo);
  }

  return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_forward(alp_command_t* command, uint8_t* itf_id, d7ap_master_session_config_t* session_config) {
  // TODO move session config to alp_command_t struct
  error_t err;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(&command->alp_command_fifo, itf_id, 1); assert(err == SUCCESS);
  switch(*itf_id) {
    case ALP_ITF_ID_D7ASP:
      err = fifo_pop(&command->alp_command_fifo, &session_config->qos.raw, 1); assert(err == SUCCESS);
      err = fifo_pop(&command->alp_command_fifo, &session_config->dormant_timeout, 1); assert(err == SUCCESS);
      err = fifo_pop(&command->alp_command_fifo, &session_config->addressee.ctrl.raw, 1); assert(err == SUCCESS);
      uint8_t id_length = alp_addressee_id_length(session_config->addressee.ctrl.id_type);
      err = fifo_pop(&command->alp_command_fifo, &session_config->addressee.access_class, 1); assert(err == SUCCESS);
      err = fifo_pop(&command->alp_command_fifo, session_config->addressee.id, id_length); assert(err == SUCCESS);
      DPRINT("FORWARD D7ASP");
      break;
    case ALP_ITF_ID_SERIAL:
      // no configuration
      DPRINT("FORWARD SERIAL");
      break;
    default:
      DPRINT("unsupported ITF %i", itf_id);
      assert(false);
  }

  return ALP_STATUS_PARTIALLY_COMPLETED;
}

static alp_status_codes_t process_op_request_tag(alp_command_t* command, bool respond_when_completed) {
  error_t err;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(&command->alp_command_fifo, &command->tag_id, 1); assert(err == SUCCESS);
  command->respond_when_completed = respond_when_completed;
  return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_return_file_data(alp_command_t* command) {
//  alp_control_tag_response_t crtl;
//  alp_operand_file_data_request_t operand;

//  fifo_pop(&command->alp_command_fifo, &crtl, 1); //get control byte
//  operand.file_offset = alp_parse_file_offset_operand(&command->alp_command_fifo);
//  operand.requested_data_length = alp_parse_length_operand(&command->alp_command_fifo);

  // determine total size of alp
  uint8_t total_len = 2; // ALP control byte + File ID byte
  uint8_t length_field;
  uint8_t len; // b7 b6 of length field
  uint32_t data_len;

  //read length field of file offset
  fifo_peek(&command->alp_command_fifo, &length_field, total_len, 1);
  len = (length_field >> 6);
  DPRINT("file offset length: %d", len + 1);
  total_len += len + 1;

  //read length field of data length data
  fifo_peek(&command->alp_command_fifo, &length_field, total_len, 1);
  len = (length_field >> 6);
  DPRINT("data length length: %d", len + 1);

  //read data lenght data
  data_len = (length_field & 0x3F) << ( 8 * len); // mask field length specificier bits and shift before adding other length bytes
  fifo_peek(&command->alp_command_fifo, (uint8_t*) &data_len, total_len + 1, len); //ofset is one byte further as we have read the first byte

  DPRINT("data length: %d", data_len);
  total_len += len + 1 + data_len;

  uint8_t alp_response[total_len];
  fifo_pop(&command->alp_command_fifo, alp_response, total_len);

  if(shell_enabled)
    alp_cmd_handler_output_d7asp_response(current_d7asp_result, alp_response, total_len);

  if(init_args != NULL && init_args->alp_received_unsolicited_data_cb != NULL)
    init_args->alp_received_unsolicited_data_cb(current_d7asp_result, alp_response, total_len);

  return ALP_STATUS_OK;
}

static void add_tag_response(alp_command_t* command, bool eop, bool error) {
  // fill response with tag response
  DPRINT("add_tag_response %i", command->tag_id);
  uint8_t op_return_tag = ALP_OP_RETURN_TAG | (eop << 7);
  op_return_tag |= (error << 6);
  error_t err = fifo_put_byte(&command->alp_response_fifo, op_return_tag); assert(err == SUCCESS);
  err = fifo_put_byte(&command->alp_response_fifo, command->tag_id); assert(err == SUCCESS);
}

void alp_layer_process_d7aactp(d7ap_master_session_config_t* session_config, uint8_t* alp_command, uint8_t alp_command_length)
{
  uint8_t alp_result_length = 0;
  // TODO refactor
  alp_command_t* command = alloc_command();
  assert(command != NULL);
  alp_layer_process_command(alp_command, alp_command_length, command->alp_command, &alp_result_length, ALP_CMD_ORIGIN_D7AACTP);
  if(alp_result_length == 0) {
    free_command(command);
    return;
  }

  d7asp_master_session_t* session = d7asp_master_session_create(session_config);
  uint8_t expected_response_length = alp_get_expected_response_length(command->alp_command, alp_result_length);
  d7asp_queue_result_t result = d7asp_queue_alp_actions(session, command->alp_command, alp_result_length, expected_response_length);
  command->fifo_token = result.fifo_token;
  command->request_id = result.request_id;
}

void alp_layer_process_command_console_output(uint8_t* alp_command, uint8_t alp_command_length) {
  DPRINT("ALP command recv from console length=%i", alp_command_length);
  DPRINT_DATA(alp_command, alp_command_length);
  uint8_t alp_response_length = 0;
  alp_layer_process_command(alp_command, alp_command_length, NULL, &alp_response_length, ALP_CMD_ORIGIN_SERIAL_CONSOLE);
}

static void add_interface_status_action(fifo_t* alp_response_fifo, d7ap_session_result_t* d7asp_result)
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
  fifo_put_byte(alp_response_fifo, d7asp_result->addressee.ctrl.raw);
  fifo_put_byte(alp_response_fifo, d7asp_result->addressee.access_class);
  uint8_t address_len = alp_addressee_id_length(d7asp_result->addressee.ctrl.id_type);
  fifo_put(alp_response_fifo, d7asp_result->addressee.id, address_len);
}

void alp_layer_process_d7asp_result(uint8_t* alp_command, uint8_t alp_command_length, uint8_t* alp_response, uint8_t* alp_response_length, d7ap_session_result_t d7asp_result)
{
  alp_command_t* command = get_command_by_fifo_token(d7asp_result.fifo_token);
  current_d7asp_result = d7asp_result; // TODO
  if(command != NULL) {
    // received result for known command
    if(shell_enabled) {
      add_interface_status_action(&(command->alp_response_fifo), &d7asp_result);
      fifo_put(&(command->alp_response_fifo), alp_command, alp_command_length);

      // tag and send response already with EOP bit cleared
      add_tag_response(command, false, false); // TODO error
      alp_cmd_handler_output_alp_command(&(command->alp_response_fifo));
      fifo_clear(&(command->alp_response_fifo));
    }

    if(init_args != NULL && init_args->alp_command_result_cb != NULL)
      init_args->alp_command_result_cb(d7asp_result, alp_command, alp_command_length);

    // TODO further bookkeeping
  } else {
    // unknown FIFO token; an incoming request or unsolicited response
    alp_layer_process_command(alp_command, alp_command_length, alp_response, alp_response_length, ALP_CMD_ORIGIN_D7ASP);
  }
}

void alp_layer_execute_command(uint8_t* alp_command, uint8_t alp_command_length, d7ap_master_session_config_t* d7asp_master_session_config) {
  DPRINT("ALP cmd size %i", alp_command_length);
  assert(alp_command_length <= ALP_PAYLOAD_MAX_SIZE);

  alp_command_t* command = alloc_command(); // TODO check if we have a matching open session to append to
  assert(command != NULL); // TODO return to app
  d7asp_master_session_t* session = d7asp_master_session_create(d7asp_master_session_config); // TODO store in alp_command_t
  uint8_t expected_response_length = alp_get_expected_response_length(alp_command, alp_command_length);
  d7asp_queue_result_t queue_result = d7asp_queue_alp_actions(session, alp_command, alp_command_length, expected_response_length); // TODO pass fifo directly?
  command->fifo_token = queue_result.fifo_token;
}

// TODO refactor
bool alp_layer_process_command(uint8_t* alp_command, uint8_t alp_command_length, uint8_t* alp_response, uint8_t* alp_response_length, alp_command_origin_t origin)
{
  DPRINT("ALP cmd size %i", alp_command_length);
  assert(alp_command_length <= ALP_PAYLOAD_MAX_SIZE);

  // TODO support more than 1 active cmd
  alp_command_t* command = alloc_command();
  assert(command != NULL); // TODO return error

  memcpy(command->alp_command, alp_command, alp_command_length); // TODO not needed to store this
  fifo_init_filled(&(command->alp_command_fifo), command->alp_command, alp_command_length, ALP_PAYLOAD_MAX_SIZE);
  command->origin = origin;

  (*alp_response_length) = 0;
  d7ap_master_session_config_t d7asp_session_config;
  uint8_t forward_itf_id = ALP_ITF_ID_HOST;

  while(fifo_get_size(&command->alp_command_fifo) > 0) {
    if(forward_itf_id != ALP_ITF_ID_HOST) {
      if(forward_itf_id == ALP_ITF_ID_D7ASP) {
        // forward rest of the actions over the D7ASP interface
        uint8_t forwarded_alp_size = fifo_get_size(&command->alp_command_fifo);
        uint8_t forwarded_alp_actions[forwarded_alp_size];
        fifo_pop(&command->alp_command_fifo, forwarded_alp_actions, forwarded_alp_size);
        // TODO support multiple FIFOs
        d7asp_master_session_t* session = d7asp_master_session_create(&d7asp_session_config);
        // TODO current_command.fifo_token = session->token;
        uint8_t expected_response_length = alp_get_expected_response_length(forwarded_alp_actions, forwarded_alp_size);
        d7asp_queue_result_t queue_result = d7asp_queue_alp_actions(session, forwarded_alp_actions, forwarded_alp_size, expected_response_length); // TODO pass fifo directly?
        command->fifo_token = queue_result.fifo_token;
      } else if(forward_itf_id == ALP_ITF_ID_SERIAL) {
        alp_cmd_handler_output_alp_command(&command->alp_command_fifo);
      } else {
        assert(false);
      }

      break; // TODO return response
    }

    alp_control_t control;
    fifo_peek(&command->alp_command_fifo, &control.raw, 0, 1);
    alp_status_codes_t alp_status;
    switch(control.operation) {
      case ALP_OP_READ_FILE_DATA:
        alp_status = process_op_read_file_data(command);
        break;
      case ALP_OP_READ_FILE_PROPERTIES:
        alp_status = process_op_read_file_properties(command);
        break;
      case ALP_OP_WRITE_FILE_DATA:
        alp_status = process_op_write_file_data(command);
        break;
      case ALP_OP_WRITE_FILE_PROPERTIES:
        alp_status = process_op_write_file_properties(command);
        break;
      case ALP_OP_BREAK_QUERY:
        alp_status = process_op_break_query(command);
        break;
      case ALP_OP_FORWARD:
        alp_status = process_op_forward(command, &forward_itf_id, &d7asp_session_config);
        break;
      case ALP_OP_REQUEST_TAG: ;
        alp_control_tag_request_t* tag_request = (alp_control_tag_request_t*)&control;
        alp_status = process_op_request_tag(command, tag_request->respond_when_completed);
        break;
      case ALP_OP_RETURN_FILE_DATA:
        alp_status = process_op_return_file_data(command);
        break;
      default:
        assert(false); // TODO return error
        //alp_status = ALP_STATUS_UNKNOWN_OPERATION;
    }    
  }

  if(command->origin == ALP_CMD_ORIGIN_SERIAL_CONSOLE) {
    // make sure we include tag response also for commands with interface HOST
    // for interface D7ASP this will be done when flush completes
    if(command->respond_when_completed && forward_itf_id != ALP_ITF_ID_D7ASP)
      add_tag_response(command, true, false); // TODO error

    alp_cmd_handler_output_alp_command(&command->alp_response_fifo);
  }

    // TODO APP
    // TODO return ALP status if requested

//    if(alp_status != ALP_STATUS_OK)
//      return false;
  if(alp_response != NULL) {
    (*alp_response_length) = fifo_get_size(&command->alp_response_fifo);
    memcpy(alp_response, command->alp_response, *alp_response_length);
  }

  if(forward_itf_id == ALP_ITF_ID_HOST)
    free_command(command); // when forwarding the response will arrive async, clean up then

  return true;
}



void alp_layer_d7asp_fifo_flush_completed(uint8_t fifo_token, uint8_t* progress_bitmap, uint8_t* success_bitmap, uint8_t bitmap_byte_count) {
    // TODO end session
    DPRINT("D7ASP flush completed");
    alp_command_t* command = get_command_by_fifo_token(fifo_token);
    assert(command != NULL);
    bool error;

    while(command)
    {
        error = (progress_bitmap[command->request_id] && success_bitmap[command->request_id]) ? 0 : 1;

        if(shell_enabled && command->respond_when_completed) {
            add_tag_response(command, true, error);
            alp_cmd_handler_output_alp_command(&(command->alp_response_fifo));
        }

        if(init_args != NULL && init_args->alp_command_completed_cb != NULL)
            init_args->alp_command_completed_cb(command->tag_id, !error);

        free_command(command);
        command = get_command_by_fifo_token(fifo_token);
    }
}

