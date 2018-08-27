/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "modem.h"
#include "debug.h"
#include "log.h"
#include "errors.h"
#include "fifo.h"
#include "alp.h"
#include "scheduler.h"
#include "timer.h"
#include "d7ap.h"
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"

#define RX_BUFFER_SIZE 256
#define CMD_BUFFER_SIZE 256

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_MODEM_LOG_ENABLED)
  #define DPRINT(...) log_print_string(__VA_ARGS__)
  #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
    #define DPRINT(...)
    #define DPRINT_DATA(...)
#endif


typedef struct {
  uint8_t tag_id;
  bool is_active;
  fifo_t fifo;
  uint8_t buffer[256];
} command_t;

static uart_handle_t* uart_handle;
static modem_callbacks_t* callbacks;
static fifo_t rx_fifo;
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static command_t command; // TODO only one active command supported for now
static uint8_t next_tag_id = 0;
static bool parsed_header = false;
static uint8_t payload_len = 0;

static void process_serial_frame(fifo_t* fifo) {
  bool command_completed = false;
  bool completed_with_error = false;
  while(fifo_get_size(fifo)) {
    alp_action_t action;
    alp_parse_action(fifo, &action);

    switch(action.operation) {
      case ALP_OP_RETURN_TAG:
        if(action.tag_response.tag_id == command.tag_id) {
          command_completed = action.tag_response.completed;
          completed_with_error = action.tag_response.error;
        } else {
          DPRINT("received resp with unexpected tag_id (%i vs %i)", action.tag_response.tag_id, command.tag_id);
          // TODO unsolicited responses
        }
        break;
      case ALP_OP_WRITE_FILE_DATA:
        if(callbacks->write_file_data_callback)
          callbacks->write_file_data_callback(action.file_data_operand.file_offset.file_id,
                                               action.file_data_operand.file_offset.offset,
                                               action.file_data_operand.provided_data_length,
                                               action.file_data_operand.data);
        break;
      case ALP_OP_RETURN_FILE_DATA:
        if(callbacks->return_file_data_callback)
          callbacks->return_file_data_callback(action.file_data_operand.file_offset.file_id,
                                               action.file_data_operand.file_offset.offset,
                                               action.file_data_operand.provided_data_length,
                                               action.file_data_operand.data);
        break;
      case ALP_OP_RETURN_STATUS: ;
        d7ap_session_result_t interface_status = *((d7ap_session_result_t*)action.status.data);
        uint8_t addressee_len = d7ap_addressee_id_length(interface_status.addressee.ctrl.id_type);
        DPRINT("received resp from: ");
        DPRINT_DATA(interface_status.addressee.id, addressee_len);
        // TODO callback?
        break;
      default:
        assert(false);
    }
  }


  if(command_completed) {
    DPRINT("command with tag %i completed @ %i", command.tag_id, timer_get_counter_value());
    if(callbacks->command_completed_callback)
      callbacks->command_completed_callback(completed_with_error);

    command.is_active = false;
  }
}

static void process_rx_fifo(void *arg) {
  if(!parsed_header) {
    // <sync byte (0xC0)><version (0x00)><length of ALP command (1 byte)><ALP command> // TODO CRC
    if(fifo_get_size(&rx_fifo) > SERIAL_ALP_FRAME_HEADER_SIZE) {
        uint8_t header[SERIAL_ALP_FRAME_HEADER_SIZE];
        fifo_peek(&rx_fifo, header, 0, SERIAL_ALP_FRAME_HEADER_SIZE);
        DPRINT_DATA(header, 3); // TODO tmp

        if(header[0] != SERIAL_ALP_FRAME_SYNC_BYTE || header[1] != SERIAL_ALP_FRAME_VERSION) {
          fifo_skip(&rx_fifo, 1);
          DPRINT("skip");
          parsed_header = false;
          payload_len = 0;
          if(fifo_get_size(&rx_fifo) > SERIAL_ALP_FRAME_HEADER_SIZE)
            sched_post_task(&process_rx_fifo);

          return;
        }

        parsed_header = true;
        fifo_skip(&rx_fifo, SERIAL_ALP_FRAME_HEADER_SIZE);
        payload_len = header[2];
        DPRINT("found header, payload size = %i", payload_len);
        sched_post_task(&process_rx_fifo);
    }
  } else {
    if(fifo_get_size(&rx_fifo) < payload_len) {
      DPRINT("payload not complete yet");
      return;
    }

    // payload complete, start parsing
    // rx_fifo can be bigger than the current serial packet, init a subview fifo
    // which is restricted to payload_len so we can't parse past this packet.
    fifo_t payload_fifo;
    fifo_init_subview(&payload_fifo, &rx_fifo, 0, payload_len);
    process_serial_frame(&payload_fifo);

    // pop parsed bytes from original fifo
    fifo_skip(&rx_fifo, payload_len - fifo_get_size(&payload_fifo));
    parsed_header = false;
  }
}

