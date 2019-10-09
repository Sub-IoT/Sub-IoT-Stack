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
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

#include "d7ap.h"
#include "alp_layer.h"
#include "d7ap_fs.h"
#include "log.h"
#include "dae.h"
#include "platform_defs.h"
#include "modem_interface.h"
#include "platform.h"

//This application contains a modem which can be used from another MCU through the serial interface

void bootstrap()
{
    log_print_string("Device booted\n");
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
    modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, MODEM2MCU_INT_PIN, MCU2MODEM_INT_PIN);
#else
    modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, (pin_id_t) 0, (pin_id_t) 0);
#endif

    d7ap_init();
    alp_layer_init(NULL, true);

    uint8_t uid[8];
    d7ap_fs_read_uid(uid);
    log_print_string("UID %02X%02X%02X%02X%02X%02X%02X%02X\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);

    uint8_t buffer[60] = {4, 0, 255, 48, 0, 0, 0, 0, 0};
    uint8_t buffer_f[60] = {0x0, 0x0, 0x0, 0x28, 0xe4, 0x0, 0x1, 0x33, 0x36, 0x0, 0x1, 0xeb, 0xac, 0x0, 0x0, 0x25, 0x80, 0x0, 0x0, 0x12, 0xc0, 0x0, 0x0, 0xd9, 0x3, 0x0, 0x0, 0xc3, 0x50, 0x0, 0x2, 0x8b, 0xb, 0x0, 0x0, 0xa2, 0xc3, 0x4, 0x5, 0x10, 0x3, 0x8, 0x2, 0x0};

    d7ap_fs_write_file(D7A_FILE_FACTORY_SETTINGS_FILE_ID, 0, buffer_f, D7A_FILE_FACTORY_SETTINGS_SIZE);

    d7ap_fs_write_file(D7A_FILE_ENGINEERING_MODE_FILE_ID, 0, buffer, D7A_FILE_ENGINEERING_MODE_SIZE);
}

