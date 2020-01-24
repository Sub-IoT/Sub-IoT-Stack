/*! \file alp.c
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *  Copyright 2018 CORTUS S.A
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
 *  \author philippe.nunes@cortus.com
 */

#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "ng.h"

#include "alp.h"
#include "dae.h"
#include "fifo.h"
#include "log.h"
#include "shell.h"
#include "timer.h"
#include "modules_defs.h"
#include "MODULE_ALP_defs.h"


#ifdef MODULE_D7AP
#include "d7ap.h"
#endif

#include "d7ap_fs.h"
#include "lorawan_stack.h"

#include "alp_layer.h"
#include "serial_interface.h"

#include "platform_defs.h"
#include "platform.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_ALP_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_ALP, __VA_ARGS__)
#define DPRINT_DATA(p, n) log_print_data(p, n)
#else
#define DPRINT(...)
#define DPRINT_DATA(p, n)
#endif

static alp_itf_id_t current_lorawan_interface_type = ALP_ITF_ID_LORAWAN_OTAA;
static interface_deinit current_itf_deinit = NULL;

bool use_serial_itf;

typedef struct {
  bool is_active;
  uint16_t trans_id;
  uint8_t tag_id;
  bool respond_when_completed;
  alp_itf_id_t origin_itf_id;
  fifo_t alp_command_fifo;
  fifo_t alp_response_fifo;
  uint8_t alp_command[ALP_PAYLOAD_MAX_SIZE];
  uint8_t alp_response[ALP_PAYLOAD_MAX_SIZE];
} alp_command_t;

static alp_command_t NGDEF(_commands)[MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT];
#define commands NG(_commands)

static alp_interface_status_t NGDEF( current_status);
#define current_status NG(current_status)

static alp_init_args_t* NGDEF(_init_args);
#define init_args NG(_init_args)

static uint8_t alp_client_id = 0;
static timer_event alp_layer_process_command_timer;

static uint8_t previous_interface_file_id = 0;
static bool interface_file_changed = true;
static alp_interface_config_t session_config_saved;
static uint8_t alp_data[ALP_PAYLOAD_MAX_SIZE]; // temp buffer statically allocated to prevent runtime stackoverflows
static uint8_t alp_data2[ALP_PAYLOAD_MAX_SIZE]; // temp buffer statically allocated to prevent runtime stackoverflows
static alp_operand_file_data_t file_data_operand; // statically allocated to prevent runtime stackoverflows

extern alp_interface_t* interfaces[MODULE_ALP_INTERFACE_SIZE];
static alp_interface_t serial_interface;
static alp_interface_config_t* session_config_buffer;
static bool expect_completed = false;

static void _async_process_command(void* arg);
static void alp_layer_lorawan_init();
static void lorawan_error_handler(uint16_t* trans_id, lorawan_stack_status_t status);

static void free_command(alp_command_t* command) {
  DPRINT("Free cmd %02x", command->trans_id);
  memset(command, 0, sizeof (alp_command_t));
  command->is_active = false;
  fifo_init(&command->alp_command_fifo, command->alp_command, ALP_PAYLOAD_MAX_SIZE);
  fifo_init(&command->alp_response_fifo, command->alp_response, ALP_PAYLOAD_MAX_SIZE);
  // other fields are initialized on usage
}

static void init_commands()
{
  for(uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
    free_command(&commands[i]);
  }
}

static alp_command_t* alloc_command()
{
  for(uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
    if(commands[i].is_active == false) {
      commands[i].is_active = true;
      DPRINT("alloc cmd %p in slot %i", &commands[i], i);
      return &(commands[i]);
    }
  }

  DPRINT("Could not alloc command, all %i reserved slots active", MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT);
  return NULL;
}

static alp_command_t* get_command_by_transid(uint16_t trans_id) {
  for(uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
    if(commands[i].trans_id == trans_id && commands[i].is_active) {
        DPRINT("command trans Id %i in slot %i", trans_id, i);  
        return &(commands[i]);
    }
  }

  DPRINT("No active command found with transaction Id = %i", trans_id);
  return NULL;
}

void alp_layer_init(alp_init_args_t* alp_init_args, bool use_serial_interface)
{
  init_args = alp_init_args;
  use_serial_itf = use_serial_interface;
  init_commands();

  if (use_serial_itf)
    serial_interface_register();

#ifdef MODULE_LORAWAN
  alp_layer_lorawan_init();
#endif

  timer_init_event(&alp_layer_process_command_timer, &_async_process_command);
}

void alp_layer_register_interface(alp_interface_t* interface) {
  alp_register_interface(interface);
}

