/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
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
#include "framework_defs.h"


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

bool fwd_unsollicited_serial;

static alp_command_t NGDEF(_commands)[MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT];
#define commands NG(_commands)

static alp_init_args_t* NGDEF(_init_args);
#define init_args NG(_init_args)

static uint8_t previous_interface_file_id = 0;
static bool interface_file_changed = true;
static alp_interface_config_t session_config_saved;
static uint8_t alp_data[ALP_PAYLOAD_MAX_SIZE]; // temp buffer statically allocated to prevent runtime stackoverflows
static uint8_t alp_data2[ALP_QUERY_COMPARE_BODY_MAX_SIZE]; // temp buffer statically allocated to prevent runtime stackoverflows

extern alp_interface_t* interfaces[MODULE_ALP_INTERFACE_CNT];

static itf_ctrl_t current_itf_ctrl;

static void process_async(void* arg);

static uint8_t next_tag_id = 0;

static fifo_t command_fifo;
static alp_command_t* command_fifo_buffer[MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT];

#ifdef MODULE_D7AP
/*!
 * \brief Process a command triggered by D7A Action Protocol, where the resut of the command will be transmitted over the supplied interface
 * \param interface_config The interface config to transmit the response on
 * \param alp_command The ALP command to execute
 * \param alp_command_length Length of the command
 */
static void alp_layer_process_d7aactp(uint8_t* interface_config, uint8_t* alp_command, uint32_t alp_command_length);
#endif

static void free_command(alp_command_t* command) {
  DPRINT("!!! Free cmd %02x %p", command->trans_id, command);
  memset(command, 0, sizeof (alp_command_t));
  command->is_active = false;
  fifo_init(&command->alp_command_fifo, command->alp_command, ALP_PAYLOAD_MAX_SIZE);
}

void alp_layer_free_commands()
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
            DPRINT("alloc cmd %p in slot %i", &commands[i], i);
            if (with_tag_request) {
                next_tag_id++;
                if(!alp_append_tag_request_action(&commands[i], next_tag_id, always_respond))
                    return NULL;

                commands[i].tag_id = next_tag_id;
            }

            return &(commands[i]);
        }
    }

    DPRINT("Could not alloc command, all %i reserved slots active", MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT);
    
    return NULL;
}

bool alp_layer_command_free(alp_command_t* command)
{
    for (uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
        if (&commands[i] == command) {
            free_command(command);
            return true;
        }
    }
    
    DPRINT("Could not free command");
    return false;
}

alp_command_t* alp_layer_get_command_by_tag_id(uint8_t tag_id) {
    for(uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++)
        if(commands[i].tag_id == tag_id)
            return &(commands[i]);
    return NULL;
}

