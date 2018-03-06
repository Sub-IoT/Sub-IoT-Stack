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

// This is an example application where the stack is running on an a standalone MCU,
// typically used in combination with another MCU where the main application (for instance sensor reading)
// in running. The application accesses the stack using the serial modem interface.

#include "hwleds.h"
#include "hwsystem.h"
#include "hwuart.h"
#include "assert.h"

#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "modem.h"
#include "d7ap.h"


// This example application shows how to integrate a serial D7 modem

static uart_handle_t* modem_uart;

void command_completed_cb(bool with_error) {
  log_print_string("command completed!");
}

void return_file_data_cb(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* buffer) {
  if(file_id == D7A_FILE_UID_FILE_ID && size == D7A_FILE_UID_SIZE) {
    log_print_string("received modem uid:");
    log_print_data(buffer, size);
  }
}

modem_callbacks_t callbacks = (modem_callbacks_t){
  .command_completed_callback = &command_completed_cb,
  .return_file_data_callback = &return_file_data_cb,
};

void bootstrap()
{
  modem_uart = uart_init(1, 9600, 0);
  modem_init(modem_uart, &callbacks);
  modem_read_file(0, 0, 8); // try reading the UID file to make sure we are connected to a modem

  log_print_string("Device booted\n");
}