static void rx_cb(uint8_t byte) {
  fifo_put_byte(&rx_fifo, byte);
  if(!sched_is_scheduled(&process_rx_fifo))
    sched_post_task_prio(&process_rx_fifo, MAX_PRIORITY, NULL);
}

static void send(uint8_t* buffer, uint8_t len) {
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
  platform_modem_wakeup();
#endif

  uint8_t header[] = {'A', 'T', '$', 'D', 0xC0, 0x00, len };
  uart_send_bytes(uart_handle, header, sizeof(header));
  uart_send_bytes(uart_handle, buffer, len);

#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
  platform_modem_release();
#endif

  DPRINT("> %i bytes @ %i", len, timer_get_counter_value());
}

void modem_init(uart_handle_t* uart, modem_callbacks_t* cbs) {
  assert(uart != NULL);

  uart_handle = uart;
  callbacks = cbs;
  fifo_init(&rx_fifo, rx_buffer, RX_BUFFER_SIZE);

  sched_register_task(&process_rx_fifo);

  // TODO for now we keep uart enabled so we can use RX IRQ.
  // can be optimized later if GPIO IRQ lines are implemented.
  assert(uart_enable(uart_handle));
  uart_set_rx_interrupt_callback(uart_handle, &rx_cb);
  assert(uart_rx_interrupt_enable(uart_handle) == SUCCESS);
}

void modem_reinit() {
  command.is_active = false;
}

bool modem_execute_raw_alp(uint8_t* alp, uint8_t len) {
  send(alp, len);
}

bool alloc_command() {
  if(command.is_active) {
    DPRINT("prev command still active @ %i", timer_get_counter_value());
    return false;
  }

  command.is_active = true;
  fifo_init(&command.fifo, command.buffer, CMD_BUFFER_SIZE);
  command.tag_id = next_tag_id;
  next_tag_id++;

  alp_append_tag_request_action(&command.fifo, command.tag_id, true);
  return true;
}

bool modem_read_file(uint8_t file_id, uint32_t offset, uint32_t size) {
  if(!alloc_command())
    return false;

  alp_append_read_file_data_action(&command.fifo, file_id, offset, size, true, false);

  send(command.buffer, fifo_get_size(&command.fifo));

  return true;
}

bool modem_write_file(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* data) {
  if(!alloc_command())
    return false;

  alp_append_write_file_data_action(&command.fifo, file_id, offset, size, data, true, false);

  send(command.buffer, fifo_get_size(&command.fifo));

  return true;
}

bool modem_send_unsolicited_response(uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data,
                                     d7ap_session_config_t* d7_interface_config) {
  if(!alloc_command())
    return false;

  alp_append_forward_action(&command.fifo, ALP_ITF_ID_D7ASP, (uint8_t *)d7_interface_config, sizeof(d7ap_session_config_t));
  alp_append_return_file_data_action(&command.fifo, file_id, offset, length, data);

  send(command.buffer, fifo_get_size(&command.fifo));
  return true;
}

bool modem_send_raw_unsolicited_response(uint8_t* alp_command, uint32_t length,
                                         d7ap_session_config_t* d7_interface_config) {
  if(!alloc_command())
    return false;

  alp_append_forward_action(&command.fifo, ALP_ITF_ID_D7ASP, (uint8_t *)d7_interface_config, sizeof(d7ap_session_config_t));
  fifo_put(&command.fifo, alp_command, length);

  send(command.buffer, fifo_get_size(&command.fifo));
  return true;
}