void alp_layer_free_itf_commands(uint8_t forward_itf_id)
{
    for (uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
        if (commands[i].forward_itf_id == forward_itf_id) {
            free_command(&commands[i]);
        }
    }
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

static alp_command_t* alp_layer_get_command_by_transid(uint16_t trans_id, uint8_t itf_id) {
  for(uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++) {
    if(commands[i].forward_itf_id == itf_id && commands[i].trans_id == trans_id && commands[i].is_active) {
        DPRINT("command trans Id %i in slot %i\n", trans_id, i);
        return &(commands[i]);
    }
  }

  DPRINT("No active command found with transaction Id = %i transmitted over itf %i\n", trans_id, itf_id);
  return NULL;
}

static alp_status_codes_t alp_translate_error(int rc)
{
    switch (rc) {
    case -ENOENT:
        return ALP_STATUS_FILE_ID_NOT_EXISTS;
    case -EINVAL:
        return ALP_STATUS_LENGTH_OUT_OF_BOUNDS;
    case -ESIZE:
        return ALP_STATUS_WRONG_OPERAND_FORMAT;
    case -ENOBUFS:
        return ALP_STATUS_DATA_OVERFLOW;
    case -EFBIG:
        return ALP_STATUS_DATA_OVERFLOW;
    case -EBADF:
        return ALP_STATUS_FILE_ID_OUT_OF_BOUNDS;
    case -ENOEXEC:
        return ALP_STATUS_UNKNOWN_OPERATION;
    case -EPERM:
        return ALP_STATUS_NOT_YET_IMPLEMENTED;
    case -EFAULT:
        return ALP_STATUS_FIFO_OUT_OF_BOUNDS;
    case -EEXIST:
        return ALP_STATUS_FILE_ID_ALREADY_EXISTS;
    case -EILSEQ:
        return ALP_STATUS_WRONG_OPERAND_FORMAT;
    case -EACCES:
        return ALP_STATUS_INSUFFICIENT_PERMISSIONS;
    default:
        return ALP_STATUS_UNKNOWN_ERROR;
    }
}
static void itf_clear_commands(uint8_t itf_id)
{
    DPRINT("Clear commands for itf %d", itf_id);
    for(uint8_t i = 0; i < MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT; i++)
        {
            if(commands[i].is_active && (commands[i].forward_itf_id == itf_id))
            {
                DPRINT("clear command with tag: %i", commands[i].tag_id);
                error_t err = ALP_STATUS_ITF_STOPPED;
                alp_interface_status_t empty_itf_status = { .itf_id = commands[i].forward_itf_id, .len = 0 };
                alp_layer_forwarded_command_completed(commands[i].trans_id, &err, &empty_itf_status, true);
            }
        }
}

static void itf_ctrl_file_callback(uint8_t file_id)
{
    error_t err;
    uint32_t file_length = USER_FILE_ALP_CTRL_SIZE;
    err = d7ap_fs_read_file(
        USER_FILE_ALP_CTRL_FILE_ID, 0, (uint8_t*)&current_itf_ctrl.raw_itf_ctrl, &file_length, ROOT_AUTH);
    if (err != SUCCESS) {
        log_print_error_string("alp_layer: stack ctrl file callback: read file returned error %i", err);
        current_itf_ctrl = (itf_ctrl_t) {
            .action = ITF_STOP,
            .interface = 0
        };
    }
    if (current_itf_ctrl.action == ITF_STOP) {
        if (!current_itf_deinit)
            return;
        current_itf_deinit();
        current_itf_deinit = NULL;

        // clean up all alp commands associated with the interface
        itf_clear_commands(current_itf_ctrl.interface);
    } else {
        for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_CNT; i++) {
            if(interfaces[i] && (interfaces[i]->itf_id == current_itf_ctrl.interface)) {
                if (interfaces[i]->unique && interfaces[i]->init) {
                    if (current_itf_deinit) {
                        if (interfaces[i]->deinit == current_itf_deinit) // interface is already inited
                            return;
                        else
                            current_itf_deinit();
                    }
                    err = interfaces[i]->init();
                    current_itf_deinit = interfaces[i]->deinit;
                    if(err < 0) {
                        current_itf_deinit();
                        current_itf_deinit = NULL;
                        current_itf_ctrl.action = ITF_STOP;
                        log_print_error_string("failed to init interface %i with error %i. Stopping", current_itf_ctrl.interface, err);
                    }
                }
                return;
            }
        }
        log_print_error_string(
            "tried to start an interface (%i) that is not registered", current_itf_ctrl.interface);
    }
}

static void init_auth_key_files()
{
#ifdef MODULE_ALP_LOCK_KEY_FILES
    d7ap_fs_file_header_t file_header = {
        .file_permissions = (file_permission_t) { .guest_read = false, .guest_write = false, .user_read = false, .user_write = false},
        .file_properties.storage_class = FS_STORAGE_PERMANENT,
        .length = ALP_AUTH_KEY_FILE_LENGTH,
        .allocated_length = ALP_AUTH_KEY_FILE_LENGTH
    };
#else
    d7ap_fs_file_header_t file_header = {
        .file_permissions = (file_permission_t) { .guest_read = true, .guest_write = true, .user_read = true, .user_write = true},
        .file_properties.storage_class = FS_STORAGE_PERMANENT,
        .length = ALP_AUTH_KEY_FILE_LENGTH,
        .allocated_length = ALP_AUTH_KEY_FILE_LENGTH
    };
#endif

    int rc = d7ap_fs_init_file(ALP_FILE_ID_ROOT_AUTH_KEY, &file_header, NULL);
    if(rc != -EEXIST && rc != SUCCESS) {
        log_print_error_string("Error initing ALP root auth key file: %d", rc);
    }

    rc = d7ap_fs_init_file(ALP_FILE_ID_USER_AUTH_KEY, &file_header, NULL);
    if(rc != -EEXIST && rc != SUCCESS) {
        log_print_error_string("Error initing ALP user auth key file: %d", rc);
    }

    // TODO how to handle situation where these files are not inited correctly?
}

void alp_layer_init(alp_init_args_t* alp_init_args, bool forward_unsollicited_over_serial)
{
  init_args = alp_init_args;
  fwd_unsollicited_serial = forward_unsollicited_over_serial;
  fifo_init(&command_fifo, (uint8_t*)command_fifo_buffer, MODULE_ALP_MAX_ACTIVE_COMMAND_COUNT*sizeof(alp_command_t*));
  alp_layer_free_commands();

  d7ap_fs_init();
#ifdef MODULE_D7AP
  d7ap_fs_set_process_d7aactp_callback(&alp_layer_process_d7aactp);
#endif
  init_auth_key_files();
#ifdef FRAMEWORK_MODEM_INTERFACE_ENABLED
  serial_interface_register();
#else
  assert(!fwd_unsollicited_serial);
#endif

#ifdef MODULE_D7AP
  d7ap_interface_register();
#endif

#ifdef MODULE_LORAWAN
  lorawan_interface_register();
#endif

  sched_register_task(&process_async);

  if(fs_is_file_defined(USER_FILE_ALP_CTRL_FILE_ID)) {
    d7ap_fs_register_file_modified_callback(USER_FILE_ALP_CTRL_FILE_ID, &itf_ctrl_file_callback);
    itf_ctrl_file_callback(USER_FILE_ALP_CTRL_FILE_ID);
  } else
    current_itf_ctrl.raw_itf_ctrl = 0;
}

