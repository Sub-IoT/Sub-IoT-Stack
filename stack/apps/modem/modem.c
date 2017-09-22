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
#include "assert.h"

#include <stdio.h>
#include <stdlib.h>

#include "d7ap_stack.h"
#include "fs.h"
#include "log.h"

// This example application contains a modem which can be used from another MCU through
// the serial interface

void bootstrap()
{
    log_print_string("Device booted\n");

    dae_access_profile_t access_classes[1] = {
        {
            .channel_header = {
                .ch_coding = PHY_CODING_PN9,
                .ch_class = PHY_CLASS_NORMAL_RATE,
                .ch_freq_band = PHY_BAND_868
            },
            .subprofiles[0] = {
                .subband_bitmap = 0x00, // void scan automation channel list
                .scan_automation_period = 0,
            },
            .subbands[0] = (subband_t){
                .channel_index_start = 0,
                .channel_index_end = 0,
                .eirp = 10,
                .cca = 86,
                .duty = 0,
            }
        }
    };

    fs_init_args_t fs_init_args = (fs_init_args_t){
        .access_profiles_count = 1,
        .access_profiles = access_classes,
        .access_class = 0x01
    };

    d7ap_stack_init(&fs_init_args, NULL, true, NULL);
}

