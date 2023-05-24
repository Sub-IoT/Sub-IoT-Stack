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

/*
 * scheduler.c
 *
 *  Created on: 22 Jan 2015
 *      Author: Daniel van den Akker
 */


#include "scheduler.h"

//needs to be decoupled
#include "callstack.h"
#include "debug.h"
#include "log.h"
#include <string.h>
#include <stdio.h>
#include "hwatomic.h"
#include "ng.h"
#include "hwsystem.h"
#include "error_event_file.h"
#include "errors.h"
#include "timer.h"
#include "hwwatchdog.h"
#include "power_tracking_file.h"

#include "framework_defs.h"
#define SCHEDULER_MAX_TASKS FRAMEWORK_SCHEDULER_MAX_TASKS

_Static_assert(SCHEDULER_MAX_TASKS < UINT8_MAX,
               "SCHEDULER_MAX_TASKS can not be set larger than 254");

#ifdef NODE_GLOBALS
    #warning NODE_GLOBALS is defined when using the default scheduler. Are you sure this is what you want ?
#endif

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_SCHED_LOG_ENABLED)
  #define DPRINT(...) log_print_string( __VA_ARGS__)
#else
  #define DPRINT(...)
#endif

enum
{
	NUM_PRIORITIES = MIN_PRIORITY+1,
	NUM_TASKS = SCHEDULER_MAX_TASKS,
	NOT_SCHEDULED = NUM_PRIORITIES,
	NO_TASK = SCHEDULER_MAX_TASKS,
};

typedef struct
{
	task_t task;
	void *arg;
	uint8_t next;
	uint8_t prev;
	uint8_t priority;

} task_info_t;


typedef struct
{
	task_t task;
	uint8_t index;
} taskindex_info_t;

taskindex_info_t NGDEF(m_index)[NUM_TASKS];
task_info_t NGDEF(m_info)[NUM_TASKS];

uint8_t NGDEF(m_head)[NUM_PRIORITIES];
uint8_t NGDEF(m_tail)[NUM_PRIORITIES];
uint8_t current_task_id;
volatile uint8_t NGDEF(current_priority);
uint8_t NGDEF(num_registered_tasks);
static bool scheduler_active = false;
#if defined FRAMEWORK_USE_WATCHDOG
#define WATCHDOG_WARNING_TIMEOUT TIMER_TICKS_PER_SEC * 17
bool watchdog_wakeup;
static timer_tick_t last_task_start_time = 0;
#endif

volatile bool task_scheduled_after_sched_loop = false;

#ifdef SCHEDULER_DEBUG
void check_structs_are_valid()
{
	start_atomic();
	assert(NG(num_registered_tasks) <= NUM_TASKS);
	bool visited[NUM_TASKS];
	memset(visited, false, NUM_TASKS);
	for(int i = 0; i < NG(num_registered_tasks); i++)
	{
		assert(NG(m_index)[i].task != 0x0);
		assert(NG(m_index)[i].index != NO_TASK);
		assert(NG(m_index)[i].index < NUM_TASKS);
		assert(NG(m_info)[NG(m_index)[i].index].task == NG(m_index)[i].task);
		assert(!visited[NG(m_index)[i].index]);
		visited[NG(m_index)[i].index] = true;
	}
	for(int i = NG(num_registered_tasks); i < NUM_TASKS; i++)
	{
		assert(NG(m_index)[i].task == 0x0);
		assert(NG(m_index)[i].index == NO_TASK);
	}
	for(int i = 0; i < NUM_TASKS; i++)
		assert(visited[i] || NG(m_info)[i].task == 0x0);


	memset(visited, false, NUM_TASKS);
	for(int prio = 0; prio < NUM_PRIORITIES;prio++)
	{
		uint8_t prev_ind=NO_TASK;
		for(uint8_t cur_ind = NG(m_head)[prio]; cur_ind != NO_TASK; cur_ind = NG(m_info)[cur_ind].next)
		{
			assert(cur_ind < NUM_TASKS);
			assert(!visited[cur_ind]);
			visited[cur_ind] = true;
			assert(NG(m_info)[cur_ind].prev == prev_ind);

			if(prev_ind != NO_TASK)
				assert(NG(m_info)[prev_ind].next == cur_ind);
			else
				assert(NG(m_head)[prio] == cur_ind);

			assert(NG(m_info)[cur_ind].task != 0x0);
			assert(NG(m_info)[cur_ind].priority == prio);
			prev_ind=cur_ind;
		}
		assert(NG(m_tail)[prio] == prev_ind);
	}
	for(int i = 0; i < NUM_TASKS; i++)
	{
		assert((visited[i]) || NG(m_info)[i].priority == NOT_SCHEDULED);
	}

	assert(NG(current_priority) <= NUM_PRIORITIES);
	for(int i = 0; i < NG(current_priority); i++)
		assert(NG(m_head)[i] == NO_TASK);
	//INT_Enable();
	end_atomic();
}
#else
static inline void check_structs_are_valid(){}
#endif

