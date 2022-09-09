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
#ifndef MODEM_INTERFACE_H
#define MODEM_INTERFACE_H

#include "fifo.h"
#include "hwsystem.h"
#include "serial_protocol.h"

typedef void (*modem_interface_cmd_handler_t)(fifo_t* cmd_fifo);
typedef void (*modem_interface_target_rebooted_callback_t)(system_reboot_reason_t reboot_reason);

/*
---------------HEADER(bytes)---------------------
|sync|sync|counter|message type|length|crc1|crc2|
-------------------------------------------------
*/

/** @brief Initialize the modem interface by registering
 *  tasks, initialising fifos/UART and registering callbacks/interrupts
 *  @return Void.
 */
void modem_interface_init(void);

/** @brief  Adds header to bytes containing sync bytes, counter, length and crc and puts it in UART fifo
 *  @param bytes Bytes that need to be transmitted
 *  @param length Length of bytes
 *  @param type type of message (SERIAL_MESSAGE_TYPE_ALP, SERIAL_MESSAGE_TYPE_PING_REQUEST, SERIAL_MESSAGE_TYPE_LOGGING, ...)
 *  @return Void.
 */
error_t modem_interface_transfer_bytes(uint8_t* bytes, uint8_t length, serial_message_type_t type);
/** @brief Transmits a string by adding a header and putting it in the UART fifo
 *  @param string Bytes that need to be transmitted
 *  @return error_t 	SUCCESS if the data is put into the Tx buffer.
 *                      ENOMEM if the data doesn't fit in the Tx buffer.
 */
error_t modem_interface_transfer(char* string);
/** @brief Registers callback to process a certain type of received UART data
 *  @param cmd_handler Pointer to function that processes the data
 *  @param type The type of data that needs to be processed by the given callback function
 *  @return error_t 	SUCCESS if the data is put into the Tx buffer.
 *                      ENOMEM if the data doesn't fit in the Tx buffer.
 */
error_t modem_interface_register_handler(modem_interface_cmd_handler_t cmd_handler, serial_message_type_t type);

void modem_interface_unregister_handler(serial_message_type_t type);

/** @brief Registers callback to be executed when the remote target reboots
 *  @param cb Callback function pointer
 */
error_t modem_interface_set_target_rebooted_callback(modem_interface_target_rebooted_callback_t cb);

void modem_interface_clear_handler();

#endif //MODEM_INTERFACE_H

