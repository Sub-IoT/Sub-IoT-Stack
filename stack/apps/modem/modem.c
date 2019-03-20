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

#include "hwradio.h"

void start(){
  em_init();
}

// packet callbacks only here to make hwradio_init() happy, not used
hw_radio_packet_t* alloc_packet_callback(uint8_t length) {
  assert(false);
}

void release_packet_callback(hw_radio_packet_t* p) {
  assert(false);
}

void bootstrap()
{
    log_print_string("Device booted\n");
#ifdef PLATFORM_USE_MODEM_INTERRUPT_LINES
    modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, MODEM2MCU_INT_PIN, MCU2MODEM_INT_PIN);
#else
    modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, (pin_id_t) 0, (pin_id_t) 0);
#endif

    blockdevice_init(d7_systemfiles_blockdevice);
    d7ap_init(d7_systemfiles_blockdevice);
    alp_layer_init(NULL, true);

    uint8_t uid[8];
    d7ap_fs_read_uid(uid);
    log_print_string("UID %02X%02X%02X%02X%02X%02X%02X%02X\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);

    sched_post_task(&start);
}