void alp_layer_register_interface(alp_interface_t* interface) {
  alp_register_interface(interface);
}

static alp_status_codes_t process_op_read_file_data(alp_action_t* action, alp_command_t* resp_command, alp_command_t* command, authentication_t origin_auth)
{
    alp_operand_file_data_request_t operand = action->file_data_request_operand;
    DPRINT("READ FILE %i LEN %i OFFSET %i", operand.file_offset.file_id, operand.requested_data_length, operand.file_offset.offset);

    if (operand.requested_data_length <= 0 || operand.requested_data_length > ALP_PAYLOAD_MAX_SIZE)
        return ALP_STATUS_EXCEEDS_MAX_ALP_SIZE;

    int rc = d7ap_fs_read_file(operand.file_offset.file_id, operand.file_offset.offset, alp_data, &operand.requested_data_length, origin_auth);
    
    if (rc == -ENOENT && init_args != NULL && init_args->alp_unhandled_read_action_cb != NULL) // give the application layer the chance to fullfill this request ...
        rc = init_args->alp_unhandled_read_action_cb(&command->origin_itf_status, operand, alp_data);

    if (rc == SUCCESS) {
        // fill response
        if(!alp_append_return_file_data_action(resp_command, operand.file_offset.file_id, operand.file_offset.offset, operand.requested_data_length, alp_data))
            return ALP_STATUS_FIFO_OUT_OF_BOUNDS;

    } else
        return alp_translate_error(rc);


    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_read_file_properties(alp_action_t* action, alp_command_t* resp_command)
{
    error_t err;
    
    DPRINT("READ FILE PROPERTIES %i", action->file_id_operand.file_id);

    d7ap_fs_file_header_t file_header;
    err = d7ap_fs_read_file_header(action->file_id_operand.file_id, &file_header);
    if (err != SUCCESS) 
        return alp_translate_error(err);
    
    // convert to big endian
    file_header.length = __builtin_bswap32(file_header.length);
    file_header.allocated_length = __builtin_bswap32(file_header.allocated_length);

    // fill response
    err = fifo_put_byte(&resp_command->alp_command_fifo, ALP_OP_RETURN_FILE_PROPERTIES);
    err += fifo_put_byte(&resp_command->alp_command_fifo, action->file_id_operand.file_id);
    err += fifo_put(&resp_command->alp_command_fifo, (uint8_t*)&file_header, sizeof(d7ap_fs_file_header_t));

    return err == SUCCESS ? ALP_STATUS_OK : ALP_STATUS_FIFO_OUT_OF_BOUNDS;
}

static alp_status_codes_t process_op_write_file_properties(alp_action_t* action, authentication_t origin_auth)
{
    DPRINT("WRITE FILE PROPERTIES %i", action->file_header_operand.file_id);
        
    int rc = d7ap_fs_write_file_header(action->file_header_operand.file_id, &action->file_header_operand.file_header, origin_auth);
    return rc == SUCCESS ? ALP_STATUS_OK : alp_translate_error(rc);
}

static alp_status_codes_t process_op_write_file_data(alp_action_t* action, authentication_t origin_auth) {
    DPRINT("WRITE FILE %i LEN %i OFFSET %i", action->file_data_operand.file_offset.file_id, action->file_data_operand.provided_data_length, action->file_data_operand.file_offset.offset);
    if (action->file_data_operand.provided_data_length > ALP_PAYLOAD_MAX_SIZE)
        return ALP_STATUS_EXCEEDS_MAX_ALP_SIZE;
    
    int rc = d7ap_fs_write_file(action->file_data_operand.file_offset.file_id, action->file_data_operand.file_offset.offset,
        action->file_data_operand.data, action->file_data_operand.provided_data_length, origin_auth);
    return rc == SUCCESS ? ALP_STATUS_OK : alp_translate_error(rc);
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

  log_print_error_string("process_arithm_predicate type %i got to unreachable code", comp_type);
  return false; //not implemented type should always fail
}

static alp_status_codes_t process_op_break_query(alp_action_t* action, authentication_t origin_auth)
{
    
    DPRINT("BREAK QUERY");
    if(action->query_operand.code.type != QUERY_CODE_TYPE_ARITHM_COMP_WITH_VALUE_IN_QUERY)
        return ALP_STATUS_NOT_YET_IMPLEMENTED;
    
    if(action->query_operand.code.mask)
        return ALP_STATUS_NOT_YET_IMPLEMENTED;
    
    // parse arithm query params
    bool use_signed_comparison = true;
    if((action->query_operand.code.param & 0x08) == 0)
        use_signed_comparison = false;
    
    alp_query_arithmetic_comparison_type_t comp_type = action->query_operand.code.param & 0x07;

    // TODO assuming no compare mask for now + assume compare value present + only 1 file offset operand
    fifo_t temp_fifo;
    fifo_init_filled(&temp_fifo, action->query_operand.compare_body, ALP_QUERY_COMPARE_BODY_MAX_SIZE, ALP_QUERY_COMPARE_BODY_MAX_SIZE);
    
    memset(alp_data, 0, action->query_operand.compare_operand_length);
    if(fifo_pop(&temp_fifo, alp_data, action->query_operand.compare_operand_length) != SUCCESS)
        return ALP_STATUS_FIFO_OUT_OF_BOUNDS;
    alp_operand_file_offset_t offset_a;
    if(!alp_parse_file_offset_operand(&temp_fifo, &offset_a))
        return ALP_STATUS_FIFO_OUT_OF_BOUNDS;

    // make sure the uint32_t is word-aligned before passing it as a pointer
    uint32_t length = action->query_operand.compare_operand_length;

    int rc = d7ap_fs_read_file(offset_a.file_id, offset_a.offset, alp_data2, &length, origin_auth);
    if(rc != SUCCESS)
        return alp_translate_error(rc);
    
    if(!process_arithm_predicate(alp_data2, alp_data, action->query_operand.compare_operand_length, comp_type)) {
        //clear command?
        return ALP_STATUS_BREAK_QUERY_FAILED;
    }
    
    return ALP_STATUS_OK;
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
    alp_control_t ctrl;
    if ((previous_interface_file_id != action->indirect_interface_operand.interface_file_id)
        || interface_file_changed) {
        re_read = true;
        interface_file_changed = false;
        if (previous_interface_file_id != action->indirect_interface_operand.interface_file_id) {
            if (fs_is_file_defined(action->indirect_interface_operand.interface_file_id)) {
                d7ap_fs_unregister_file_modified_callback(previous_interface_file_id);
                d7ap_fs_register_file_modified_callback(action->indirect_interface_operand.interface_file_id, &interface_file_changed_callback);
                uint32_t length = 1;
                d7ap_fs_read_file(action->indirect_interface_operand.interface_file_id, 0, itf_id, &length, ROOT_AUTH);
                previous_interface_file_id = action->indirect_interface_operand.interface_file_id;
                session_config_saved.itf_id = *itf_id;
            } else
                return ALP_STATUS_WRONG_OPERAND_FORMAT;
        } else {
            *itf_id = session_config_saved.itf_id;
        }
    } else {
        *itf_id = session_config_saved.itf_id;
    }

    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_CNT; i++) {
        if (*itf_id == interfaces[i]->itf_id) {
            session_config->itf_id = *itf_id;
            if (re_read) {
                session_config_saved.itf_id = *itf_id;
                uint32_t itf_cfg_len = interfaces[i]->itf_cfg_len;
                d7ap_fs_read_file(action->indirect_interface_operand.interface_file_id, 1,
                    session_config_saved.itf_config, &itf_cfg_len, ROOT_AUTH);
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
            DPRINT("indirect forward %02X", *itf_id);
            return ALP_STATUS_PARTIALLY_COMPLETED;
        }
    }
    DPRINT("interface %02X is not registered", *itf_id);
    return ALP_STATUS_WRONG_OPERAND_FORMAT;
}

static alp_status_codes_t process_op_forward(alp_action_t* action, uint8_t* itf_id, alp_interface_config_t* session_config)
{
    // TODO move session config to alp_command_t struct
    memcpy(session_config, &action->interface_config, sizeof(alp_interface_config_t));
    *itf_id = action->interface_config.itf_id;
    DPRINT("FORWARD %02X", session_config->itf_id);
    return ALP_STATUS_PARTIALLY_COMPLETED;
}

static alp_status_codes_t process_op_response_tag(alp_action_t* action, alp_command_t* command, uint8_t* tag_id, bool* is_response_completed, bool* is_response_error)
{
    *tag_id = action->tag_id_operand.tag_id;
    *is_response_completed = action->ctrl.b7;
    *is_response_error = action->ctrl.b6;
    DPRINT("tag response %i EOP %i, ERR %i\n", *tag_id, *is_response_completed, *is_response_error);
    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_status(alp_action_t* action, alp_command_t* command)
{
    if (!action->ctrl.b7 && !action->ctrl.b6) {
        //action status operation
        DPRINT("act status");
        //TODO implement handling of action status
    } else if (!action->ctrl.b7 && action->ctrl.b6) {
        //interface status operation
        memcpy(&command->origin_itf_status, &action->interface_status, sizeof(alp_interface_status_t));
        command->origin_itf_id = command->origin_itf_status.itf_id; 
        DPRINT("itf status (%i)", command->origin_itf_status.itf_id);
    } else
        return ALP_STATUS_NOT_YET_IMPLEMENTED;

    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_request_tag(alp_action_t* action, uint8_t* tag_id, bool* respond_when_completed)
{
    *tag_id = action->tag_id_operand.tag_id;
    *respond_when_completed = action->ctrl.b7;
    DPRINT("tag req %i, EOP %i", *tag_id, *respond_when_completed);
    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_return_file_data(
    alp_action_t* action, alp_command_t* unsollicited_response_command)
{

    DPRINT("Return file data (%i):", action->file_data_operand.file_offset.file_id);
    DPRINT("offset size: %d", action->file_data_operand.file_offset.offset);
    DPRINT("data size: %d", action->file_data_operand.provided_data_length);
    // fill unsollicited_response_command
    if (!alp_append_return_file_data_action(unsollicited_response_command,
            action->file_data_operand.file_offset.file_id, action->file_data_operand.file_offset.offset,
            action->file_data_operand.provided_data_length, action->file_data_operand.data))
        return ALP_STATUS_FIFO_OUT_OF_BOUNDS;

    unsollicited_response_command->is_unsollicited = true;
    return ALP_STATUS_OK;
}

static alp_status_codes_t process_op_create_file(alp_action_t* action) {
    DPRINT("CREATE FILE %i", action->file_header_operand.file_id);
    int rc = d7ap_fs_init_file(action->file_header_operand.file_id, &action->file_header_operand.file_header, NULL);
    return rc == SUCCESS ? ALP_STATUS_OK : alp_translate_error(rc);
}

static alp_status_codes_t write_itf_command(itf_ctrl_action_t action, authentication_t origin_auth)
{
    int rc = d7ap_fs_write_file(USER_FILE_ALP_CTRL_FILE_ID, 0, (uint8_t*)&action, 1, origin_auth); // gets handled in write file callback
    return rc == SUCCESS ? ALP_STATUS_OK : alp_translate_error(rc);
}

static alp_status_codes_t process_op_start_itf(alp_action_t* action, authentication_t origin_auth)
{
    DPRINT("START INTERFACE");
    return write_itf_command(ITF_START, origin_auth);
}

static alp_status_codes_t process_op_stop_itf(alp_action_t* action, authentication_t origin_auth)
{
    DPRINT("STOP INTERFACE");
    return write_itf_command(ITF_STOP, origin_auth);
}

static bool forward_command(alp_command_t* command, alp_interface_config_t* itf_config)
{
    alp_interface_status_t empty_itf_status = { .itf_id = 0, .len = 0 };

    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_CNT; i++) {
        if (command->forward_itf_id == interfaces[i]->itf_id) {
            if(interfaces[i] && interfaces[i]->unique) {
                if(current_itf_ctrl.action == ITF_STOP) {
                    command->trans_id = command->tag_id;
                    error_t err = ALP_STATUS_ITF_STOPPED;
                    empty_itf_status.itf_id = command->forward_itf_id;
                    alp_layer_forwarded_command_completed(command->trans_id, &err, &empty_itf_status, true);
                    log_print_error_string("tried to forward something over a unique itf while stack stop is active");
                    return true;
                } else if(interfaces[i]->deinit != current_itf_deinit) {
                    if(current_itf_deinit) {
                        current_itf_deinit();
                        itf_clear_commands(current_itf_ctrl.interface);
                    }
                    interfaces[i]->init();
                    current_itf_deinit = interfaces[i]->deinit;

                    current_itf_ctrl.interface = interfaces[i]->itf_id;
                    d7ap_fs_write_file_with_callback(USER_FILE_ALP_CTRL_FILE_ID, 0, (uint8_t*)&current_itf_ctrl.raw_itf_ctrl, USER_FILE_ALP_CTRL_SIZE, ROOT_AUTH, false);
                }
            }
            uint16_t forwarded_alp_size = fifo_get_size(&command->alp_command_fifo);
            if(forwarded_alp_size > ALP_PAYLOAD_MAX_SIZE)
                return false;

            fifo_pop(&command->alp_command_fifo, command->alp_command, forwarded_alp_size);
            DPRINT("Forwarding command:");
            DPRINT_DATA(command->alp_command, forwarded_alp_size);
            int expected_response_length = alp_get_expected_response_length(command);
            if(expected_response_length < 0) {
                free_command(command);
                return false;
            }
            command->forward_itf_id = itf_config->itf_id;
            error_t error = interfaces[i]->send_command(command->alp_command, forwarded_alp_size, expected_response_length, &command->trans_id, itf_config);
            if (command->trans_id == 0)
                command->trans_id = command->tag_id; // interface does not provide transaction tracking, using tag_id

            if (error) {
                DPRINT("transmit returned error %d", error);
                empty_itf_status.itf_id = command->forward_itf_id;
                alp_layer_forwarded_command_completed(command->trans_id, &error, &empty_itf_status, true);
            }

            DPRINT("forwarded over interface %02X", command->forward_itf_id);
            return true;
        }
    }
    DPRINT("interface %02X not registered, can therefore not be forwarded");
    return false;
}

static alp_interface_t* find_interface(uint8_t itf_id)
{
    for (uint8_t i = 0; i < MODULE_ALP_INTERFACE_CNT; i++) {
        if ((interfaces[i] != NULL) && (interfaces[i]->itf_id == itf_id)) {
            return interfaces[i];
        }
    }

    DPRINT("interface %i not found", itf_id);
    return NULL;
}

static error_t transmit_response(
    alp_command_t* resp, alp_itf_id_t transmit_itf, alp_interface_status_t* origin_itf_status)
{
    DPRINT("async response to ori itf %i completed %i", transmit_itf, resp->is_response_completed);
    // when the command originates from the app code call callbacks directly, since this is not a 'real' interface
    if (transmit_itf == ALP_ITF_ID_HOST) {
        if(init_args && init_args->alp_command_result_cb)
            init_args->alp_command_result_cb(resp, origin_itf_status);
        return SUCCESS;
    }

    if (origin_itf_status) {
        if (!alp_append_interface_status(resp, origin_itf_status))
            return ALP_STATUS_FIFO_OUT_OF_BOUNDS;
    }

    alp_interface_t* interface = find_interface(transmit_itf);
    if (interface == NULL) {
        DPRINT("interface %i not found", transmit_itf);
        assert(false); // Leaving this assert as it is technically not possible to answer a request without a registered
                       // interface
    }

    uint8_t alp_response_length = (uint8_t)fifo_get_size(&resp->alp_command_fifo);
    uint8_t expected_response_length = 0; // TODO alp_get_expected_response_length(&resp->alp_command_fifo);
    DPRINT("interface found, sending len %i, expect %i answer", alp_response_length, expected_response_length);
    return interface->send_command(
        resp->alp_command, alp_response_length, expected_response_length, &resp->trans_id, NULL);
}

static void process_async(void* arg)
{
    (void)arg; // suppress unused warning
    static alp_command_t* command = NULL;
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
    if(resp_command == NULL) {
        log_print_error_string("process async: alloc command failed for the response command, dropping command");
        free_command(command);
        return;
    }
    static alp_action_t action;
    bool error = false;

    authentication_t origin_auth;
    switch(command->origin_itf_id) {
    case ALP_ITF_ID_HOST:
        origin_auth = ROOT_AUTH;
        break;
    case ALP_ITF_ID_SERIAL:
    case ALP_ITF_ID_NFC:
        origin_auth = USER_AUTH;
        break;
    case ALP_ITF_ID_D7ASP:
    case ALP_ITF_ID_LORAWAN_OTAA:
        origin_auth = GUEST_AUTH;
        break;
    default: //this shouldn't happen but we don't want to give higher permission in any case
        origin_auth = GUEST_AUTH;
        break;
    }

    while (fifo_get_size(&command->alp_command_fifo) > 0) {
        if (!alp_parse_action(command, &action)) {
            log_print_error_string("parsing failed in process async, the action we tried could be %i",
                action.ctrl
                    .operation); // we are not sure here that the operation got read but could still be nice to know
            free_command(command);
            free_command(resp_command);
            sched_post_task(&process_async);
            return;
        }
        alp_status_codes_t alp_status;
        switch (action.ctrl.operation) {
        case ALP_OP_READ_FILE_DATA:
            alp_status = process_op_read_file_data(&action, resp_command, command, origin_auth);
            break;
        case ALP_OP_READ_FILE_PROPERTIES:
            alp_status = process_op_read_file_properties(&action, resp_command);
            break;
        case ALP_OP_WRITE_FILE_DATA:
            alp_status = process_op_write_file_data(&action, origin_auth);
            break;
        case ALP_OP_WRITE_FILE_PROPERTIES:
            alp_status = process_op_write_file_properties(&action, origin_auth);
            break;
        case ALP_OP_BREAK_QUERY:
            alp_status = process_op_break_query(&action, origin_auth);
            break;
        case ALP_OP_STATUS:
            alp_status = process_op_status(&action, command);
            break;
        case ALP_OP_RESPONSE_TAG:;
            uint8_t resp_tag_id;
            alp_status = process_op_response_tag(&action, command, &resp_tag_id, &command->is_response_completed, &command->is_response_error);
            command->is_response = true;
            if((command->tag_id != resp_tag_id) && (command->tag_id != 0)) {
                log_print_error_string("process_async: tag_id's don't sync up! (%i != %i != 0)", resp_tag_id, command->tag_id);
                free_command(command);
                free_command(resp_command);
                return;
            }
            command->tag_id = resp_tag_id;
            resp_command->is_unsollicited = false;
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
            alp_status = process_op_return_file_data(&action, resp_command);
            break;
        case ALP_OP_CREATE_FILE:
            alp_status = process_op_create_file(&action);
            break;
        case ALP_OP_START_ITF:
            alp_status = process_op_start_itf(&action, origin_auth);
            break;
        case ALP_OP_STOP_ITF:
            alp_status = process_op_stop_itf(&action, origin_auth);
            break;
        default:
            alp_status = ALP_STATUS_UNKNOWN_OPERATION;
        }
        if(alp_status != ALP_STATUS_OK && alp_status != ALP_STATUS_PARTIALLY_COMPLETED) {
            //should BREAK QUERY FAILED also return error?
            //TODO put error code in action status and send to requester
            error = (alp_status != ALP_STATUS_BREAK_QUERY_FAILED);
            if(error)
                log_print_error_string("process async process command %i went wrong with error code 0x%02X", action.ctrl.operation, alp_status);
            break;
        }

        if (command->origin_itf_id != ALP_ITF_ID_HOST && command->is_response) {
            DPRINT("Command with length %i is response from itf %i", fifo_get_size(&command->alp_command_fifo), command->origin_itf_id);
            break; // stop parsing and return the response since this is a response to a forward
        }

        if (command->forward_itf_id != ALP_ITF_ID_HOST) {
            if (!command->is_response) {
                forward_command(command, &forward_interface_config);
                free_command(resp_command); // command itself will be free-ed when interface responds with this command
                                            // with correct tag
                return;
            } else {
                // response received over interface we forwarded to, stop parsing to return rest of command
            }
        }
    }

#ifdef MODULE_D7AP
    if (command->use_d7aactp) {
        DPRINT("Using D7AActP, transmit response to the configured interface");
        resp_command->forward_itf_id = command->d7aactp_interface_config.itf_id;
        forward_command(resp_command, &command->d7aactp_interface_config);
        free_command(command);
        return;
    }
#endif
    
    DPRINT("command is_response %i , tag_id %i, completed %i, error %i, ori itf id %i, resp when completed %i\n",
            command->is_response, command->tag_id, command->is_response_completed, command->is_response_error, command->origin_itf_id, command->respond_when_completed);
    if (command->is_response) {
        // when the command is an async response to a preceding request we first find the original request and send the response to the origin itf
        // find original request
        alp_command_t* request_command = get_request_command(command->tag_id, command->origin_itf_id);
        if(request_command == NULL) { //we have no request, clean up
            log_print_error_string("process_async: found no request command for response command with tag %i and origin itf %i", command->tag_id, command->origin_itf_id);
            free_command(command);
            free_command(resp_command);
            return;
        }
        uint8_t cmd_size = fifo_get_size(&command->alp_command_fifo);
        DPRINT("async response to cmd tag %i, ori itf %i respond_when_compl %i with length %i", request_command->tag_id, request_command->origin_itf_id, request_command->respond_when_completed, cmd_size);
        if (cmd_size > 0 || request_command->respond_when_completed) {
            if(request_command->origin_itf_id != ALP_ITF_ID_HOST) {
                // when the request originates from another interface it will already contain a tagresponse, since we always request a tag on forwarding
                if(request_command->respond_when_completed) {
                    if(!alp_append_tag_response_action(resp_command, request_command->tag_id, command->is_response_completed, command->is_response_error)) {
                        log_print_error_string("process_async: command is response but no room (%i) in resp_command to add response tag", fifo_get_size(&resp_command->alp_command_fifo));
                        free_command(resp_command);
                        free_command(command);
                        free_command(request_command);
                        return;
                    }
                }
            }

            resp_command->is_response = true;
            resp_command->is_response_completed = command->is_response_completed;
            fifo_pop(&command->alp_command_fifo, alp_data, cmd_size);
            fifo_put(&resp_command->alp_command_fifo, alp_data, cmd_size);
            transmit_response(resp_command, request_command->origin_itf_id, &command->origin_itf_status);
        }

        if (command->is_response_completed) {
            if (request_command->origin_itf_id == ALP_ITF_ID_HOST && (init_args != NULL) && (init_args->alp_command_completed_cb != NULL)) {
                init_args->alp_command_completed_cb(request_command->tag_id, !command->is_response_error);
            }

            DPRINT("free orig req cmd");
            free_command(request_command);
        }
    } else {
        if (resp_command->is_unsollicited) {
            DPRINT("Unsollicited data with forward over serial enabled: %i", fwd_unsollicited_serial);
            // if serial is connected, transmit it over serial but also let the host know what happened
            if(fwd_unsollicited_serial)
                transmit_response(resp_command, ALP_ITF_ID_SERIAL, &command->origin_itf_status);
            transmit_response(resp_command, ALP_ITF_ID_HOST, &command->origin_itf_status);
        }

        // uint8_t resp_cmd_size = fifo_get_size(&resp_command->alp_command_fifo);
        // DPRINT("resp_cmd size %i", resp_cmd_size);
        // DPRINT_DATA(resp_command->alp_command, resp_cmd_size);
        
        //send response to command if required
        if (command->respond_when_completed) {
            //if ((resp_cmd_size == 0 && command->respond_when_completed) || resp_cmd_size > 0) {
            if (command->is_tag_requested && command->origin_itf_id != ALP_ITF_ID_HOST) {
                // make sure to respond when requested, even if there is no response payload
                // when the request originates from another interface it will already contain a tagresponse, since we always request a tag on forwarding
                // TODO set err flag
                if(!alp_append_tag_response_action(resp_command, command->tag_id, true, error)) {
                    log_print_error_string("process async: tag is requested but no room (%i) for tag", fifo_get_size(&resp_command->alp_command_fifo));
                    free_command(command);
                    free_command(resp_command);
                }

            }
            transmit_response(resp_command, command->origin_itf_id, NULL);
        }
    }
    
    free_command(resp_command);
    free_command(command);
    return;
}

bool alp_layer_process(alp_command_t* command)
{
    DPRINT_DATA(command->alp_command, fifo_get_size(&command->alp_command_fifo));
    int expected_response_length = alp_get_expected_response_length(command);
    if(expected_response_length < 0) {
        log_print_error_string("alp_layer_process, alp_get_expected_response_length failed with error: %i", expected_response_length);
        free_command(command);
        return false;
    }
    DPRINT("This ALP command will initiate a response containing <%d> bytes\n", expected_response_length);
    if (expected_response_length == 0) {
        command->respond_when_completed = false;
    }
    
    // add to fifo for later processing
    if(fifo_put(&command_fifo, (uint8_t*)&command, sizeof(alp_command_t*)) != SUCCESS) {
        free_command(command);
        return false;
    }
    error_t e = sched_post_task_prio(&process_async, MIN_PRIORITY, NULL);
    if((e != SUCCESS) && (e != -EALREADY))
        return false;
    
    return (expected_response_length > 0);
}


void alp_layer_forwarded_command_completed(uint16_t trans_id, error_t* error, alp_interface_status_t* status, bool command_completed)
{
    if(status == NULL) {
        log_print_error_string("forwarded command completed with NULL alp_interface_status");
        return;
    }
    DPRINT("alp_layer_forwarded_cmd_completed: with trans id %i and error location %i: value %i", trans_id, error, *error);
    alp_command_t* command = alp_layer_get_command_by_transid(trans_id, status->itf_id);
    if(command == NULL) {
        log_print_error_string("forwarded command completed failed as command with trans id %i and itf id %i not found", trans_id, status->itf_id);
        return;
    }
    DPRINT("resp for tag %i\n", command->tag_id);
    alp_command_t* resp = alp_layer_command_alloc(false, false);
    if(resp == NULL) {
        log_print_error_string("forwarded command completed failed as alloc of resp command failed");
        free_command(command);
        return;
    }
    error_t err = SUCCESS;
    err += !alp_append_interface_status(resp, status);
    err += !alp_append_tag_response_action(resp, command->tag_id, command_completed, *error != SUCCESS);

    if(err != SUCCESS) {
        log_print_error_string("forwarded command completed failed as alp appends failed on resp command");
        free_command(resp);
        free_command(command);
        return;
    }
    alp_layer_process(resp);
}

void alp_layer_received_response(uint16_t trans_id, uint8_t* payload, uint8_t payload_length, alp_interface_status_t* itf_status)
{
    DPRINT("alp layer received response: with trans id %i and length %i", trans_id, payload_length);
    alp_command_t* command = alp_layer_get_command_by_transid(trans_id, itf_status->itf_id);
    if(command == NULL) {
        log_print_error_string("received response failed as command with trans id %i and itf id %i not found", trans_id, itf_status->itf_id);
        return;
    }

    alp_command_t* resp = alp_layer_command_alloc(false, false);
    if(resp == NULL) {
        log_print_error_string("received response failed as alloc of resp command failed");
        free_command(command);
        return;
    }
    error_t err;
    err = !alp_append_interface_status(resp, itf_status);
    err += !alp_append_tag_response_action(resp, command->tag_id, false, false);
    resp->trans_id = trans_id;
    err += fifo_put(&resp->alp_command_fifo, payload, payload_length);
    if(err != SUCCESS) {
        log_print_error_string("received response failed as alp appends failed on resp command");
        free_command(resp);
        free_command(command);
        return;
    }
    alp_layer_process(resp);
}

#ifdef MODULE_D7AP
void alp_layer_process_d7aactp(uint8_t *interface_config, uint8_t* alp_command, uint32_t alp_command_length)
{
    alp_interface_config_t* alp_interface_config = (alp_interface_config_t*)interface_config;
    // TODO refactor, might be removed
    alp_command_t* command = alp_layer_command_alloc(false, false);
    if(command == NULL) {
        log_print_error_string("process d7aactp failed as alloc failed");
        return;
    }

    command->use_d7aactp = true;
    memcpy(&command->d7aactp_interface_config, alp_interface_config, sizeof(alp_interface_config_t));
    error_t e = fifo_put(&command->alp_command_fifo, alp_command, alp_command_length);
    if(e != SUCCESS) {
        log_print_error_string("process d7aactp failed as fifo put of %i bytes failed", alp_command_length);
        free_command(command);
        return;
    }
    
    alp_layer_process(command);
}
#endif // MODULE_D7AP