#if defined FRAMEWORK_USE_WATCHDOG
static void __feed_watchdog_task(void *arg) { watchdog_wakeup = true; }
#endif

__LINK_C void scheduler_init()
{
	for(unsigned int i = 0; i < NUM_TASKS; i++)
	{
		NG(m_info)[i].next = NO_TASK;
		NG(m_info)[i].prev = NO_TASK;
		NG(m_info)[i].task = 0x0;
		NG(m_info)[i].priority = NOT_SCHEDULED;

		NG(m_index)[i].index = NO_TASK;
		NG(m_index)[i].task = 0x0;
	}
	memset(NG(m_head), NO_TASK, sizeof(NG(m_head)));
	memset(NG(m_tail), NO_TASK, sizeof(NG(m_tail)));
	NG(current_priority) = NUM_PRIORITIES;
	NG(num_registered_tasks) = 0;
	check_structs_are_valid();
#if defined FRAMEWORK_USE_WATCHDOG
	__watchdog_init();
	sched_register_task(&__feed_watchdog_task);
	timer_post_task_prio_delay(&__feed_watchdog_task, hw_watchdog_get_timeout() * TIMER_TICKS_PER_SEC, MAX_PRIORITY);
#endif
}

__LINK_C uint8_t get_task_id(task_t task)
{
	check_structs_are_valid();
	assert(NG(num_registered_tasks) <= NUM_TASKS);
	int begin = 0;
	int end = NG(num_registered_tasks)-1;
	uint32_t i = 0;

	while (begin <= end)
	{
		int pivot = begin + ((end - begin) / 2);
		if (((void*)NG(m_index)[pivot].task) < ((void*)task))
			begin= pivot+1;
		else if (((void*)task) < ((void*)NG(m_index)[pivot].task))
			end = pivot-1;
		else
		{
			while((pivot > 1) && (NG(m_index)[pivot-1].task == task))
			{
				pivot--;
			}
			return pivot;
		}
		i++;
		assert(i <= NG(num_registered_tasks));
	}
	return NO_TASK;
}

__LINK_C error_t sched_register_task_allow_multiple(task_t task, bool allow)
{
  assert(NG(num_registered_tasks) < NUM_TASKS);
  if(!allow && (get_task_id(task) != NO_TASK))
    return -EALREADY;

	error_t retVal;
	check_structs_are_valid();
	//INT_Disable();
	start_atomic();

    for(int i = NG(num_registered_tasks); i >= 0; i--)
    {
        if (i == 0 || ((void*)NG(m_index)[i-1].task) < ((void*)task))
        {
            NG(m_index)[i].task = task;
            NG(m_index)[i].index = NG(num_registered_tasks);
            NG(m_info)[NG(m_index)[i].index].task = task;
            NG(m_info)[NG(m_index)[i].index].arg = (void*)UINTPTR_MAX;
            break;
        }
        else
        {
            NG(m_index)[i] = NG(m_index)[i-1];
        }
    }
    NG(num_registered_tasks)++;
    retVal = SUCCESS;

	//INT_Enable();
	end_atomic();
	check_structs_are_valid();
	return retVal;
}

static inline bool is_next_task_the_same(uint8_t id)
{
	return ((id + 1) < NG(num_registered_tasks)) && (NG(m_index)[id].task == NG(m_index)[id + 1].task);
}

static inline bool is_scheduled(uint8_t id)
{
	assert(id < NUM_TASKS);
	check_structs_are_valid();
	return NG(m_info)[NG(m_index)[id].index].priority != NOT_SCHEDULED;
}

static bool is_scheduled_with_arg(uint8_t id, void *arg)
{
	assert(id < NUM_TASKS);
	check_structs_are_valid();
	while(true)
	{
		uint8_t index = NG(m_index)[id].index;
		if(arg == NG(m_info)[index].arg)
		{
			return NG(m_info)[index].priority != NOT_SCHEDULED;
		}
		if(is_next_task_the_same(id))
		{
			id += 1;
		}
		else
		{
			break;
		}
	}
	return false;
}

__LINK_C bool sched_is_scheduled_with_arg(task_t task, void *arg)
{
	//INT_Disable();
	start_atomic();
	uint8_t task_id = get_task_id(task);
	bool retVal = false;
	if(task_id != NO_TASK)
	{
		if(is_next_task_the_same(task_id))
		{
			retVal = is_scheduled_with_arg(task_id, arg);
		}
		else
		{
			retVal = is_scheduled(task_id);
		}
	}
	//INT_Enable();
	end_atomic();
	return retVal;
}

