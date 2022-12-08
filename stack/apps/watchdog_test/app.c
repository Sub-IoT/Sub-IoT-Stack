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

// this example tests the watchdog. it will first set a timer to schedule a task right after a feed, then it will do some longer tasks
// of about 5 seconds each (which shouldn't be a problem), then it will execute tasks that are 20 seconds which should trigger the software 
// watchdog but not yet the hardware one. Then we do tasks that are too long and it should reboot the system

// Configuration cmake -D FRAMEWORK_USE_CALLSTACK=y -D FRAMEWORK_USE_ERROR_EVENT_FILE=y -D FRAMEWORK_USE_POWER_TRACKING=n -D APP_WATCHDOG_TEST=y -DMODULE_D7AP=n -DMODULE_ALP=n .

#include "error_event_file.h"
#include "d7ap_fs.h"
#include "scheduler.h"
#include "hwwatchdog.h"
#include "framework_defs.h"
#include "timer.h"
#include "platform.h"
#include "log.h"
#include "debug.h"

#ifndef FRAMEWORK_USE_WATCHDOG
	#error "Enable the watchdog for this example"
#endif

#ifndef FRAMEWORK_USE_ERROR_EVENT_FILE
	#error "Enable the error event file for this example"
#endif

static uint8_t blocking_task_count = 0;

void blocking_task() {
	uint32_t milliseconds_wait_time;
	if(blocking_task_count > 7)
		milliseconds_wait_time = 60000;
	else if (blocking_task_count > 5)
		milliseconds_wait_time = 20000;
	else
		milliseconds_wait_time = 5000;
	
	for(uint32_t i = 0; i < milliseconds_wait_time; i++)
		hw_busy_wait(650);

	log_print_string("blocking task %i", blocking_task_count);
	blocking_task_count++;
	sched_post_task(&blocking_task);
}

void bootstrap()
{
	log_print_string("Device booted at time: %d\n", timer_get_counter_value());
	d7ap_fs_init();
	error_event_file_init(&low_level_read_cb, &low_level_write_cb);
    sched_register_task((task_t)&blocking_task);
	timer_post_task_delay(&blocking_task, TIMER_TICKS_PER_SEC * 36 + 3);
}
