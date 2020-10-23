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

#include "hwleds.h"
#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include "debug.h"
#include "hwwatchdog.h"

#include "platform.h"

static void _task_immediate(void* _) { 
	log_print_string("task immediate @ %lu\n", timer_get_counter_value());
    log_print_string("\n\n");
}
static void _task_10sec(void* _) { 
	log_print_string("task 10 sec @ %lu\n", timer_get_counter_value());
    log_print_string("\n\n");
}
static void _task_300sec(void* _) { 
	log_print_string("task 300 sec @ %lu\n", timer_get_counter_value()); 
    log_print_string("\n\n");
}

void bootstrap()
{
    sched_register_task(&_task_immediate);
    sched_register_task(&_task_10sec);
    sched_register_task(&_task_300sec);

	timer_post_task_delay(&_task_10sec, 10 * TIMER_TICKS_PER_SEC);
	timer_post_task_delay(&_task_300sec, 300 * TIMER_TICKS_PER_SEC);
	timer_post_task_delay(&_task_immediate, 0);
        log_print_string("start!");
}
