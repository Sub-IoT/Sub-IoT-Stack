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
#include "fs.h"
#include "log.h"
#include "dae.h"
#include "platform_defs.h"
#include "modem_interface.h"

// This example application contains a modem which can be used from another MCU through
// the serial interface

void bootstrap()
{
    log_print_string("Device booted\n");

    fs_init_args_t fs_init_args = (fs_init_args_t){
        .fs_d7aactp_cb = &alp_layer_process_d7aactp,
        .access_profiles_count = DEFAULT_ACCESS_PROFILES_COUNT,
        .access_profiles = default_access_profiles,
        .access_class = 0x21
    };

    fs_init(&fs_init_args);
    d7ap_init();

    alp_layer_init(NULL, true);

    uint8_t uid[8];
    fs_read_uid(uid);
    log_print_string("UID %02X%02X%02X%02X%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);

//#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
    // console will be enabled when the interrupt line is toggled
    // TODO remove when serial ALP is removed from console
    //console_disable();
    modem_interface_disable();
//#endif
}

