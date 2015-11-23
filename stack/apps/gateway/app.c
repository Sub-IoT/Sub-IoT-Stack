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

/*
 * \author	maarten.weyn@uantwerpen.be
 */

#include "hwuart.h"
#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include "debug.h"
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>

#include "hwlcd.h"
#include "d7ap_stack.h"
#include "dll.h"
#include "hwuart.h"
#include "fifo.h"
#include "alp_cmd_handler.h"

static d7asp_init_args_t d7asp_init_args;

static void on_unsollicited_response_received(d7asp_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size, hw_rx_metadata_t* rx_meta)
{
    // TODO move to uart_alp_interface module
    uart_transmit_data(ALP_ITF_ID_D7ASP);
    uart_transmit_data(d7asp_result.status.raw);
    uart_transmit_data(d7asp_result.fifo_token);
    uart_transmit_data(d7asp_result.request_id);
    uart_transmit_data(d7asp_result.response_to);
    uart_transmit_data(d7asp_result.addressee->addressee_ctrl);
    uint8_t address_len = d7asp_result.addressee->addressee_ctrl_virtual_id? 2 : 8; // TODO according to spec this can be 1 byte as well?
    uart_transmit_message(d7asp_result.addressee->addressee_id, address_len);
    uart_transmit_message(alp_command, alp_command_size);
}

static void notify_booted()
{
    // TODO refactor
    //uint8_t alp_command[] = { 0x01, D7A_FILE_FIRMWARE_VERSION_FILE_ID, 0, D7A_FILE_FIRMWARE_VERSION_SIZE };
    //uart_alp_interface_process_command(alp_command, sizeof(alp_command));
}

void bootstrap()
{
    dae_access_profile_t access_classes[2] = {
        {
            .control_scan_type_is_foreground = true,
            .control_csma_ca_mode = CSMA_CA_MODE_UNC,
            .control_number_of_subbands = 1,
            .subnet = 0x05,
            .scan_automation_period = 0,
            .transmission_timeout_period = 0xFF,
            .subbands[0] = (subband_t){
                .channel_header = {
                    .ch_coding = PHY_CODING_PN9,
                    .ch_class = PHY_CLASS_NORMAL_RATE,
                    .ch_freq_band = PHY_BAND_433
                },
                .channel_index_start = 0, // TODO tmp
                .channel_index_end = 0, // TODO tmp
                .eirp = 0,
                .ccao = 0
            }
        },

        {
            .control_scan_type_is_foreground = true,
            .control_csma_ca_mode = CSMA_CA_MODE_RIGD,
            .control_number_of_subbands = 1,
            .subnet = 0x05,
            .scan_automation_period = 0,
            .transmission_timeout_period = 120,
            .subbands[0] = (subband_t){
                .channel_header = {
                        .ch_coding = PHY_CODING_PN9,
                        .ch_class = PHY_CLASS_LO_RATE,
                        .ch_freq_band = PHY_BAND_433
                },
                .channel_index_start = 16,
                .channel_index_end = 16,
                .eirp = 0,
                .ccao = 0
             }
        }
    };

    fs_init_args_t fs_init_args = (fs_init_args_t){
        .fs_user_files_init_cb = NULL,
        .access_profiles_count = 2,
        .access_profiles = access_classes
    };

    d7asp_init_args.d7asp_received_unsollicited_data_cb = &on_unsollicited_response_received;

    d7ap_stack_init(&fs_init_args, &d7asp_init_args, true);

    fs_write_dll_conf_active_access_class(1); // use access class 1 for scan automation

    lcd_write_string("started");

    notify_booted();
}

