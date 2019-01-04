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
#include "console.h"

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
#include "hwblockdevice.h"
#include "stm32_common_eeprom.h"

// This example application contains a modem which can be used from another MCU through
// the serial interface

static blockdevice_stm32_eeprom_t systemfiles_eeprom_blockdevice;

void bootstrap()
{
    log_print_string("Device booted\n");
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
    modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, MODEM2MCU_INT_PIN, MCU2MODEM_INT_PIN);
#else
    modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, (pin_id_t) 0, (pin_id_t) 0);
#endif

    // TODO remove
    fs_init_args_t fs_init_args = (fs_init_args_t){
        .fs_d7aactp_cb = &alp_layer_process_d7aactp,
    };

    systemfiles_eeprom_blockdevice = (blockdevice_stm32_eeprom_t){
      .base.driver = &blockdevice_driver_stm32_eeprom,
    };

    blockdevice_init((blockdevice_t*)&systemfiles_eeprom_blockdevice);

    d7ap_init((blockdevice_t*)&systemfiles_eeprom_blockdevice);
    alp_layer_init(NULL, true);

    uint8_t uid[8];
    d7ap_fs_read_uid(uid);
    log_print_string("UID %02X%02X%02X%02X%02X%02X%02X%02X\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);
}

