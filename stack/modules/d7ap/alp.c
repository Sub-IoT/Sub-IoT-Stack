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
#include "fifo.h"
#include "log.h"
#include "alp_cmd_handler.h"
#include "shell.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_ALP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#else
#define DPRINT(...)
#endif

static bool NGDEF(_shell_enabled);
#define shell_enabled NG(_shell_enabled)

static alp_command_t NGDEF(_current_command); // TODO support multiple active commands
#define current_command NG(_current_command)

static d7asp_result_t NGDEF(_current_d7asp_result);
#define current_d7asp_result NG(_current_d7asp_result)

static alp_init_args_t* NGDEF(_init_args);
#define init_args NG(_init_args)

void alp_init(alp_init_args_t* alp_init_args, bool is_shell_enabled)
{
  init_args = alp_init_args;
  shell_enabled = is_shell_enabled;

  uint8_t read_firmware_version_alp_command[] = { 0x01, D7A_FILE_FIRMWARE_VERSION_FILE_ID, 0, D7A_FILE_FIRMWARE_VERSION_SIZE };
  if(shell_enabled)
  {
#ifdef FRAMEWORK_SHELL_ENABLED
      shell_init();
      shell_register_handler((cmd_handler_registration_t){ .id = ALP_CMD_HANDLER_ID, .cmd_handler_callback = &alp_cmd_handler });
      // alp_cmd_handler_set_appl_itf_callback(alp_cmd_handler_appl_itf_cb); // TODO

      // notify booted to serial
      alp_process_command_console_output(read_firmware_version_alp_command, sizeof(read_firmware_version_alp_command));
#endif
  }
}

alp_operation_t alp_get_operation(uint8_t* alp_command)
{
    alp_control_t alp_ctrl;
    alp_ctrl.raw = (*alp_command);
    return alp_ctrl.operation;
}

static uint8_t process_action(uint8_t* alp_action, uint8_t* alp_response, uint8_t* alp_response_length)
{

}

