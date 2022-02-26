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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hwlcd.h"
#include "hwleds.h"
#include "hwsystem.h"

#include "d7ap_fs.h"
#include "debug.h"
#include "log.h"
#include "scheduler.h"
#include "timer.h"

#include "alp_layer.h"
#include "d7ap.h"
#include "dae.h"

#include "platform.h"

#include "button.h"
#include "led.h"

// debug
// cmake ../stack/ -DPLATFORM=PUSH7  -DAPP_PUSH7_BUTTON=y -DMODULE_ALP_SERIAL_INTERFACE_ENABLED=n
// -DFRAMEWORK_POWER_TRACKING_RF=n -DFRAMEWORK_USE_POWER_TRACKING=n -DMODULE_ALP_LOCK_KEY_FILES=n
// -DMODULE_D7AP_NLS_ENABLED=y -DFRAMEWORK_SCHEDULER_LP_MODE=1 -DFRAMEWORK_LOG_ENABLED=y  -DFRAMEWORK_DEBUG_ENABLE_SWD=y
// release
// cmake ../stack/ -DPLATFORM=PUSH7  -DAPP_PUSH7_BUTTON=y -DMODULE_ALP_SERIAL_INTERFACE_ENABLED=n
// -DFRAMEWORK_POWER_TRACKING_RF=n -DFRAMEWORK_USE_POWER_TRACKING=n -DMODULE_ALP_LOCK_KEY_FILES=n
// -DMODULE_D7AP_NLS_ENABLED=y -DFRAMEWORK_SCHEDULER_LP_MODE=1 -DFRAMEWORK_LOG_ENABLED=n -DFRAMEWORK_DEBUG_ENABLE_SWD=n

#define BUTTON_FILE_ID 50
#define BUTTON_FILE_SIZE 1
#define CHANNEL_ID 100
#define USE_PUSH7_CHANNEL_SETTINGS false

static alp_init_args_t alp_init_args;
static uint8_t key[]
    = { 0X00, 0X01, 0X02, 0X03, 0X02, 0X01, 0X00, 0X01, 0X02, 0X03, 0X02, 0X01, 0X00, 0X01, 0X02, 0X03 };

// Define the D7 interface configuration used for sending the ALP command on

static alp_interface_config_d7ap_t itf_config = (alp_interface_config_d7ap_t){
  .itf_id = ALP_ITF_ID_D7ASP,
  .d7ap_session_config = {
    .qos = {
        .qos_resp_mode = SESSION_RESP_MODE_PREFERRED,
        .qos_retry_mode = SESSION_RETRY_MODE_NO
    },
    .dormant_timeout = 0,
    .addressee = {
        .ctrl = {
            .nls_method = AES_CTR,
            .id_type = ID_TYPE_NBID,
        },
        .access_class = 0x01, // use access profile 0 and select the first subprofile
        .id = { 3 }
    }
  }
};

void transmit_button_pressed_message(uint8_t pressed_button)
{
    // Generate ALP command.
    // We will be sending a return file data action, without a preceding file read request.
    // This is an unsolicited message, where we push the sensor data to the gateway(s).

    uint8_t button_id = pressed_button;
    alp_command_t* command
        = alp_layer_command_alloc(true, true); // alloc command. This will be freed when the command completes

    alp_append_forward_action(
        command, (alp_interface_config_t*)&itf_config, sizeof(itf_config)); // forward to the D7 interface

    alp_append_return_file_data_action(
        command, BUTTON_FILE_ID, 0, BUTTON_FILE_SIZE, (uint8_t*)&button_id); // add the return file data action

    alp_layer_process(command); // and finally execute this
}

void on_alp_command_completed_cb(uint8_t tag_id, bool success)
{
    if (success) {
        log_print_string("Command (%i) completed successfully", tag_id);
        led_flash_green();
    } else
        log_print_string("Command failed, no ack received");
}

void on_alp_command_result_cb(alp_command_t* alp_command, alp_interface_status_t* origin_itf_status)
{
    if (origin_itf_status && (origin_itf_status->itf_id == ALP_ITF_ID_D7ASP) && (origin_itf_status->len > 0)) {
        d7ap_session_result_t* d7_result = ((d7ap_session_result_t*)origin_itf_status->itf_status);
        log_print_string("recv response @ %i dB link budget from:", d7_result->rx_level);
        log_print_data(d7_result->addressee.id, d7ap_addressee_id_length(d7_result->addressee.ctrl.id_type));
    }
    log_print_string("response payload:");
    log_print_data(alp_command->alp_command, fifo_get_size(&alp_command->alp_command_fifo));
    fifo_skip(&alp_command->alp_command_fifo, fifo_get_size(&alp_command->alp_command_fifo));
}

static void userbutton_callback(button_id_t button_id, uint8_t mask, uint8_t number_of_calls)
{
    // transmit_button_pressed_message(button_id);
    log_print_string("Button callback - id: %d, mask: %d, calls: %d", button_id, mask, number_of_calls);
}

void send_heartbeat()
{
    transmit_button_pressed_message(1);
    timer_post_task_delay(&send_heartbeat, TIMER_TICKS_PER_SEC * 10);
}

void bootstrap()
{
    log_print_string("Device booted\n");
    d7ap_fs_init();
    d7ap_init();

    if (USE_PUSH7_CHANNEL_SETTINGS) {
        dae_access_profile_t push7_access_profile;
        d7ap_fs_read_access_class(0, &push7_access_profile);

        push7_access_profile.subbands[0].channel_index_start = CHANNEL_ID;
        push7_access_profile.subbands[0].channel_index_end = CHANNEL_ID;
        push7_access_profile.subbands[0].eirp = 5;
        push7_access_profile.subbands[1].channel_index_start = CHANNEL_ID;
        push7_access_profile.subbands[1].channel_index_end = CHANNEL_ID;
        push7_access_profile.subbands[1].eirp = 0;

        d7ap_fs_write_access_class(0, &push7_access_profile);

        uint32_t length = D7A_FILE_NWL_SECURITY_KEY_SIZE;
        d7ap_fs_write_file(D7A_FILE_NWL_SECURITY_KEY, 0, key, length, ROOT_AUTH);
    }
    led_flash_green();

    alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
    alp_init_args.alp_command_result_cb = &on_alp_command_result_cb;
    alp_layer_init(&alp_init_args, false);

    sched_register_task(&send_heartbeat);
    timer_post_task_delay(&send_heartbeat, TIMER_TICKS_PER_SEC * 6);

    ubutton_register_callback(&userbutton_callback);
}
