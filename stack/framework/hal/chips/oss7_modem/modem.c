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
#include "assert.h"
#include "log.h"
#include "errors.h"
#include "fifo.h"
#include "alp.h"
#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>

#define RX_BUFFER_SIZE 256



typedef struct {
  uint8_t tag_id;
  uint8_t resp_size;
  bool is_active;
  fifo_t fifo;
  uint8_t* buffer;
} command_t;

static uart_handle_t* uart_handle;
static modem_command_completed_callback_t command_completed_cb;
static fifo_t rx_fifo;
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static command_t command; // TODO only one active command supported for now
static uint8_t current_tag_id = 0;
static bool parsed_header = false;
static uint8_t payload_len = 0;

static void process_serial_frame(fifo_t* fifo) {
  alp_action_t action;
  alp_parse_action(fifo, &action);

  // TODO tmp
  log_print_string("action: ");
  log_print_data(action.file_data_operand.data, action.file_data_operand.provided_data_length);

  // TODO in loop
}

static void process_rx_fifo() {
  if(!parsed_header) {
    // <sync byte (0xC0)><version (0x00)><length of ALP command (1 byte)><ALP command> // TODO CRC
    if(fifo_get_size(&rx_fifo) > SERIAL_ALP_FRAME_HEADER_SIZE) {
        uint8_t header[SERIAL_ALP_FRAME_HEADER_SIZE];
        error_t err;
        fifo_peek(&rx_fifo, header, 0, SERIAL_ALP_FRAME_HEADER_SIZE);
        log_print_data(header, 3); // TODO tmp
        if(header[0] != SERIAL_ALP_FRAME_SYNC_BYTE || header[1] != SERIAL_ALP_FRAME_VERSION) {
          fifo_skip(&rx_fifo, 1);
          log_print_string("skip");
          parsed_header = false;
          payload_len = 0;
          if(fifo_get_size(&rx_fifo) > SERIAL_ALP_FRAME_HEADER_SIZE)
            sched_post_task(&process_rx_fifo);
        }

        parsed_header = true;
        fifo_skip(&rx_fifo, SERIAL_ALP_FRAME_HEADER_SIZE);
        payload_len = header[2];
        log_print_string("found header, payload size = %i", payload_len);
        sched_post_task(&process_rx_fifo);
    }
  } else {
    if(fifo_get_size(&rx_fifo) < payload_len) {
      log_print_string("payload not complete yet");
      return;
    }

    // payload complete, start parsing
    // rx_fifo can be bigger than the current serial packet, init a subview fifo
    // which is restricted to payload_len so we can't parse past this packet.
    fifo_t payload_fifo;
    fifo_init_subview(&payload_fifo, &rx_fifo, payload_len);
    process_serial_frame(&payload_fifo);

    // pop parsed bytes from original fifo
    log_print_string("payload_fifo size %i, payload_len %i", fifo_get_size(&payload_fifo), payload_len); // TODO tmp
    fifo_skip(&rx_fifo, payload_len - fifo_get_size(&payload_fifo));
  }
}

static void rx_cb(uint8_t byte) {
  fifo_put_byte(&rx_fifo, byte);
  if(!sched_is_scheduled(&process_rx_fifo))
    sched_post_task(&process_rx_fifo);
}

static void send(uint8_t* buffer, uint8_t len) {
  uint8_t header[] = {'A', 'T', '$', 'D', 0xC0, 0x00, len };
  uart_send_bytes(uart_handle, header, sizeof(header));
  uart_send_bytes(uart_handle, buffer, len);
  log_print_string("> %i bytes\n", len);
}

void modem_init(uart_handle_t* uart, modem_command_completed_callback_t cb) {
  assert(uart != NULL);
  assert(cb != NULL);

  uart_handle = uart;
  command_completed_cb = cb;
  fifo_init(&rx_fifo,rx_buffer, RX_BUFFER_SIZE);

  sched_register_task(&process_rx_fifo);

  // TODO for now we keep uart enabled so we can use RX IRQ.
  // can be optimized later if GPIO IRQ lines are implemented.
  assert(uart_enable(uart_handle));
  uart_set_rx_interrupt_callback(uart_handle, &rx_cb);
  assert(uart_rx_interrupt_enable(uart_handle) == SUCCESS);
}

void modem_execute_raw_alp(uint8_t* alp, uint8_t len) {
  send(alp, len);
}


bool modem_read_file(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer) {
  assert(output_buffer != NULL);

  if(command.is_active) {
    log_print_string("prev command still active");
    return false;
  }

  command.is_active = true;
  command.buffer = output_buffer;
  fifo_init(&command.fifo, command.buffer, ALP_OP_SIZE_REQUEST_TAG + ALP_OP_SIZE_READ_FILE_DATA);
  command.resp_size = size;
  command.tag_id = current_tag_id++;

  alp_append_tag_request(&command.fifo, command.tag_id, true);
  alp_append_read_file_data(&command.fifo, file_id, offset, size, true, false);

  send(command.buffer, fifo_get_size(&command.fifo));

  return true;
}

