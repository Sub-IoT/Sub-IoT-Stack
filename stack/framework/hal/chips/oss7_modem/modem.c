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

#include <stdio.h>
#include <stdlib.h>


typedef struct {
  uart_handle_t* uart_handle;
  modem_command_completed_callback_t command_completed_cb;
} modem_t;

typedef struct {
  uint8_t tag_id;
  uint8_t resp_size;
  bool is_active;
  fifo_t fifo;
  uint8_t* buffer;
} command_t;

static modem_t modem;
static command_t command; // TODO only one active command supported for now
static uint8_t current_tag_id = 0;

static void rx_cb(uint8_t byte) {
  log_print_string("< %x\n", byte);
}

static void send(uint8_t* buffer, uint8_t len) {
  uint8_t header[] = {'A', 'T', '$', 'D', 0xC0, 0x00, len };
  uart_send_bytes(modem.uart_handle, header, sizeof(header));
  uart_send_bytes(modem.uart_handle, buffer, len);
  log_print_string("> %i bytes\n", len);
}

void modem_init(uart_handle_t* uart_handle, modem_command_completed_callback_t cb) {
  assert(uart_handle != NULL);
  assert(cb != NULL);

  modem.uart_handle = uart_handle;
  modem.command_completed_cb = cb;

  // TODO for now we keep uart enabled so we can use RX IRQ.
  // can be optimized later if GPIO IRQ lines are implemented.
  assert(uart_enable(modem.uart_handle));
  uart_set_rx_interrupt_callback(modem.uart_handle, &rx_cb);
  assert(uart_rx_interrupt_enable(modem.uart_handle) == SUCCESS);
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