static error_t do_initial_task_checks(uint8_t task_id, void *arg, bool cancel)
{
	if(task_id == NO_TASK)
	{
		return -EINVAL;
	}
	if(is_next_task_the_same(task_id))
	{
		if(!cancel == is_scheduled_with_arg(task_id, arg))
		{
			return -EALREADY;
		}
	}
	else 
	{
		if ((!cancel) == is_scheduled(task_id))
		{
			return -EALREADY;
		}
	}
	return SUCCESS;
}

__LINK_C error_t sched_post_task_prio(task_t task, uint8_t priority, void *arg)
{
	error_t retVal;
	start_atomic();
	check_structs_are_valid();
	uint8_t task_id = get_task_id(task);
	retVal = do_initial_task_checks(task_id, arg, false);
	if(priority > MIN_PRIORITY)
	{
		retVal = -ESIZE;
	}
	else if(retVal == SUCCESS)
	{
		if(is_next_task_the_same(task_id))
		{
			while (true)
			{
				if(NG(m_info)[NG(m_index)[task_id].index].priority == NOT_SCHEDULED)
				{
					break;
				}
				if(is_next_task_the_same(task_id))
				{
					task_id += 1;
				}
				else
				{
					retVal = -ENOMEM;
					break;
				}
			}
		}
		uint8_t index = NG(m_index)[task_id].index;
		if(retVal == SUCCESS)
		{
			if(NG(m_head)[priority] == NO_TASK)
			{
				NG(m_head)[priority] = index;
				NG(m_tail)[priority] = index;
			}
			else
			{
				NG(m_info)[NG(m_tail)[priority]].next = index;
				NG(m_info)[index].prev = NG(m_tail)[priority];
				NG(m_tail)[priority] = index;
			}
			NG(m_info)[index].priority = priority;
			NG(m_info)[index].arg = arg;
			//if our priority is higher than the currently known maximum priority
			if((priority < NG(current_priority)))
				NG(current_priority) = priority;
			check_structs_are_valid();
		}
	}
	end_atomic();
	check_structs_are_valid();
	task_scheduled_after_sched_loop = true;
	return retVal;
}

__LINK_C error_t sched_cancel_task_with_arg(task_t task, void *arg)
{
	check_structs_are_valid();
	error_t retVal = SUCCESS;

	start_atomic();
	uint8_t id = get_task_id(task);
	retVal = do_initial_task_checks(id, arg, true);
	if(retVal == SUCCESS)
	{
		if(is_next_task_the_same(id))
		{
			while (true)
			{
				if(NG(m_info)[NG(m_index)[id].index].arg == arg)
				{
					break;
				}
				if(is_next_task_the_same(id))
				{
					id += 1;
				}
				else
				{
					// This shouldn't happen
					retVal = -EALREADY;
					break;
				}
			}
		}
		if(retVal == SUCCESS)
		{
			uint8_t index = NG(m_index)[id].index;
			if (NG(m_info)[index].prev == NO_TASK)
				NG(m_head)[NG(m_info)[index].priority] = NG(m_info)[index].next;
			else
				NG(m_info)[NG(m_info)[index].prev].next = NG(m_info)[index].next;

			if (NG(m_info)[index].next == NO_TASK)
				NG(m_tail)[NG(m_info)[index].priority] = NG(m_info)[index].prev;
			else
				NG(m_info)[NG(m_info)[index].next].prev = NG(m_info)[index].prev;

			NG(m_info)[index].prev = NO_TASK;
			NG(m_info)[index].next = NO_TASK;
			NG(m_info)[index].priority = NOT_SCHEDULED;
			check_structs_are_valid();
		}
	}
	end_atomic();
	return retVal;
}

static uint8_t pop_task(int priority)
{
	uint8_t id = NO_TASK;
	check_structs_are_valid();
	start_atomic();
	if (NG(m_head)[priority] != NO_TASK)
	{
		id = NG(m_head)[priority];
		NG(m_head)[priority] = NG(m_info)[NG(m_head)[priority]].next;
		if(NG(m_head)[priority] == NO_TASK)
			NG(m_tail)[priority] = NO_TASK;
		else
			NG(m_info)[NG(m_head)[priority]].prev = NO_TASK;

		NG(m_info)[id].next = NO_TASK;
		NG(m_info)[id].prev = NO_TASK;
		NG(m_info)[id].priority = NOT_SCHEDULED;
	}
	end_atomic();
	check_structs_are_valid();
	return id;
}

static inline bool tasks_waiting(int priority)
{
	return NG(m_head)[priority] != NO_TASK;
}

static uint8_t low_power_mode = FRAMEWORK_SCHEDULER_LP_MODE;

uint8_t sched_get_low_power_mode(void) {
  return low_power_mode;
}

void sched_set_low_power_mode(uint8_t mode) {
  low_power_mode = mode;
}

