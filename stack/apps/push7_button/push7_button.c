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
#include "adc_stuff.h"
#include "file_definitions.h"
#include "little_queue.h"

//#define FRAMEWORK_APP_LOG 1
#ifdef FRAMEWORK_APP_LOG
#include "log.h"
    #define DPRINT(...)      log_print_string(__VA_ARGS__)
    #define DPRINT_DATA(...) log_print_data(__VA_ARGS__)
#else
    #define DPRINT(...)
    #define DPRINT_DATA(...)
#endif

// debug
// cmake ../stack/ -DPLATFORM=PUSH7  -DAPP_PUSH7_BUTTON=y -DMODULE_ALP_SERIAL_INTERFACE_ENABLED=n
// -DFRAMEWORK_POWER_TRACKING_RF=n -DFRAMEWORK_USE_POWER_TRACKING=n -DMODULE_ALP_LOCK_KEY_FILES=n
// -DMODULE_D7AP_NLS_ENABLED=y -DFRAMEWORK_SCHEDULER_LP_MODE=1 -DFRAMEWORK_LOG_ENABLED=y  -DFRAMEWORK_DEBUG_ENABLE_SWD=y
// release
// cmake ../stack/ -DPLATFORM=PUSH7  -DAPP_PUSH7_BUTTON=y -DMODULE_ALP_SERIAL_INTERFACE_ENABLED=n
// -DFRAMEWORK_POWER_TRACKING_RF=n -DFRAMEWORK_USE_POWER_TRACKING=n -DMODULE_ALP_LOCK_KEY_FILES=n
// -DMODULE_D7AP_NLS_ENABLED=y -DFRAMEWORK_SCHEDULER_LP_MODE=1 -DFRAMEWORK_LOG_ENABLED=n -DFRAMEWORK_DEBUG_ENABLE_SWD=n


static void userbutton_callback(button_id_t button_id, uint8_t mask, uint8_t elapsed_deciseconds, buttons_state_t buttons_state)
{
    button_file_t button_file = 
    {
        .button_id=button_id,
        .mask=mask,
        .elapsed_deciseconds=elapsed_deciseconds,
        .buttons_state=buttons_state,
    };

    queue_add_file(button_file.bytes, BUTTON_FILE_SIZE, BUTTON_FILE_ID);
    DPRINT("Button callback - id: %d, mask: %d, elapsed time: %d, all_button_state %d \n", button_id, mask, elapsed_deciseconds, buttons_state);
}

void send_heartbeat()
{
    // uint64_t button_id = get_battery_voltage();
    // transmit_file(BUTTON_FILE_ID, 0, BUTTON_FILE_SIZE, (uint8_t*)&button_id);
    // timer_post_task_delay(&send_heartbeat, TIMER_TICKS_PER_SEC * 10);
}

void bootstrap()
{
    log_print_string("Device booted\n");
    
    little_queue_init();
    adc_stuff_init();

    led_flash_white();
    ubutton_register_callback(&userbutton_callback);

#ifndef FRAMEWORK_DEBUG_ENABLE_SWD
    GPIO_InitTypeDef GPIO_InitStruct= { 0 };
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif

}