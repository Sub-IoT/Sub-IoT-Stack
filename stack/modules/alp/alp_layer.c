/*! \file alp_layer.c
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
 *  \author glenn.ergeerts@aloxy.io
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

#ifdef MODULE_LORAWAN
#include "lorawan_interface.h"
#endif

#ifdef MODULE_D7AP
#include "d7ap_interface.h"
#endif

#include "d7ap_fs.h"

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

static interface_deinit current_itf_deinit = NULL;

bool use_serial_itf;

static alp_command_t NGDEF(_commands)[MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT];
#define commands NG(_commands)

static alp_interface_status_t NGDEF( current_status);
#define current_status NG(current_status)

static alp_init_args_t* NGDEF(_init_args);
#define init_args NG(_init_args)

static uint8_t previous_interface_file_id = 0;
static bool interface_file_changed = true;
static alp_interface_config_t session_config_saved;
static uint8_t alp_data[ALP_PAYLOAD_MAX_SIZE]; // temp buffer statically allocated to prevent runtime stackoverflows
static uint8_t alp_data2[ALP_PAYLOAD_MAX_SIZE]; // temp buffer statically allocated to prevent runtime stackoverflows

extern alp_interface_t* interfaces[MODULE_ALP_INTERFACE_SIZE];
static alp_interface_config_t* session_config_buffer;

static void process_async(void* arg);

static uint8_t next_tag_id = 0;

static fifo_t command_fifo;
static alp_command_t* command_fifo_buffer[MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT];

static void free_command(alp_command_t* command) {
  DPRINT("!!! Free cmd %02x %p", command->trans_id, command);
  memset(command, 0, sizeof (alp_command_t));
  command->is_active = false;
  fifo_init(&command->alp_command_fifo, command->alp_command, ALP_PAYLOAD_MAX_SIZE);
  // other fields are initialized on usage
}

static void init_commands()
{
  for(uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
    free_command(&commands[i]);
  }
}

alp_command_t* alp_layer_command_alloc(bool with_tag_request, bool always_respond)
{
    for (uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
        if (commands[i].is_active == false) {
            commands[i].is_active = true;
            DPRINT("alloc cmd %p in slot %i (%p)", &commands[i], i, &commands[i]);
            if (with_tag_request) {
                next_tag_id++;
                alp_append_tag_request_action(&commands[i], next_tag_id, always_respond);
                commands[i].tag_id = next_tag_id;
            }

            return &(commands[i]);
        }
    }

    DPRINT("Could not alloc command, all %i reserved slots active", MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT);
    return NULL;
}

void alp_layer_command_free(alp_command_t* command)
{
    for (uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
        if (&commands[i] == command) {
            free_command(command);
        }
    }
    
    DPRINT("Could not free command");
}

static alp_command_t* get_request_command(uint8_t tag_id, uint8_t itf_id)
{
    for (uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
        if (commands[i].forward_itf_id == itf_id && commands[i].tag_id == tag_id && commands[i].is_active && !commands[i].is_response) {
            DPRINT("found matching req command with tag %i for fwd itf %i in slot %i\n", tag_id, itf_id, i);
            return &(commands[i]);
        }
    }

    DPRINT("No matching req command found with tag %i fwd over itf %i\n", tag_id, itf_id);
    return NULL;
}

alp_command_t* alp_layer_get_command_by_transid(uint16_t trans_id, uint8_t itf_id) {
  for(uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
    if(commands[i].forward_itf_id == itf_id && commands[i].trans_id == trans_id && commands[i].is_active) {
        DPRINT("command trans Id %i in slot %i\n", trans_id, i);
        return &(commands[i]);
    }
  }

  DPRINT("No active command found with transaction Id = %i transmitted over itf %i\n", trans_id, itf_id);
  return NULL;
}

void alp_layer_init(alp_init_args_t* alp_init_args, bool use_serial_interface)
{
  init_args = alp_init_args;
  use_serial_itf = use_serial_interface;
  fifo_init(&command_fifo, (uint8_t*)command_fifo_buffer, MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT);
  init_commands();

  if (use_serial_itf)
    serial_interface_register();

#ifdef MODULE_D7AP
  d7ap_interface_register();
#endif

#ifdef MODULE_LORAWAN
  lorawan_interface_register();
#endif
  
  sched_register_task(&process_async);
}

void alp_layer_register_interface(alp_interface_t* interface) {
  alp_register_interface(interface);
}

static alp_status_codes_t process_op_read_file_data(alp_action_t* action, alp_command_t* resp_command)
{
    alp_operand_file_data_request_t operand = action->file_data_request_operand;
    DPRINT("READ FILE %i LEN %i", operand.file_offset.file_id, operand.requested_data_length);

    if (operand.requested_data_length <= 0 || operand.requested_data_length > ALP_PAYLOAD_MAX_SIZE)
        return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error + move to fs_read_file?

    int rc = d7ap_fs_read_file(operand.file_offset.file_id, operand.file_offset.offset, alp_data, operand.requested_data_length);
    if (rc == -ENOENT) {
        // give the application layer the chance to fullfill this request ...
        if (init_args != NULL && init_args->alp_unhandled_read_action_cb != NULL)
            rc = init_args->alp_unhandled_read_action_cb(&current_status, operand, alp_data);
    }

    if (rc == 0) {
        // fill response
        alp_append_return_file_data_action(resp_command, operand.file_offset.file_id, operand.file_offset.offset,
            operand.requested_data_length, alp_data);
    }

    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_read_file_properties(alp_action_t* action, alp_command_t* resp_command)
{
    error_t err;
    
    DPRINT("READ FILE PROPERTIES %i", action->file_id_operand.file_id);

    d7ap_fs_file_header_t file_header;
    err = d7ap_fs_read_file_header(action->file_id_operand.file_id, &file_header);
    if (err != SUCCESS) {
        DPRINT("!!! d7ap_fs_read_file_header returned error %i", err);
        assert(false); // TODO error handlink
    }
    
    // convert to big endian
    file_header.length = __builtin_bswap32(file_header.length);
    file_header.allocated_length = __builtin_bswap32(file_header.allocated_length);

    // fill response
    err = fifo_put_byte(&resp_command->alp_command_fifo, ALP_OP_RETURN_FILE_PROPERTIES);
    assert(err == SUCCESS);
    err = fifo_put_byte(&resp_command->alp_command_fifo, action->file_id_operand.file_id);
    assert(err == SUCCESS);
    err = fifo_put(&resp_command->alp_command_fifo, (uint8_t*)&file_header, sizeof(d7ap_fs_file_header_t));
    assert(err == SUCCESS);
    
    return SUCCESS;
}

static alp_status_codes_t process_op_write_file_properties(alp_action_t* action)
{
    DPRINT("WRITE FILE PROPERTIES %i", action->file_header_operand.file_id);
        
    int rc = d7ap_fs_write_file_header(action->file_header_operand.file_id, &action->file_header_operand.file_header);
    if(rc != 0)
        return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error

    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_write_file_data(alp_action_t* action) {
    DPRINT("WRITE FILE %i LEN %i", action->file_data_operand.file_offset.file_id, action->file_data_operand.provided_data_length);
    if (action->file_data_operand.provided_data_length > ALP_PAYLOAD_MAX_SIZE)
        return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error
    
    int rc = d7ap_fs_write_file(action->file_data_operand.file_offset.file_id, action->file_data_operand.file_offset.offset,
        action->file_data_operand.data, action->file_data_operand.provided_data_length);
    if (rc != 0) {
        assert(false);
        return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific error
    }

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
  return false;
}

static alp_status_codes_t process_op_break_query(alp_action_t* action)
{
    
    DPRINT("BREAK QUERY");
    assert(action->query_operand.code.type
        == QUERY_CODE_TYPE_ARITHM_COMP_WITH_VALUE_IN_QUERY); // TODO only arithm comp with value type is implemented for
    // now
    
    assert(!action->query_operand.code.mask); // TODO mask value not implemented for now
    
    // parse arithm query params
    bool use_signed_comparison = true;
    if((action->query_operand.code.param & 0x08) == 0)
        use_signed_comparison = false;
    
    alp_query_arithmetic_comparison_type_t comp_type = action->query_operand.code.param & 0x07;

    // TODO assuming no compare mask for now + assume compare value present + only 1 file offset operand
    fifo_t temp_fifo;
    fifo_init_filled(&temp_fifo, action->query_operand.compare_body, ALP_QUERY_COMPARE_BODY_MAX_SIZE, ALP_QUERY_COMPARE_BODY_MAX_SIZE);
    
    memset(alp_data, 0, action->query_operand.compare_operand_length);
    error_t err = fifo_pop(&temp_fifo, alp_data, action->query_operand.compare_operand_length); assert(err == SUCCESS);
    alp_operand_file_offset_t offset_a = alp_parse_file_offset_operand(&temp_fifo);
    
    d7ap_fs_read_file(offset_a.file_id, offset_a.offset, alp_data2, action->query_operand.compare_operand_length);
    
    if(!process_arithm_predicate(alp_data2, alp_data, action->query_operand.compare_operand_length, comp_type)) {
        DPRINT("predicate failed, clearing ALP command to stop further processing");
        goto error;
    }
    
    return ALP_STATUS_OK;

error:
    //fifo_clear(&command->alp_command_fifo);
    return ALP_STATUS_UNKNOWN_ERROR; // TODO more specific  
}

static void interface_file_changed_callback(uint8_t file_id)
{
    (void)file_id; // suppress unused warning
    interface_file_changed = true;
}

static alp_status_codes_t process_op_indirect_forward(
    alp_action_t* action, uint8_t* itf_id, alp_interface_config_t* session_config)
{
    DPRINT("indirect fwd");
    bool re_read = false;
    bool found = false;
    alp_control_t ctrl;
    if ((previous_interface_file_id != action->indirect_interface_operand.interface_file_id)
        || interface_file_changed) {
        re_read = true;
        interface_file_changed = false;
        if (previous_interface_file_id != action->indirect_interface_operand.interface_file_id) {
            if (fs_file_stat(action->indirect_interface_operand.interface_file_id) != NULL) {
                fs_unregister_file_modified_callback(previous_interface_file_id);
                fs_register_file_modified_callback(interface_file_changed, &interface_file_changed_callback);
                d7ap_fs_read_file(action->indirect_interface_operand.interface_file_id, 0, itf_id, 1);
                previous_interface_file_id = action->indirect_interface_operand.interface_file_id;
            } else {
                DPRINT("itf file %i is not defined", action->indirect_interface_operand.interface_file_id);
                assert(false);
            }
        } else {
            *itf_id = session_config_saved.itf_id;
        }
    } else {
        *itf_id = session_config_saved.itf_id;
    }

    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if (*itf_id == interfaces[i]->itf_id) {
            session_config->itf_id = *itf_id;
            if (re_read) {
                session_config_saved.itf_id = *itf_id;
                d7ap_fs_read_file(action->indirect_interface_operand.interface_file_id, 1,
                    session_config_saved.itf_config, interfaces[i]->itf_cfg_len);
            }
            if (!ctrl.b7)
                memcpy(session_config->itf_config, session_config_saved.itf_config, interfaces[i]->itf_cfg_len);
#ifdef MODULE_D7AP
            else { // overload bit set
                // TODO
                memcpy(session_config->itf_config, session_config_saved.itf_config, interfaces[i]->itf_cfg_len - 10);
                memcpy(&session_config->itf_config[interfaces[i]->itf_cfg_len - 10],
                    action->indirect_interface_operand.overload_data, 2);
                uint8_t id_len = d7ap_addressee_id_length(
                    ((alp_interface_config_d7ap_t*)session_config)->d7ap_session_config.addressee.ctrl.id_type);
                memcpy(&session_config->itf_config[interfaces[i]->itf_cfg_len - 8],
                    action->indirect_interface_operand.overload_data, id_len);
            }
#endif
            found = true;
            DPRINT("indirect forward %02X", *itf_id);
            break;
        }
    }
    if (!found) {
        DPRINT("interface %02X is not registered", *itf_id);
        assert(false);
    }

    return ALP_STATUS_PARTIALLY_COMPLETED;
}

static alp_status_codes_t process_op_forward(alp_action_t* action, uint8_t* itf_id, alp_interface_config_t* session_config)
{
    // TODO move session config to alp_command_t struct
    memcpy(session_config, &action->interface_config, sizeof(alp_interface_config_t));
    *itf_id = action->interface_config.itf_id;
    DPRINT("FORWARD %02X", session_config->itf_id);
    return ALP_STATUS_PARTIALLY_COMPLETED;
}

static alp_status_codes_t process_op_response_tag(alp_command_t* command, uint8_t* tag_id, bool* is_response_completed, bool* is_response_error)
{
    alp_control_t ctrl;
    error_t err = fifo_pop(&command->alp_command_fifo, &ctrl.raw, 1); assert(err == SUCCESS);
    err = fifo_pop(&command->alp_command_fifo, tag_id, 1); assert(err == SUCCESS);

    *is_response_completed = ctrl.b7;
    *is_response_error = ctrl.b6;
    DPRINT("tag response %i EOP %i, ERR %i\n", *tag_id, ctrl.b7, ctrl.b6);
    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_status(alp_action_t* action, alp_command_t* command)
{
    if (!action->ctrl.b7 && !action->ctrl.b6) {
        //action status operation
        DPRINT("act status");
        //TO DO implement handling of action status
    } else if (!action->ctrl.b7 && action->ctrl.b6) {
        //interface status operation
        memcpy(&command->origin_itf_status, &action->interface_status, sizeof(alp_interface_status_t));
        command->origin_itf_id = command->origin_itf_status.itf_id; 
        DPRINT("itf status (%i)", command->origin_itf_status.itf_id);
    } else {
        DPRINT("op_status ext not defined b6=%i, b7=%i", action->ctrl.b6, action->ctrl.b7);
        assert(false); // TODO
    }

    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_request_tag(alp_action_t* action, uint8_t* tag_id, bool* respond_when_completed)
{
    *tag_id = action->tag_id_operand.tag_id;
    *respond_when_completed = action->ctrl.b7;
    DPRINT("tag req %i, EOP %i", *tag_id, *respond_when_completed);
    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_return_file_data(alp_action_t* action, alp_command_t** unsollicited_response_command)
{
    if (*unsollicited_response_command == NULL) {
        *unsollicited_response_command = alp_layer_command_alloc(false, false);
    }
    
    DPRINT("Return file data (%i):", action->file_data_operand.file_offset.file_id);
    DPRINT("offset size: %d", action->file_data_operand.file_offset.offset);
    DPRINT("data size: %d", action->file_data_operand.provided_data_length);
    // fill unsollicited_response_command
    alp_append_return_file_data_action(*unsollicited_response_command, action->file_data_operand.file_offset.file_id,
        action->file_data_operand.file_offset.offset, action->file_data_operand.provided_data_length, action->file_data_operand.data);
    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_create_file(alp_action_t* action) {
    DPRINT("CREATE FILE %i", action->file_header_operand.file_id);
    d7ap_fs_init_file(action->file_header_operand.file_id, &action->file_header_operand.file_header, NULL);
    return ALP_STATUS_OK;
}

void alp_layer_execute_command_over_itf(uint8_t* alp_command, uint8_t alp_command_length, alp_interface_config_t* itf_cfg) {
    bool found = false;
    DPRINT("alp cmd size %i", alp_command_length);
    assert(alp_command_length <= ALP_PAYLOAD_MAX_SIZE);

    alp_command_t* command = alp_layer_command_alloc(true, true);
    assert(command != NULL);

    fifo_init_filled(&command->alp_command_fifo, alp_command, alp_command_length, alp_command_length+1);
    error_t err = SUCCESS;

    for(uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
      if((interfaces[i] != NULL) && (interfaces[i]->itf_id == itf_cfg->itf_id)) {
        err = interfaces[i]->send_command(alp_command, alp_command_length, alp_get_expected_response_length(command), &command->trans_id, itf_cfg);
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

static void forward_command(alp_command_t* command, alp_interface_config_t* itf_config)
{
    bool found = false;
    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if (command->forward_itf_id == interfaces[i]->itf_id) {
            if (interfaces[i]->unique && (interfaces[i]->deinit != current_itf_deinit)) {
                // TODO refactor? always init the interface?
                if (current_itf_deinit != NULL)
                    current_itf_deinit();

                interfaces[i]->init(itf_config);
                current_itf_deinit = interfaces[i]->deinit;
            }

            uint8_t forwarded_alp_size = fifo_get_size(&command->alp_command_fifo);
            assert(forwarded_alp_size <= ALP_PAYLOAD_MAX_SIZE);
            fifo_pop(&command->alp_command_fifo, command->alp_command, forwarded_alp_size);
            DPRINT("Forwarding command:");
            DPRINT_DATA(command->alp_command, forwarded_alp_size);
            uint8_t expected_response_length = alp_get_expected_response_length(command);
            command->forward_itf_id = itf_config->itf_id;
            error_t error = interfaces[i]->send_command(command->alp_command, forwarded_alp_size, expected_response_length, &command->trans_id, itf_config);
            if (command->trans_id == 0)
                command->trans_id = command->tag_id; // interface does not provide transaction tracking, using tag_id

            if (error) {
                DPRINT("transmit returned error %x", error);
                alp_layer_forwarded_command_completed(command->trans_id, &error, NULL); // TODO return error to caller instead?
            }

            found = true;
            DPRINT("forwarded over interface %02X", command->forward_itf_id);
            break;
        }
    }
    if (!found) {
        DPRINT("interface %02X not registered, can therefore not be forwarded");
        assert(false);
    }
}

static void transmit_response_to_app(alp_command_t* resp)
{
    if (init_args && init_args->alp_command_result_cb) {
        uint8_t alp_len = fifo_get_size(&resp->alp_command_fifo);
        fifo_pop(&resp->alp_command_fifo, alp_data, alp_len);
        init_args->alp_command_result_cb(NULL, resp);
    }
}

static void transmit_response_to_serial(alp_command_t* resp, alp_interface_status_t* origin_itf_status)
{
    bool found = false;
    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if ((interfaces[i] != NULL) && (interfaces[i]->itf_id == ALP_ITF_ID_SERIAL)) {
            DPRINT("serial itf found, sending");
            found = true;
            alp_append_interface_status(resp, origin_itf_status);
            uint32_t len = fifo_get_size(&resp->alp_command_fifo);
            fifo_pop(&resp->alp_command_fifo, alp_data, len);
            interfaces[i]->send_command(alp_data, len, 0, NULL, NULL);
            break;
        }
    }
    if (!found) {
        DPRINT("serial itf not found");
        assert(false);
    }
}

static void transmit_response(alp_command_t* req, alp_command_t* resp)
{
    DPRINT("async response to cmd tag %i, ori itf %i completed %i", req->tag_id, req->origin_itf_id, resp->is_response_completed);
    // when the command originates from the app code call callbacks directly, since this is not a 'real' interface
    if (req->origin_itf_id == ALP_ITF_ID_HOST) {
        transmit_response_to_app(resp);
        return;
    }

    bool found = false;
    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_SIZE; i++) {
        if ((interfaces[i] != NULL) && (interfaces[i]->itf_id == req->origin_itf_id)) {
            uint8_t alp_response_length = (uint8_t)fifo_get_size(&resp->alp_command_fifo);
            uint8_t expected_response_length = 0; // TODO alp_get_expected_response_length(&resp->alp_command_fifo);
            //fifo_pop(&command->alp_response_fifo, command->alp_response, alp_response_length);
            DPRINT("interface found, sending len %i, expect %i answer", alp_response_length, expected_response_length);
            found = true;
            error_t err = interfaces[i]->send_command(resp->alp_command, alp_response_length, expected_response_length, &resp->trans_id, session_config_buffer);
            if (err == SUCCESS) {
                free_command(resp); // TODO req
            } else {
                assert(false); // TODO
            }

            break;
        }
    }
    if (!found) {
        DPRINT("interface %i not found", req->origin_itf_id);
        assert(false);
    }
}

void process_async(void* arg)
{
    (void)arg; // suppress unused warning
    alp_command_t* command = NULL;
    if (fifo_pop(&command_fifo, (void*)&command, sizeof(alp_command_t*)) != SUCCESS) {
        return;
    }

    if (fifo_get_size(&command_fifo) > 0) {
        sched_post_task_prio(&process_async, MIN_PRIORITY, 0);
    }
    
    DPRINT("process command");
    DPRINT_DATA(command->alp_command, fifo_get_size(&command->alp_command_fifo));
    alp_interface_config_t forward_interface_config;
    alp_command_t* resp_command = alp_layer_command_alloc(false, false);
    alp_command_t* unsollicited_resp_command = NULL;
    alp_action_t action;

    while (fifo_get_size(&command->alp_command_fifo) > 0) {      
        alp_parse_action(command, &action);
//        alp_control_t control;
//        assert(fifo_peek(&command->alp_command_fifo, &control.raw, 0, 1) == SUCCESS); // TODO pop control byte
        alp_status_codes_t alp_status;
        switch (action.ctrl.operation) {
        case ALP_OP_READ_FILE_DATA:
            alp_status = process_op_read_file_data(&action, resp_command);
            break;
        case ALP_OP_READ_FILE_PROPERTIES:
            alp_status = process_op_read_file_properties(&action, resp_command);
            break;
        case ALP_OP_WRITE_FILE_DATA:
            alp_status = process_op_write_file_data(&action);
            break;
        case ALP_OP_WRITE_FILE_PROPERTIES:
            alp_status = process_op_write_file_properties(&action);
            break;
        case ALP_OP_BREAK_QUERY:
            alp_status = process_op_break_query(&action);
            break;
        case ALP_OP_STATUS:
            alp_status = process_op_status(&action, command);
            break;
        case ALP_OP_RESPONSE_TAG:;
            uint8_t resp_tag_id;
            alp_status = process_op_response_tag(command, &resp_tag_id, &command->is_response_completed, &command->is_response_error);
            command->is_response = true;
            assert(command->tag_id == 0 || command->tag_id == resp_tag_id);
            command->tag_id = resp_tag_id;
            break;
        case ALP_OP_FORWARD:
            alp_status = process_op_forward(&action, &command->forward_itf_id, &forward_interface_config);
            break;
        case ALP_OP_INDIRECT_FORWARD:
            alp_status = process_op_indirect_forward(&action, &command->forward_itf_id, &forward_interface_config);
            break;
        case ALP_OP_REQUEST_TAG:;
            alp_status = process_op_request_tag(&action, &command->tag_id, &command->respond_when_completed);
            command->is_tag_requested = true;
            break;
        case ALP_OP_RETURN_FILE_DATA:
            alp_status = process_op_return_file_data(&action, &unsollicited_resp_command);
            break;
        case ALP_OP_CREATE_FILE:
            alp_status = process_op_create_file(&action);
            break;
        default:
            DPRINT("op %i not implemented\n", action.ctrl.operation);
            alp_status = ALP_STATUS_UNKNOWN_OPERATION;
            assert(false); // TODO return error
        }

        if (alp_status != ALP_STATUS_OK && alp_status != ALP_STATUS_PARTIALLY_COMPLETED) {
            DPRINT("ALP status NOK (%i), skipping", alp_status);
            break; // TODO cleanup?
        }

        if (command->forward_itf_id != ALP_ITF_ID_HOST) {
            if (!command->is_response) {
                //fifo_skip(&command->alp_command_fifo, parse_command_index);
                forward_command(command, &forward_interface_config);
                free_command(resp_command); // command itself will be free-ed when interface responds with this command with correct tag
                return;
            } else {
                // response received over interface we forwarded to, stop parsing to return rest of command
            }
        }
    }
    
    if (unsollicited_resp_command != NULL) {
        if (use_serial_itf) {
            DPRINT("Unsollicited response, sending to serial");
            transmit_response_to_serial(unsollicited_resp_command, &command->origin_itf_status);
        } else {
            DPRINT("Unsollicited response, sending to app");
            transmit_response_to_app(unsollicited_resp_command);
        }
        
        free_command(unsollicited_resp_command);
    }
    
    DPRINT("command is_reponse %i , tag_id %i, completed %i, error %i, ori itf id %i, resp when completed %i\n",
        command->is_response, command->tag_id, command->is_response_completed, command->is_response_error, command->origin_itf_id, command->respond_when_completed);
    if (command->is_response) {
        // when the command is an async response to a preceding request we first find the original request and send the response to the origin itf
        // find original request
        alp_command_t* request_command = get_request_command(command->tag_id, command->origin_itf_id);
        assert(request_command != NULL); // TODO
        DPRINT("async response to cmd tag %i, ori itf %i respond_when_compl %i", request_command->tag_id, request_command->origin_itf_id, request_command->respond_when_completed);
        uint8_t cmd_size = fifo_get_size(&command->alp_command_fifo);
        if (cmd_size > 0 || request_command->respond_when_completed) {
            if (request_command->respond_when_completed && request_command->origin_itf_id != ALP_ITF_ID_HOST) {
                // when the request originates from another interface it will already contain a tagresponse, since we always request a tag on forwarding
                alp_append_tag_response_action(resp_command, request_command->tag_id, command->is_response_completed, command->is_response_error);
            }

            resp_command->is_response = true;
            resp_command->is_response_completed = command->is_response_completed;
            fifo_pop(&command->alp_command_fifo, alp_data, cmd_size);
            fifo_put(&resp_command->alp_command_fifo, alp_data, cmd_size);
            transmit_response(request_command, resp_command);
        }

        if (command->is_response_completed) {
            if (request_command->origin_itf_id == ALP_ITF_ID_HOST && (init_args != NULL) && (init_args->alp_command_completed_cb != NULL)) {
                init_args->alp_command_completed_cb(request_command->tag_id, !command->is_response_error);
            }

            DPRINT("free orig req cmd");
            free_command(request_command);
        }
    } else {
        uint8_t resp_cmd_size = fifo_get_size(&resp_command->alp_command_fifo);
        DPRINT("resp_cmd size %i", resp_cmd_size);
        DPRINT_DATA(resp_command->alp_command, resp_cmd_size);
        if (resp_cmd_size > 0 || command->respond_when_completed) {
            //if ((resp_cmd_size == 0 && command->respond_when_completed) || resp_cmd_size > 0) {
            if (command->is_tag_requested && command->origin_itf_id != ALP_ITF_ID_HOST) {
                // make sure to respond when requested, even if there is no response payload
                // when the request originates from another interface it will already contain a tagresponse, since we always request a tag on forwarding
                // TODO set err flag
                alp_append_tag_response_action(resp_command, command->tag_id, true, false);
            }

            transmit_response(command, resp_command);
        }
    }

    free_command(resp_command);
    free_command(command);
    return;
}

bool alp_layer_process(alp_command_t* command)
{
//    alp_command_t* command = alp_layer_command_alloc(false, false);
//    if (command == NULL) {
//        assert(false); // TODO error handling
//    }
    
    //fifo_put(&command->alp_command_fifo, payload, len);
 
    DPRINT_DATA(command->alp_command, fifo_get_size(&command->alp_command_fifo));
    uint8_t expected_response_length = alp_get_expected_response_length(command);
    DPRINT("This ALP command will initiate a response containing <%d> bytes\n", expected_response_length);
    if (expected_response_length == 0) {
        command->respond_when_completed = false;
    }
    
    // add to fifo for later processing
    error_t rtc = fifo_put(&command_fifo, (uint8_t*)&command, sizeof(alp_command_t*));
    rtc = sched_post_task_prio(&process_async, MIN_PRIORITY, NULL);
    assert(rtc == SUCCESS || rtc == EALREADY);
    
    return (expected_response_length > 0);
}

void alp_layer_forwarded_command_completed(uint16_t trans_id, error_t* error, alp_interface_status_t* status)
{
    // TODO
    DPRINT("command completed with trans id %i and error location %i: value %i", trans_id, error, *error);
    assert(status != NULL);
    alp_command_t* command = alp_layer_get_command_by_transid(trans_id, status->itf_id);
    assert(command != NULL);
    DPRINT("resp for tag %i\n", command->tag_id);
    alp_command_t* resp = alp_layer_command_alloc(false, false);
    resp->forward_itf_id = status->itf_id;
    resp->origin_itf_id = status->itf_id;
    resp->is_response_completed = true;
    resp->is_response = true;
    resp->is_response_error = (*error != SUCCESS);
    resp->tag_id = command->tag_id;
    alp_append_interface_status(resp, status);

    // we do not add a tag response, since we know the original command using the trans_id
    // alp_layer_process(resp->alp_command, fifo_get_size(&resp->alp_command_fifo));
    alp_layer_process(resp);

    //free_command(command);
}

void alp_layer_received_response(uint16_t trans_id, uint8_t* payload, uint8_t payload_length, alp_interface_status_t* itf_status)
{
    DPRINT("received response");
    alp_command_t* command = alp_layer_get_command_by_transid(trans_id, itf_status->itf_id);
    assert(command != NULL);

    alp_command_t* resp = alp_layer_command_alloc(false, false);
    resp->forward_itf_id = itf_status->itf_id;
    resp->is_response = true;
    resp->tag_id = command->tag_id;
    resp->origin_itf_id = itf_status->itf_id;
    alp_append_interface_status(resp, itf_status);
    // we do not add a tag response, since we know the original command using the trans_id
    fifo_put(&resp->alp_command_fifo, payload, payload_length);
    //alp_layer_process(resp->alp_command, fifo_get_size(&resp->alp_command_fifo));
    alp_layer_process(resp);
}

#ifdef MODULE_D7AP
void alp_layer_process_d7aactp(d7ap_session_config_t* session_config, uint8_t* alp_command, uint32_t alp_command_length)
{
    // TODO refactor, might be removed
//    alp_command_t* command = alp_layer_command_alloc(false);
//    assert(command != NULL);

//    memcpy(command->alp_command, alp_command, alp_command_length);
//    fifo_init_filled(&(command->alp_command_fifo), command->alp_command, alp_command_length, ALP_PAYLOAD_MAX_SIZE);
//    fifo_init(&(command->alp_response_fifo), command->alp_response, ALP_PAYLOAD_MAX_SIZE);

//    alp_layer_parse_and_execute_alp_command(command);

//    uint8_t expected_response_length = alp_get_expected_response_length(command->alp_response_fifo);
//    error_t error = d7ap_send(alp_client_id, session_config, command->alp_response,
//        fifo_get_size(&(command->alp_response_fifo)), expected_response_length, &command->trans_id);

//    if (error)
//    {
//        DPRINT("d7ap_send returned an error %x", error);
//        free_command(command);
//    }
}
#endif // MODULE_D7AP

#ifdef MODULE_LORAWAN

#endif