// This is in interrupt context
__LINK_C timer_tick_t sched_check_software_watchdog(task_t task, timer_tick_t current_time) {
#ifdef FRAMEWORK_USE_WATCHDOG
	if(task == &__feed_watchdog_task) {
		if(scheduler_active) {
			timer_tick_t difference = timer_calculate_difference(last_task_start_time, current_time);
			if(difference > WATCHDOG_WARNING_TIMEOUT) {
				// PANIC
#if defined(FRAMEWORK_USE_CALLSTACK) && defined(FRAMEWORK_USE_ERROR_EVENT_FILE)
				error_event_create_watchdog_event();
#endif
				return hw_watchdog_get_timeout() * TIMER_TICKS_PER_SEC;
			}
			return (hw_watchdog_get_timeout() * TIMER_TICKS_PER_SEC - difference);
		} else {
			return hw_watchdog_get_timeout() * TIMER_TICKS_PER_SEC;
		}
	}
#endif
	return 0;
}

__LINK_C task_t sched_get_current_task(void)
{
	if(scheduler_active)
	{
		return NG(m_info)[current_task_id].task;
	}
	else
	{
		return 0;
	}
}

__LINK_C void scheduler_run()
{
	while(1)
	{
#if defined FRAMEWORK_USE_WATCHDOG
		bool task_list_empty = true;
		uint8_t executed_tasks = 0;
		watchdog_wakeup = false;
#endif
#if (defined FRAMEWORK_USE_POWER_TRACKING || FRAMEWORK_SCHEDULER_MAX_ACTIVE_TIME > 0)
		timer_tick_t wakeup_time = timer_get_counter_value();
#endif
		while(NG(current_priority) < NUM_PRIORITIES)
		{
			check_structs_are_valid();
			for(uint8_t id = pop_task((NG(current_priority))); id != NO_TASK; id = pop_task(NG(current_priority)))
			{
				scheduler_active = true;
#if defined FRAMEWORK_USE_WATCHDOG
				executed_tasks++;
				task_list_empty = false;
				hw_watchdog_feed();
				last_task_start_time = timer_get_counter_value();
#endif
				check_structs_are_valid();
#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_SCHED_LOG_ENABLED)
				timer_tick_t start = timer_get_counter_value();
				log_print_string("SCHED start %p at %i", NG(m_info)[id].task, start);
#endif
				current_task_id = id;
				NG(m_info)[id].task(NG(m_info)[id].arg);
		#if FRAMEWORK_SCHEDULER_MAX_ACTIVE_TIME > 0
				assert(timer_get_current_time_difference(wakeup_time) < FRAMEWORK_SCHEDULER_MAX_ACTIVE_TIME * TIMER_TICKS_PER_SEC);
		#endif
#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_SCHED_LOG_ENABLED)
				timer_tick_t stop = timer_get_counter_value();
				timer_tick_t duration = stop - start;
				log_print_string("SCHED stop %p at %i took %i", NG(m_info)[id].task, stop, duration);
#endif
			}
			//this needs to be done atomically since otherwise we risk decrementing the current priority
			//while a higher priority task is waiting in the queue
			start_atomic();
			if (!tasks_waiting(NG(current_priority)))
				NG(current_priority)++;
#ifndef NDEBUG
			for(int i = 0; i < NG(current_priority); i++)
				assert(!tasks_waiting(i));
#endif
			task_scheduled_after_sched_loop = false;
			end_atomic();	
		}		
		scheduler_active = false;
#if defined FRAMEWORK_USE_WATCHDOG
		hw_watchdog_feed();
#if defined FRAMEWORK_USE_POWER_TRACKING
		//we don't want to register wake-ups that only trigger the watchdog
		//we also need to check that the watchdog task was the only task that was executed as there is a small chance that
		//the watchdog task is triggered when also other tasks are executing. In that case we want to track the time as active.
		if(!(task_list_empty || (watchdog_wakeup && executed_tasks == 1)))
		{
#endif
#endif
#if defined FRAMEWORK_USE_POWER_TRACKING
			power_tracking_register_run_time(timer_get_current_time_difference(wakeup_time));
#endif			
#if defined FRAMEWORK_USE_WATCHDOG && defined FRAMEWORK_USE_POWER_TRACKING
		}
#endif

		//during some oss7-testsuite cases we can see a scheduling of the flushing of the fifos for the UART in between the end of the scheduler 
		//priority loop, and the call to enter low power mode. This caused the test to fail as the response was received by the testsuite only 
		//after the watchdog woke up the device. So, task_scheduled_after_sched_loop is used to ensure the tasklist is really empty.
		start_atomic();
		if(!task_scheduled_after_sched_loop) {
			hw_enter_lowpower_mode(low_power_mode);
		}
		end_atomic();
	}
}