static alp_status_codes_t process_op_read_file_data(alp_command_t* command) {
  alp_operand_file_data_request_t operand;
  error_t err;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  operand.file_offset = alp_parse_file_offset_operand(&command->alp_command_fifo);
  operand.requested_data_length = alp_parse_length_operand(&command->alp_command_fifo);
  DPRINT("READ FILE %i LEN %i", operand.file_offset.file_id, operand.requested_data_length);

  if(operand.requested_data_length <= 0 || operand.requested_data_length > ALP_PAYLOAD_MAX_SIZE)
    return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error + move to fs_read_file?

  int rc = d7ap_fs_read_file(operand.file_offset.file_id, operand.file_offset.offset, alp_data, operand.requested_data_length);
  if(rc == -ENOENT) {
    // give the application layer the chance to fullfill this request ...
    if(init_args != NULL && init_args->alp_unhandled_read_action_cb != NULL)
      rc = init_args->alp_unhandled_read_action_cb(&current_status, operand, alp_data);
  }

  if(rc == 0) {
    // fill response
    alp_append_return_file_data_action(&command->alp_response_fifo, operand.file_offset.file_id, operand.file_offset.offset,
                                       operand.requested_data_length, alp_data);
  }

  return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_read_file_properties(alp_command_t* command) {
  uint8_t file_id;
  error_t err;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(&command->alp_command_fifo, &file_id, 1); assert(err == SUCCESS);
  DPRINT("READ FILE PROPERTIES %i", file_id);

  d7ap_fs_file_header_t file_header;
  alp_status_codes_t alp_status = d7ap_fs_read_file_header(file_id, &file_header);

  // convert to big endian
  file_header.length = __builtin_bswap32(file_header.length);
  file_header.allocated_length = __builtin_bswap32(file_header.allocated_length);

  if(alp_status == ALP_STATUS_OK) {
    // fill response
    err = fifo_put_byte(&command->alp_response_fifo, ALP_OP_RETURN_FILE_PROPERTIES); assert(err == SUCCESS);
    err = fifo_put_byte(&command->alp_response_fifo, file_id); assert(err == SUCCESS);
    err = fifo_put(&command->alp_response_fifo, (uint8_t*)&file_header, sizeof(d7ap_fs_file_header_t)); assert(err == SUCCESS);
  }

  return alp_status;
}

static alp_status_codes_t process_op_write_file_properties(alp_command_t* command) {
  uint8_t file_id;
  error_t err;
  d7ap_fs_file_header_t file_header;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(&command->alp_command_fifo, &file_id, 1); assert(err == SUCCESS);
  err = fifo_pop(&command->alp_command_fifo, (uint8_t*)&file_header, sizeof(d7ap_fs_file_header_t)); assert(err == SUCCESS);
  DPRINT("WRITE FILE PROPERTIES %i", file_id);

  // convert to little endian (native)
  file_header.length = __builtin_bswap32(file_header.length);
  file_header.allocated_length = __builtin_bswap32(file_header.allocated_length);

  int rc = d7ap_fs_write_file_header(file_id, &file_header);
  if(rc != 0)
    return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error

  return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_write_file_data(alp_command_t* command) {
  error_t err;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  file_data_operand.file_offset = alp_parse_file_offset_operand(&command->alp_command_fifo);
  file_data_operand.provided_data_length = alp_parse_length_operand(&command->alp_command_fifo);
  DPRINT("WRITE FILE %i LEN %i", file_data_operand.file_offset.file_id, file_data_operand.provided_data_length);

  if(file_data_operand.provided_data_length > ALP_PAYLOAD_MAX_SIZE)
    return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error

  err = fifo_pop(&command->alp_command_fifo, alp_data, (uint16_t)file_data_operand.provided_data_length);
  int rc = d7ap_fs_write_file(file_data_operand.file_offset.file_id, file_data_operand.file_offset.offset, alp_data, file_data_operand.provided_data_length);
  if(rc != 0)
    return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error

  return ALP_STATUS_OK;
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

  if(comp_length > ALP_PAYLOAD_MAX_SIZE)
    goto error;

  memset(alp_data, 0, comp_length);
  err = fifo_pop(&command->alp_command_fifo, alp_data, (uint16_t)comp_length); assert(err == SUCCESS);
  alp_operand_file_offset_t offset_a = alp_parse_file_offset_operand(&command->alp_command_fifo);

  d7ap_fs_read_file(offset_a.file_id, offset_a.offset, alp_data2, comp_length);

  if(!process_arithm_predicate(alp_data2, alp_data, comp_length, comp_type)) {
    DPRINT("predicate failed, clearing ALP command to stop further processing");
    goto error;
  }

  return ALP_STATUS_OK;

error:
  fifo_clear(&command->alp_command_fifo);
  return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific
}

static void interface_file_changed_callback(uint8_t file_id) {
  interface_file_changed = true;
}

static alp_status_codes_t process_op_indirect_forward(alp_command_t* command, uint8_t* itf_id, alp_interface_config_t* session_config) {
  error_t err;
  bool re_read = false;
  bool found = false;
  alp_control_t ctrl;
  err = fifo_pop(&command->alp_command_fifo, &ctrl.raw, 1); assert(err == SUCCESS);
  uint8_t interface_file_id;
  err = fifo_pop(&command->alp_command_fifo, &interface_file_id, 1);
  if((previous_interface_file_id != interface_file_id) || interface_file_changed) {
    re_read = true;
    interface_file_changed = false;
    if(previous_interface_file_id != interface_file_id) {
      if(fs_file_stat(interface_file_id)!=NULL) {
        fs_unregister_file_modified_callback(previous_interface_file_id);
        fs_register_file_modified_callback(interface_file_changed, &interface_file_changed_callback);
        d7ap_fs_read_file(interface_file_id, 0, itf_id, 1);
        previous_interface_file_id = interface_file_id;
      } else {
        DPRINT("given file is not defined");
        assert(false);
      }
    } else
      *itf_id = session_config_saved.itf_id;
  } else
    *itf_id = session_config_saved.itf_id;
  for(uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
    if(*itf_id == interfaces[i]->itf_id) {
      session_config->itf_id = *itf_id;
      if(re_read) {
        session_config_saved.itf_id = *itf_id;
        d7ap_fs_read_file(interface_file_id, 1, session_config_saved.itf_config, interfaces[i]->itf_cfg_len);
      }
      if(!ctrl.b7) 
        memcpy(session_config->itf_config, session_config_saved.itf_config, interfaces[i]->itf_cfg_len);
#ifdef MODULE_D7AP
      else { //overload bit set
          // TODO
        memcpy(session_config->itf_config, session_config_saved.itf_config, interfaces[i]->itf_cfg_len - 10);
        err = fifo_pop(&command->alp_command_fifo, &session_config->itf_config[interfaces[i]->itf_cfg_len - 10], 2); assert(err == SUCCESS);
        uint8_t id_len = d7ap_addressee_id_length(((alp_interface_config_d7ap_t*)session_config)->d7ap_session_config.addressee.ctrl.id_type);
        err = fifo_pop(&command->alp_command_fifo, &session_config->itf_config[interfaces[i]->itf_cfg_len - 8], id_len); assert(err == SUCCESS);
      }
#endif
      found = true;
      DPRINT("indirect forward %02X", *itf_id);
      break;
    }
  }
  if(!found) {
    DPRINT("interface %02X is not registered", *itf_id);
    assert(false);
  }

  return ALP_STATUS_PARTIALLY_COMPLETED;
}

static alp_status_codes_t process_op_forward(alp_command_t* command, uint8_t* itf_id, alp_interface_config_t* session_config) {
  // TODO move session config to alp_command_t struct
  error_t err;
  bool found = false;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(&command->alp_command_fifo, itf_id, 1); assert(err == SUCCESS);
  for(uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
    if(*itf_id == interfaces[i]->itf_id) {
      session_config->itf_id = *itf_id;
      if(*itf_id == ALP_ITF_ID_D7ASP) {
#ifdef MODULE_D7AP
        uint8_t min_size = interfaces[i]->itf_cfg_len - 8; // substract max size of responder ID
        err = fifo_pop(&command->alp_command_fifo, session_config->itf_config, min_size); assert(err == SUCCESS);
        uint8_t id_len = d7ap_addressee_id_length(((alp_interface_config_d7ap_t*)session_config)->d7ap_session_config.addressee.ctrl.id_type);
        err = fifo_pop(&command->alp_command_fifo, session_config->itf_config + min_size, id_len); assert(err == SUCCESS);
#endif
      } else {
        err = fifo_pop(&command->alp_command_fifo, session_config->itf_config, interfaces[i]->itf_cfg_len); assert(err == SUCCESS);
      }
      found = true;
      DPRINT("FORWARD %02X", *itf_id);
      break;
    }
  }
  if(!found) {
    DPRINT("FORWARD interface %02X not found", *itf_id);
    assert(false);
  }

  return ALP_STATUS_PARTIALLY_COMPLETED;
}

static alp_status_codes_t process_op_response_tag(alp_command_t* command) {
  error_t err;
  alp_control_t ctrl;
  uint8_t tag_id;
  err = fifo_pop(&command->alp_command_fifo, (uint8_t*)&ctrl, 1); assert(err == SUCCESS);
  err = fifo_pop(&command->alp_command_fifo, &tag_id, 1); assert(err == SUCCESS);
  if(tag_id == command->tag_id && ctrl.b7) {
    if(init_args != NULL && init_args->alp_command_completed_cb != NULL) {
      init_args->alp_command_completed_cb(command->tag_id, !ctrl.b6);
    } 
    return ALP_STATUS_OK;
  }
  
  DPRINT("command does not carry same tag_id: %i != %i or not completed: %i", tag_id, command->tag_id, ctrl.b7);
  return ALP_STATUS_UNKNOWN_ERROR;
}

static alp_status_codes_t process_op_status(alp_command_t* command) {
  error_t err;
  alp_control_t ctrl;
  err = fifo_pop(&command->alp_command_fifo, (uint8_t*)&ctrl, 1);
  if (!ctrl.b7 && !ctrl.b6) {
      //action status operation
      uint8_t status_code;
      fifo_pop(&command->alp_command_fifo, &status_code, 1);
      //TO DO implement handling of action status
  } else if (!ctrl.b7 && ctrl.b6) {
      //interface status operation
      alp_interface_status_t status;
      fifo_pop(&command->alp_command_fifo, (uint8_t*)&status.itf_id, 1);
      status.len = (uint8_t)alp_parse_length_operand(&command->alp_command_fifo);
      fifo_pop(&command->alp_command_fifo, status.itf_status, status.len);
      if ((init_args != NULL) && (init_args->alp_command_result_cb != NULL))
          init_args->alp_command_result_cb(&status, NULL, 0);
  } else {
      DPRINT("op_status ext not defined b6=%i, b7=%i", ctrl.b6, ctrl.b7);
      assert(false); // TODO
  }

  return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_request_tag(alp_command_t* command, bool respond_when_completed) {
  error_t err;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  err = fifo_pop(&command->alp_command_fifo, &command->tag_id, 1); assert(err == SUCCESS);
  command->respond_when_completed = respond_when_completed;
  return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_return_file_data(alp_command_t* command) {
  // determine total size of alp
  uint8_t total_len = 2; // ALP control byte + File ID byte
  uint8_t len; // b7 b6 of length field
  uint32_t data_len;
  uint16_t fifo_skip_size;

  // TODO refactor to reuse alp_parse_file_offset_operand() etc but in a way that does not pop() from fifo ..

  // parse file offset length
  error_t e = fifo_peek(&command->alp_command_fifo, (uint8_t*)&len, total_len, 1);
  if(e != SUCCESS) goto incomplete_error;

  uint8_t field_len = len >> 6;
  data_len = (uint32_t)(len & 0x3F) << ( 8 * field_len); // mask field length specificier bits and shift before adding other length bytes
  total_len += 1;
  if(field_len > 0) {
    e = fifo_peek(&command->alp_command_fifo, (uint8_t*)&data_len, total_len, field_len);
    total_len += field_len;
    if(e != SUCCESS) goto incomplete_error;
  }

  DPRINT("Return file data:");
  DPRINT("offset size: %d", field_len + 1);

  // parse file length length
  e = fifo_peek(&command->alp_command_fifo, (uint8_t*)&len, total_len, 1);
  if(e != SUCCESS) goto incomplete_error;

  field_len = len >> 6;
  data_len = (uint32_t)(len & 0x3F) << ( 8 * field_len); // mask field length specificier bits and shift before adding other length bytes
  total_len += 1;
  if(field_len > 0) {
    e = fifo_peek(&command->alp_command_fifo, (uint8_t*)&data_len, total_len, field_len);
    total_len += field_len;
    if(e != SUCCESS) goto incomplete_error;
  }

  DPRINT("length size: %d", field_len + 1);
  DPRINT("data size: %d", data_len);
  total_len += data_len;

  if(fifo_get_size(&command->alp_command_fifo) < total_len) goto incomplete_error;

  if(use_serial_itf && command->origin_itf_id != ALP_ITF_ID_SERIAL) { // make sure we not transmit again over the itf on which we received
    // TODO refactor
    bool found = false;
    for(uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if((interfaces[i] != NULL) && (interfaces[i]->itf_id == ALP_ITF_ID_SERIAL)) {
        DPRINT("serial itf found, sending");
        found = true;
        fifo_t serial_fifo;
        fifo_init(&serial_fifo, alp_data, sizeof(alp_data));
        alp_append_interface_status(&serial_fifo, &current_status);
        fifo_pop(&command->alp_command_fifo, alp_data + fifo_get_size(&serial_fifo), total_len);
        interfaces[i]->send_command(alp_data, total_len + current_status.len + 3, 0, NULL, NULL);
        break;
      }
    }
    if(!found) {
      DPRINT("serial itf not found");
      assert(false);
    }
  } else {
    fifo_pop(&command->alp_command_fifo, alp_data, total_len);
  } 

  if(init_args != NULL && init_args->alp_received_unsolicited_data_cb != NULL)
    init_args->alp_received_unsolicited_data_cb(&current_status, &alp_data[use_serial_itf * current_status.len], total_len);

  return ALP_STATUS_OK;  

incomplete_error:
  // pop processed bytes
  fifo_skip_size = fifo_get_size(&command->alp_command_fifo);
  if(total_len < fifo_skip_size)
    fifo_skip_size = total_len;

  DPRINT("incomplete operand, skipping %i\n", fifo_skip_size);
  fifo_skip(&command->alp_command_fifo, fifo_skip_size);
  return ALP_STATUS_INCOMPLETE_OPERAND;
}

static alp_status_codes_t process_op_create_file(alp_command_t* command) {
  alp_operand_file_header_t operand;
  error_t err;
  err = fifo_skip(&command->alp_command_fifo, 1); assert(err == SUCCESS); // skip the control byte
  operand = alp_parse_file_header_operand(&command->alp_command_fifo);
  DPRINT("CREATE FILE %i", operand.file_id);

  d7ap_fs_init_file(operand.file_id, &operand.file_header, NULL);
}

static void add_tag_response(alp_command_t* command, bool eop, bool error) {
  // fill response with tag response
  DPRINT("add_tag_response %i", command->tag_id);
  uint8_t op_return_tag = ALP_OP_RESPONSE_TAG | (eop << 7);
  op_return_tag |= (error << 6);
  error_t err = fifo_put_byte(&command->alp_response_fifo, op_return_tag); assert(err == SUCCESS);
  err = fifo_put_byte(&command->alp_response_fifo, command->tag_id); assert(err == SUCCESS);
}

void alp_layer_execute_command_over_itf(uint8_t* alp_command, uint8_t alp_command_length, alp_interface_config_t* itf_cfg) {
    bool found = false;
    DPRINT("alp cmd size %i", alp_command_length);
    assert(alp_command_length <= ALP_PAYLOAD_MAX_SIZE);

    alp_command_t* command = alloc_command();
    assert(command != NULL);

    fifo_init_filled(&command->alp_command_fifo, alp_command, alp_command_length, alp_command_length+1);
    error_t err;

    for(uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
      if((interfaces[i] != NULL) && (interfaces[i]->itf_id == itf_cfg->itf_id)) {
        err = interfaces[i]->send_command(alp_command, alp_command_length, alp_get_expected_response_length(command->alp_command_fifo), &command->trans_id, itf_cfg);
        found = true;
        break;
      }
    }
    if(!found) {
      DPRINT("interface %i not found", itf_cfg->itf_id);
      assert(false);
    }

    if(err) {
      DPRINT("transmit returned an error %x", err);
      free_command(command);
    }
}

static bool alp_layer_parse_and_execute_alp_command(alp_command_t* command) {
  DPRINT("parse and exe command");
  alp_interface_config_t session_config;
  uint8_t forward_itf_id = ALP_ITF_ID_HOST;
  bool do_forward = false;
  bool found = false;

  while(fifo_get_size(&command->alp_command_fifo) > 0) {
    if(forward_itf_id != ALP_ITF_ID_HOST) {
      do_forward = true;
      for(uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if(forward_itf_id == interfaces[i]->itf_id) {
          if(interfaces[i]->unique && (interfaces[i]->deinit != current_itf_deinit)) {
            // TODO refactor?
            if(current_itf_deinit != NULL)
              current_itf_deinit();

            interfaces[i]->init(&session_config);
            current_itf_deinit = interfaces[i]->deinit;
          }

          uint8_t forwarded_alp_size = fifo_get_size(&command->alp_command_fifo);
          assert(forwarded_alp_size <= ALP_PAYLOAD_MAX_SIZE);
          uint8_t expected_response_length = alp_get_expected_response_length(command->alp_command_fifo);
          fifo_pop(&command->alp_command_fifo, command->alp_command, forwarded_alp_size);
          error_t error = interfaces[i]->send_command(command->alp_command, forwarded_alp_size, expected_response_length, &command->trans_id, &session_config);
          if(error) {
            DPRINT("transmit returned error %x", error);
            alp_layer_command_completed(command->trans_id, &error, NULL);
          }

          found = true;
          DPRINT("forwarded over interface %02X", forward_itf_id);
          break;
        }
      }
      if(!found) {
        DPRINT("interface %02X not registered, can therefore not be forwarded");
        assert(false);
      }

      return do_forward;
    }

    alp_control_t control;
    assert(fifo_peek(&command->alp_command_fifo, &control.raw, 0, 1) == SUCCESS);
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
    case ALP_OP_STATUS:
        alp_status = process_op_status(command);
        break;
    case ALP_OP_RESPONSE_TAG:
        alp_status = process_op_response_tag(command);
        break;
    case ALP_OP_FORWARD:
        alp_status = process_op_forward(command, &forward_itf_id, &session_config);
        break;
    case ALP_OP_INDIRECT_FORWARD:
        alp_status = process_op_indirect_forward(command, &forward_itf_id, &session_config);
        break;
    case ALP_OP_REQUEST_TAG: ;
        alp_control_tag_request_t* tag_request = (alp_control_tag_request_t*)&control;
        alp_status = process_op_request_tag(command, tag_request->respond_when_completed);
        break;
    case ALP_OP_RETURN_FILE_DATA:
        alp_status = process_op_return_file_data(command);
        break;
    case ALP_OP_CREATE_FILE:
        alp_status = process_op_create_file(command);
        break;
      default:
        assert(false); // TODO return error
        //alp_status = ALP_STATUS_UNKNOWN_OPERATION;
    }
  }

  return do_forward;
}

bool alp_layer_process_command(uint8_t* payload, uint8_t payload_length, alp_itf_id_t origin_itf_id, alp_interface_status_t* itf_status) {
  DPRINT("alp_layer_new_command");
  DPRINT_DATA(payload, payload_length);
  alp_command_t* command = alloc_command();
  assert(command != NULL);
  command->origin_itf_id = origin_itf_id;

  if(itf_status != NULL) // TODO
      current_status = *itf_status;

  memcpy(command->alp_command, payload, payload_length);
  fifo_init_filled(&(command->alp_command_fifo), command->alp_command, payload_length, ALP_PAYLOAD_MAX_SIZE);
  fifo_init(&(command->alp_response_fifo), command->alp_response, ALP_PAYLOAD_MAX_SIZE);

  // TODO
//  if(itf_cfg && (command->itf_id == ALP_ITF_ID_D7ASP))
//    expect_completed = true; //d7aactp

  alp_layer_process_command_timer.next_event = 0;
  alp_layer_process_command_timer.priority = MAX_PRIORITY;
  alp_layer_process_command_timer.arg = command;
  error_t rtc = timer_add_event(&alp_layer_process_command_timer);
  assert(rtc == SUCCESS);

  uint8_t expected_response_length = alp_get_expected_response_length(command->alp_command_fifo);
  DPRINT("This ALP command will initiate a response containing <%d> bytes", expected_response_length);
  return (expected_response_length > 0);
}

static void _async_process_command(void* arg)
{
    alp_command_t* command = (alp_command_t*)arg;
    DPRINT("command allocated <%p>", command);
    assert(command != NULL);

    bool do_forward = alp_layer_parse_and_execute_alp_command(command);

    uint8_t expected_response_length = alp_get_expected_response_length(command->alp_response_fifo);
    if(command->respond_when_completed && !do_forward && (command->origin_itf_id == ALP_ITF_ID_SERIAL)) // TODO will proabably not work anymore, but should be refactored
      add_tag_response(command, true, false);

    uint8_t alp_response_length = (uint8_t)fifo_get_size(&command->alp_response_fifo);
    fifo_pop(&command->alp_response_fifo, command->alp_response, alp_response_length);

    if(alp_response_length) {
      // when the command originates from the app code call callbacks directly, since this is not a 'real' interface
      if(command->origin_itf_id == ALP_ITF_ID_HOST) {
        if(init_args && init_args->alp_command_result_cb)
          init_args->alp_command_result_cb(NULL, command->alp_response, alp_response_length);

        if(init_args && init_args->alp_command_completed_cb)
          init_args->alp_command_completed_cb(command->tag_id, true); // TODO pass possible error

        goto cleanup;
      }

      bool found = false;
      for(uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if((interfaces[i] != NULL) && (interfaces[i]->itf_id == command->origin_itf_id)) {
          DPRINT("interface found, sending len %i, expect %i answer", alp_response_length, expected_response_length);
          found = true;
          error_t err = interfaces[i]->send_command(command->alp_response, alp_response_length, expected_response_length, &command->trans_id, session_config_buffer);
          if(err) {
            free_command(command);
          }

          break;
        }
      }
      if(!found) {
        DPRINT("interface %i not found", command->origin_itf_id);
        assert(false);
      }
    }

cleanup:
    if(!do_forward && !expect_completed) {
      free_command(command);
    }

    return;
}

void alp_layer_command_completed(uint16_t trans_id, error_t* error, alp_interface_status_t* status) {
  DPRINT("command completed with trans id %i and error location %i: value %i", trans_id, error, *error);
  alp_command_t* command = get_command_by_transid(trans_id);
  assert(command != NULL);

  if(use_serial_itf && command->respond_when_completed) {
    if(error != NULL)
      add_tag_response(command, true, *error);
    if(status != NULL)
      fifo_put(&command->alp_response_fifo, status->itf_status, status->len);
    
    uint8_t response_size = fifo_get_size(&command->alp_response_fifo);
    bool found = false;
    fifo_pop(&command->alp_response_fifo, command->alp_response, response_size);
    for(uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
      if((interfaces[i] != NULL) && (interfaces[i]->itf_id == ALP_ITF_ID_SERIAL)) {
        found = true;
        interfaces[i]->send_command(command->alp_response, response_size, 0, NULL, NULL);
        break;
      }
    }
    if(!found) {
      DPRINT("serial itf not found");
      assert(false);
    }
  }

  if(init_args != NULL && init_args->alp_command_completed_cb != NULL && error != NULL)
    init_args->alp_command_completed_cb(command->tag_id, !(*error));

  free_command(command);
}

void alp_layer_received_response(uint16_t trans_id, uint8_t* payload, uint8_t payload_length, alp_interface_status_t* itf_status) {
  DPRINT("received response");
  alp_command_t* command = get_command_by_transid(trans_id);
  assert(command != NULL);
  current_status = *itf_status;


  // received result for known command
  if(use_serial_itf) {
      // TODO refactor

      alp_append_interface_status(&command->alp_response_fifo, itf_status);
      fifo_put(&command->alp_response_fifo, payload, payload_length);

      // tag and send response already with EOP bit cleared
      add_tag_response(command, false, false); // TODO error
      uint8_t response_size = fifo_get_size(&command->alp_response_fifo);
      bool found = false;
      fifo_pop(&command->alp_response_fifo, command->alp_response, response_size);
      for(uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if((interfaces[i] != NULL) && (interfaces[i]->itf_id == ALP_ITF_ID_SERIAL)) {
          found = true;
          interfaces[i]->send_command(command->alp_response, response_size, 0, NULL, NULL);
          break;
        }
      }
      if(!found) {
        DPRINT("serial itf not found");
        assert(false);
      }
  }

  if(init_args != NULL && init_args->alp_command_result_cb != NULL)
    init_args->alp_command_result_cb(itf_status, payload, payload_length);
}



#ifdef MODULE_D7AP
void alp_layer_process_d7aactp(d7ap_session_config_t* session_config, uint8_t* alp_command, uint32_t alp_command_length)
{
    // TODO refactor, might be removed
    alp_command_t* command = alloc_command();
    assert(command != NULL);

    memcpy(command->alp_command, alp_command, alp_command_length);
    fifo_init_filled(&(command->alp_command_fifo), command->alp_command, alp_command_length, ALP_PAYLOAD_MAX_SIZE);
    fifo_init(&(command->alp_response_fifo), command->alp_response, ALP_PAYLOAD_MAX_SIZE);

    alp_layer_parse_and_execute_alp_command(command);

    uint8_t expected_response_length = alp_get_expected_response_length(command->alp_response_fifo);
    error_t error = d7ap_send(alp_client_id, session_config, command->alp_response,
        fifo_get_size(&(command->alp_response_fifo)), expected_response_length, &command->trans_id);

    if (error)
    {
        DPRINT("d7ap_send returned an error %x", error);
        free_command(command);
    }
}
#endif // MODULE_D7AP

#ifdef MODULE_LORAWAN
alp_interface_t interface_lorawan_otaa;
alp_interface_t interface_lorawan_abp;
uint16_t lorawan_trans_id;
bool otaa_just_inited = false;
bool abp_just_inited = false;

void lorawan_rx(lorawan_AppData_t *AppData)
{
  alp_layer_process_command(AppData->Buff, AppData->BuffSize, current_lorawan_interface_type, NULL);
}

void add_interface_status_lorawan(uint8_t* payload, uint8_t attempts, lorawan_stack_status_t status) {
  payload[0] = ALP_OP_STATUS + (1 << 6);
  payload[1] = current_lorawan_interface_type;
  payload[2] = 4; //length
  payload[3] = attempts;
  payload[4] = status;
  uint16_t wait_time = __builtin_bswap16(lorawan_get_duty_cycle_delay());
  memcpy(&payload[5], (uint8_t*)&wait_time, 2);
}

void lorawan_command_completed(lorawan_stack_status_t status, uint8_t attempts) {
  error_t status_buffer = (error_t)status;
  alp_interface_status_t result = (alp_interface_status_t) {
    .itf_id = current_lorawan_interface_type,
    .len = 7,
  };
  add_interface_status_lorawan(result.itf_status, attempts, status);

  alp_layer_command_completed(lorawan_trans_id, &status_buffer, &result);
}

static void lorawan_status_callback(lorawan_stack_status_t status, uint8_t attempts)
{
  alp_command_t* command = alloc_command();
  command->respond_when_completed=true;

  alp_interface_status_t result = (alp_interface_status_t) {
    .itf_id = current_lorawan_interface_type,
    .len = 7
  };
  add_interface_status_lorawan(result.itf_status, attempts, status);
  
  alp_layer_command_completed(command->trans_id, NULL, &result);
}

static error_t lorawan_send_otaa(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* itf_cfg)
{
  alp_interface_config_lorawan_otaa_t* lorawan_itf_cfg = (alp_interface_config_lorawan_otaa_t*)itf_cfg;
  if(!otaa_just_inited && (lorawan_otaa_is_joined(&lorawan_itf_cfg->lorawan_session_config_otaa))) {
    DPRINT("sending otaa payload");
    current_lorawan_interface_type = ALP_ITF_ID_LORAWAN_OTAA;
    lorawan_stack_status_t status = lorawan_stack_send(payload, payload_length, lorawan_itf_cfg->lorawan_session_config_otaa.application_port, lorawan_itf_cfg->lorawan_session_config_otaa.request_ack);
    lorawan_error_handler(trans_id, status);
  } else { //OTAA not joined yet or still initing
    DPRINT("OTAA not joined yet");
    otaa_just_inited = false;
    alp_command_t* command = get_command_by_transid(*trans_id);
    fifo_put(&command->alp_response_fifo, payload, payload_length);
    lorawan_error_handler(trans_id, LORAWAN_STACK_ERROR_NOT_JOINED);
  }
  return SUCCESS;
}

static error_t lorawan_send_abp(uint8_t* payload, uint8_t payload_length, uint8_t expected_response_length, uint16_t* trans_id, alp_interface_config_t* itf_cfg) {
  alp_interface_config_lorawan_abp_t* lorawan_itf_cfg = (alp_interface_config_lorawan_abp_t*)itf_cfg;
  if(!abp_just_inited) {
    lorawan_itf_cfg->lorawan_session_config_abp.devAddr = __builtin_bswap32(lorawan_itf_cfg->lorawan_session_config_abp.devAddr);
    lorawan_itf_cfg->lorawan_session_config_abp.network_id = __builtin_bswap32(lorawan_itf_cfg->lorawan_session_config_abp.network_id);
    lorawan_abp_is_joined(&lorawan_itf_cfg->lorawan_session_config_abp);
  } else
    abp_just_inited = false;
  DPRINT("sending abp payload");
  current_lorawan_interface_type = ALP_ITF_ID_LORAWAN_ABP;
  lorawan_stack_status_t status = lorawan_stack_send(payload, payload_length, lorawan_itf_cfg->lorawan_session_config_abp.application_port, lorawan_itf_cfg->lorawan_session_config_abp.request_ack);
  lorawan_error_handler(trans_id, status);
  return SUCCESS;
}

static void lorawan_error_handler(uint16_t* trans_id, lorawan_stack_status_t status) {
  if(status != LORAWAN_STACK_ERROR_OK) {
    log_print_string("!!!LORAWAN ERROR: %d\n", status);
    error_t status_buffer = (error_t)status;
    alp_interface_status_t result = (alp_interface_status_t) {
      .itf_id = current_lorawan_interface_type,
      .len = 7
    };
    add_interface_status_lorawan(result.itf_status, 1, status);
  
    alp_layer_command_completed(*trans_id, &status_buffer, &result);
  } else 
    lorawan_trans_id = *trans_id;
}

static void lorawan_init_otaa(alp_interface_config_t* itf_cfg) {
  lorawan_stack_init_otaa(&((alp_interface_config_lorawan_otaa_t*)itf_cfg)->lorawan_session_config_otaa);
  otaa_just_inited = true;
}

static void lorawan_init_abp(alp_interface_config_t* itf_cfg) {
  alp_interface_config_lorawan_abp_t* lorawan_itf_cfg = (alp_interface_config_lorawan_abp_t*)itf_cfg;
  lorawan_itf_cfg->lorawan_session_config_abp.devAddr = __builtin_bswap32(lorawan_itf_cfg->lorawan_session_config_abp.devAddr);
  lorawan_itf_cfg->lorawan_session_config_abp.network_id = __builtin_bswap32(lorawan_itf_cfg->lorawan_session_config_abp.network_id);
  lorawan_stack_init_abp(&lorawan_itf_cfg->lorawan_session_config_abp);
  abp_just_inited = true;
}

static void alp_layer_lorawan_init() {
  lorawan_register_cbs(lorawan_rx, lorawan_command_completed, lorawan_status_callback);

  interface_lorawan_otaa = (alp_interface_t) {
    .itf_id = 0x03,
    .itf_cfg_len = sizeof(lorawan_session_config_otaa_t),
    .itf_status_len = 7,
    .init = lorawan_init_otaa,
    .deinit = lorawan_stack_deinit,
    .send_command = lorawan_send_otaa,
    .unique = true
  };
  alp_layer_register_interface(&interface_lorawan_otaa);

  interface_lorawan_abp = (alp_interface_t) {
    .itf_id = ALP_ITF_ID_LORAWAN_ABP,
    .itf_cfg_len = sizeof(lorawan_session_config_abp_t),
    .itf_status_len = 7,
    .init = lorawan_init_abp,
    .deinit = lorawan_stack_deinit,
    .send_command = lorawan_send_abp,
    .unique = true
  };
  alp_layer_register_interface(&interface_lorawan_abp);

}
#endif