static alp_status_codes_t process_op_read_file_data(fifo_t* alp_command_fifo, fifo_t* alp_response_fifo) {
  alp_operand_file_data_request_t operand;
  error_t err;
  err = fifo_skip(alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(alp_command_fifo, &operand.file_offset.file_id, 1); assert(err == SUCCESS);
  err = fifo_pop(alp_command_fifo, &operand.file_offset.offset, 1); assert(err == SUCCESS); // TODO can be 1-4 bytes, assume 1 for now
  err = fifo_pop(alp_command_fifo, &operand.requested_data_length, 1); assert(err == SUCCESS);
  DPRINT("READ FILE %i LEN %i", operand.file_offset.file_id, operand.requested_data_length);

  if(operand.requested_data_length <= 0)
    return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error + move to fs_read_file?

  uint8_t data[operand.requested_data_length];
  alp_status_codes_t alp_status = fs_read_file(operand.file_offset.file_id, operand.file_offset.offset, data, operand.requested_data_length);
  if(alp_status == ALP_STATUS_FILE_ID_NOT_EXISTS) {
    // give the application layer the chance to fullfill this request ...
    if(init_args != NULL && init_args->alp_unhandled_read_action_cb != NULL)
      alp_status = init_args->alp_unhandled_read_action_cb(operand, data);
  }

  if(alp_status == ALP_STATUS_OK) {
    // fill response
    err = fifo_put_byte(alp_response_fifo, ALP_OP_RETURN_FILE_DATA); assert(err == SUCCESS);
    err = fifo_put_byte(alp_response_fifo, operand.file_offset.file_id); assert(err == SUCCESS);
    err = fifo_put_byte(alp_response_fifo, operand.file_offset.offset); assert(err == SUCCESS); // TODO can be 1-4 bytes, assume 1 for now
    err = fifo_put_byte(alp_response_fifo, operand.requested_data_length); assert(err == SUCCESS);
    err = fifo_put(alp_response_fifo, data, operand.requested_data_length); assert(err == SUCCESS);
  }

  return alp_status;
}

static alp_status_codes_t process_op_write_file_data(fifo_t* alp_command_fifo, fifo_t* alp_response_fifo) {
  alp_operand_file_data_t operand;
  error_t err;
  err = fifo_skip(alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(alp_command_fifo, &operand.file_offset.file_id, 1); assert(err == SUCCESS);
  err = fifo_pop(alp_command_fifo, &operand.file_offset.offset, 1); assert(err == SUCCESS); // TODO can be 1-4 bytes, assume 1 for now
  err = fifo_pop(alp_command_fifo, &operand.provided_data_length, 1); assert(err == SUCCESS);
  DPRINT("WRITE FILE %i LEN %i", operand.file_offset.file_id, operand.provided_data_length);

  uint8_t data[operand.provided_data_length];
  err = fifo_pop(alp_command_fifo, data, operand.provided_data_length);
  return fs_write_file(operand.file_offset.file_id, operand.file_offset.offset, data, operand.provided_data_length);
}

static alp_status_codes_t process_op_forward(fifo_t* alp_command_fifo, fifo_t* alp_response_fifo, d7asp_master_session_config_t* session_config) {
  uint8_t interface_id;
  error_t err;
  err = fifo_skip(alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(alp_command_fifo, &interface_id, 1); assert(err == SUCCESS);
  assert(interface_id == ALP_ITF_ID_D7ASP); // only D7ASP supported for now // TODO return error instead of asserting
  err = fifo_pop(alp_command_fifo, &session_config->qos.raw, 1); assert(err == SUCCESS);
  err = fifo_pop(alp_command_fifo, &session_config->dormant_timeout, 1); assert(err == SUCCESS);
  err = fifo_pop(alp_command_fifo, &session_config->addressee.ctrl.raw, 1); assert(err == SUCCESS);
  uint8_t id_length = d7anp_addressee_id_length(session_config->addressee.ctrl.id_type);
  err = fifo_pop(alp_command_fifo, &session_config->addressee.access_class, 1); assert(err == SUCCESS);
  err = fifo_pop(alp_command_fifo, session_config->addressee.id, id_length); assert(err == SUCCESS);
  DPRINT("FORWARD");

  return ALP_STATUS_PARTIALLY_COMPLETED;
}

static alp_status_codes_t process_op_request_tag(fifo_t* alp_command_fifo, bool respond_when_completed) {
  error_t err;
  err = fifo_skip(alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(alp_command_fifo, &current_command.tag_id, 1); assert(err == SUCCESS);
  current_command.respond_when_completed = respond_when_completed;
  return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_return_file_data(fifo_t* alp_command_fifo) {
  uint8_t offset_operand_size = 2; // TODO we are assuming offset operand to be 2 bytes for now
  uint8_t total_len = 1 + offset_operand_size; // ALP control byte
  uint8_t requested_data_length_size = 1; // TODO we are assuming requested data length is coded in 1 bytes here, but can be 4
  total_len += requested_data_length_size;
  uint8_t data_len;
  fifo_peek(alp_command_fifo, &data_len, 1 + offset_operand_size, requested_data_length_size);
  total_len += data_len;

  uint8_t alp_response[ALP_PAYLOAD_MAX_SIZE];
  fifo_pop(alp_command_fifo, alp_response, total_len);

  if(shell_enabled)
    alp_cmd_handler_output_d7asp_response(current_d7asp_result, alp_response, total_len);

  if(init_args != NULL && init_args->alp_received_unsolicited_data_cb != NULL)
    init_args->alp_received_unsolicited_data_cb(current_d7asp_result, alp_response, total_len);

  return ALP_STATUS_OK;
}

static void add_tag_response(fifo_t* alp_response_fifo, bool eop, bool error) {
  // fill response with tag response
  uint8_t op_return_tag = ALP_OP_RETURN_TAG | (eop << 7);
  op_return_tag |= (error << 6);
  error_t err = fifo_put_byte(alp_response_fifo, op_return_tag); assert(err == SUCCESS);
  err = fifo_put_byte(alp_response_fifo, current_command.tag_id); assert(err == SUCCESS);
}

void alp_process_command_result_on_d7asp(d7asp_master_session_config_t* session_config, uint8_t* alp_command, uint8_t alp_command_length, alp_command_origin_t origin)
{
  uint8_t alp_response[ALP_PAYLOAD_MAX_SIZE];
  uint8_t alp_response_length = 0;
  alp_process_command(alp_command, alp_command_length, alp_response, &alp_response_length, origin);
  d7asp_master_session_t* session = d7asp_master_session_create(session_config);
  uint8_t expected_response_length = alp_get_expected_response_length(alp_response, alp_response_length);
  d7asp_queue_alp_actions(session, alp_response, alp_response_length, expected_response_length);
}

void alp_process_command_console_output(uint8_t* alp_command, uint8_t alp_command_length) {
  uint8_t alp_response[ALP_PAYLOAD_MAX_SIZE];
  uint8_t alp_response_length = 0;
  DPRINT("ALP command recv from console length=%i", alp_command_length);
  alp_process_command(alp_command, alp_command_length, alp_response, &alp_response_length, ALP_CMD_ORIGIN_SERIAL_CONSOLE);
}

static void add_interface_status_action(fifo_t* alp_response_fifo, d7asp_result_t* d7asp_result)
{
  fifo_put_byte(alp_response_fifo, ALP_OP_RETURN_STATUS + (1 << 6));
  fifo_put_byte(alp_response_fifo, ALP_ITF_ID_D7ASP);
  fifo_put(alp_response_fifo, (uint8_t*) &(d7asp_result->channel), 3); // TODO might need to reorder fields in channel_id
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

void alp_process_d7asp_result(uint8_t* alp_command, uint8_t alp_command_length, uint8_t* alp_response, uint8_t* alp_response_length, d7asp_result_t d7asp_result)
{
  current_d7asp_result = d7asp_result;
  if(d7asp_result.fifo_token == current_command.fifo_token) {
    // answer for current command
    if(shell_enabled) {
      add_interface_status_action(&(current_command.alp_response_fifo), &d7asp_result);
      fifo_put(&(current_command.alp_response_fifo), alp_command, alp_command_length);

      // tag and send response already with EOP bit cleared
      add_tag_response(&(current_command.alp_response_fifo), false, false); // TODO error
      uint8_t alp_response_length = fifo_get_size(&(current_command.alp_response_fifo));
      alp_cmd_handler_output_alp_command(current_command.alp_response, alp_response_length); // TODO pass fifo directly
      fifo_clear(&(current_command.alp_response_fifo));
    }

    if(init_args != NULL && init_args->alp_command_result_cb != NULL)
      init_args->alp_command_result_cb(d7asp_result, alp_command, alp_command_length);

    // TODO further bookkeeping
  } else {
    // uknown FIFO token; an incoming request or unsolicited response
    alp_process_command(alp_command, alp_command_length, alp_response, alp_response_length, ALP_CMD_ORIGIN_D7ASP);
  }
}

void alp_execute_command(uint8_t* alp_command, uint8_t alp_command_length, d7asp_master_session_config_t* d7asp_master_session_config) {
  DPRINT("ALP cmd size %i", alp_command_length);
  assert(alp_command_length <= ALP_PAYLOAD_MAX_SIZE);

  d7asp_master_session_t* session = d7asp_master_session_create(d7asp_master_session_config);
  uint8_t expected_response_length = alp_get_expected_response_length(alp_command, alp_command_length);
  d7asp_queue_result_t queue_result = d7asp_queue_alp_actions(session, alp_command, alp_command_length, expected_response_length); // TODO pass fifo directly?
  current_command.fifo_token = queue_result.fifo_token;
}

// TODO refactor
bool alp_process_command(uint8_t* alp_command, uint8_t alp_command_length, uint8_t* alp_response, uint8_t* alp_response_length, alp_command_origin_t origin)
{
  DPRINT("ALP cmd size %i", alp_command_length);
  assert(alp_command_length <= ALP_PAYLOAD_MAX_SIZE);

  // TODO support more than 1 active cmd
  memcpy(current_command.alp_command, alp_command, alp_command_length);
  fifo_init_filled(&(current_command.alp_command_fifo), current_command.alp_command, alp_command_length, ALP_PAYLOAD_MAX_SIZE);
  fifo_init(&(current_command.alp_response_fifo), current_command.alp_response, ALP_PAYLOAD_MAX_SIZE);
  current_command.origin = origin;

  (*alp_response_length) = 0;
  d7asp_master_session_config_t d7asp_session_config;
  bool do_forward = false;

  fifo_t alp_command_fifo, alp_response_fifo;
  fifo_init_filled(&alp_command_fifo, alp_command, alp_command_length, alp_command_length);
  fifo_init(&alp_response_fifo, alp_response, ALP_PAYLOAD_MAX_SIZE);

  while(fifo_get_size(&alp_command_fifo) > 0) {
    if(do_forward) {
      // forward rest of the actions over the D7ASP interface
      // TODO support multiple FIFOs
      uint8_t forwarded_alp_size = fifo_get_size(&alp_command_fifo);
      uint8_t forwarded_alp_actions[forwarded_alp_size];
      fifo_pop(&alp_command_fifo, forwarded_alp_actions, forwarded_alp_size);
      d7asp_master_session_t* session = d7asp_master_session_create(&d7asp_session_config);
      // TODO current_command.fifo_token = session->token;
      uint8_t expected_response_length = alp_get_expected_response_length(forwarded_alp_actions, forwarded_alp_size);
      d7asp_queue_result_t queue_result = d7asp_queue_alp_actions(session, forwarded_alp_actions, forwarded_alp_size, expected_response_length); // TODO pass fifo directly?
      current_command.fifo_token = queue_result.fifo_token;

      break; // TODO return response
    }

    alp_control_t control;
    fifo_peek(&alp_command_fifo, &control.raw, 0, 1);
    alp_status_codes_t alp_status;
    switch(control.operation) {
      case ALP_OP_READ_FILE_DATA:
        alp_status = process_op_read_file_data(&alp_command_fifo, &alp_response_fifo);
        break;
      case ALP_OP_WRITE_FILE_DATA:
        alp_status = process_op_write_file_data(&alp_command_fifo, &alp_response_fifo);
        break;
      case ALP_OP_FORWARD:
        alp_status = process_op_forward(&alp_command_fifo, &alp_response_fifo, &d7asp_session_config);
        do_forward = true;
        break;
      case ALP_OP_REQUEST_TAG: ;
        alp_control_tag_request_t* tag_request = (alp_control_tag_request_t*)&control;
        alp_status = process_op_request_tag(&alp_command_fifo, tag_request->respond_when_completed);
        break;
      case ALP_OP_RETURN_FILE_DATA:
        alp_status = process_op_return_file_data(&alp_command_fifo);
        break;
      default:
        assert(false); // TODO return error
        //alp_status = ALP_STATUS_UNKNOWN_OPERATION;
    }    
  }

  if(current_command.origin == ALP_CMD_ORIGIN_SERIAL_CONSOLE) {
    // make sure we include tag response also for commands with interface HOST
    // for interface D7ASP this will be done when flush completes
    if(current_command.respond_when_completed && !do_forward)
      add_tag_response(&alp_response_fifo, true, false); // TODO error

    (*alp_response_length) = fifo_get_size(&alp_response_fifo);
    if((*alp_response_length) > 0) {
      alp_cmd_handler_output_alp_command(alp_response, (*alp_response_length));
    }
  }

    // TODO APP
    // TODO return ALP status if requested

//    if(alp_status != ALP_STATUS_OK)
//      return false;

  (*alp_response_length) = fifo_get_size(&alp_response_fifo);
  return true;
}



void alp_d7asp_fifo_flush_completed(uint8_t fifo_token, uint8_t* progress_bitmap, uint8_t* success_bitmap, uint8_t bitmap_byte_count) {
  // TODO end session
  DPRINT("D7ASP flush completed");
  bool error = memcmp(success_bitmap, progress_bitmap, bitmap_byte_count) != 0;
if(shell_enabled && current_command.respond_when_completed) {
    add_tag_response(&(current_command.alp_response_fifo), true, error);
    uint8_t alp_response_length = fifo_get_size(&(current_command.alp_response_fifo));
    alp_cmd_handler_output_alp_command(current_command.alp_response, alp_response_length); // TODO pass fifo directly
  }

  if(init_args != NULL && init_args->alp_command_completed_cb != NULL)
    init_args->alp_command_completed_cb(fifo_token, !error);
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
      // TODO other operations
      default:
        assert(false);
    }
  }

  DPRINT("Expected ALP response length=%i", expected_response_length);
  return expected_response_length;
}

