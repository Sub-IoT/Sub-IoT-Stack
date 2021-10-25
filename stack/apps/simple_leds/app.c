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

// this example is back to basics. We're only using the scheduler, timer and leds here.
// Toggling the leds at a different interval while the buttons can also control the leds

#include "hwleds.h"
#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include "debug.h"
#include "hwwatchdog.h"
#include "framework_defs.h"
#include "platform.h"

#ifdef FRAMEWORK_USE_POWER_TRACKING
	#error "This example can't be used in combination with FRAMEWORK_USE_POWER_TRACKING as the filesystem is not initialized here"
#endif

#define LED0_PERIOD TIMER_TICKS_PER_SEC
#define LED1_PERIOD TIMER_TICKS_PER_SEC * 2

// this is needed as we're not using the filesystem, also possible to remove blockdevices from platf_main.c
uint8_t d7ap_volatile_files_data[FRAMEWORK_FS_VOLATILE_STORAGE_SIZE];

#if PLATFORM_NUM_BUTTONS > 0
#include "button.h"

void userbutton_callback(button_id_t button_id)
{
	log_print_string("Button PB%u pressed.", button_id);
	led_toggle(0);
}

#endif

void led0_toggle_callback()
{
	led_toggle(0);
	timer_post_task_delay(&led0_toggle_callback, LED0_PERIOD);
	log_print_string("Toggled on 0");
}



void led1_toggle_callback()
{
	led_toggle(1);
	timer_post_task_delay(&led1_toggle_callback, LED1_PERIOD);
	log_print_string("Toggled led 1");
}

void bootstrap()
{
	led_on(0);
	led_on(1);

	log_print_string("Device booted at time: %d\n", timer_get_counter_value());

    sched_register_task(&led0_toggle_callback);
    sched_register_task(&led1_toggle_callback);

    timer_post_task_delay(&led0_toggle_callback, LED0_PERIOD);
    timer_post_task_delay(&led1_toggle_callback, LED1_PERIOD);

#if PLATFORM_NUM_BUTTONS > 0
    int i= 0;
    for (i=0;i<PLATFORM_NUM_BUTTONS;i++)
	{
    	ubutton_register_callback(i, &userbutton_callback);
	}
#endif

    led_off(0);
    led_off(1);
}
