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

/*! \file dummy_timer.c
 *
 *  \author Liam Wickins <liamw9534@gmail.com>
 *
 */

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#include "hwtimer.h"
#include "log.h"
#include "platform_defs.h"

#if HAL_PERIPH_LOG_ENABLED

#define DPRINT(...)   log_print_stack_string(LOG_STACK_TIMER, __VA_ARGS__)

#else

#define DPRINT(...)

#endif

#define NS_PER_SEC    1000000000UL
#define MSEC          (NS_PER_SEC/1000)

// Can be set through the make system (default is 4)
#define NUM_TIMERS    PLATFORM_NUM_TIMERS

// Timer state is stored on a per timer basis with a maximum of NUM_TIMERS
static bool timers_init = false;
static volatile bool timer_schedule_active[NUM_TIMERS];
static volatile hwtimer_tick_t timer_schedule_tick[NUM_TIMERS];
static void (*timer_fired[NUM_TIMERS])(void);
static void (*timer_overflow[NUM_TIMERS])(void);
static volatile hwtimer_tick_t tick = 0;   // This is the global tick counter
static char stack_top[8192];

// High resolution timer
static uint64_t time_now()
{
	uint64_t val;
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	val = (tp.tv_sec * NS_PER_SEC) + tp.tv_nsec;
	//printf("\nval = %lu\n", val);
	return val;
}


// The timer handler is pretty simple.  It just wakes up once every tick and updates its tick counter,
// notifying any elapsed scheduled timers or overflows as it goes.

static int timer_handler(void *arg)
{
	static uint64_t last_t;

	last_t = time_now();  // Start-up condition so we know initial high-res time value

	while (1)
	{
		uint64_t t = time_now();

		unsigned int msec = ((t - last_t) / MSEC);

		//DPRINT("tick = %u - %u:%u", msec, timer_schedule_active[0], timer_schedule_tick[0]);

		// In some cases we may have elapsed more than 1 ms since the last call so we iterate the
		// number of milliseconds elapsed and call the scheduler for each tick that has elapsed

		for (unsigned int k = 0; k < msec; k++)
		{
			tick++;

			for (int i = 0; i < NUM_TIMERS; i++)
			{
				if (timer_schedule_active[i])
				{
					//DPRINT("checking %u : %u <> %u", i, timer_schedule_tick[i], tick);
					if (tick == timer_schedule_tick[i])
					{
						//DPRINT("!!!!!!!!!!!! FIRED !!!!!!!!!!!!!!");
						//timer_schedule_active[i] = false;
						timer_fired[i]();
					}
				}

				if (tick == 0 && timer_overflow[i])
				{
					//DPRINT("!!!!!!!!!!!! OVERFLOW !!!!!!!!!!!!!!");
					timer_overflow[i]();
				}
			}
			last_t = t;
		}

		usleep(1000);   // Assumes 1 ms timer tick but it is not guaranteed to wake-up after 1 ms
	}
}


__LINK_C hwtimer_tick_t hw_timer_getvalue(hwtimer_id_t timer_id)
{
	//DPRINT("hw_timer_getvalue(%u)=%u", timer_id, tick);
	return tick;  // Return global tick value
}

__LINK_C const hwtimer_info_t* hw_timer_get_info(hwtimer_id_t timer_id)
{
    static const hwtimer_info_t timer_info = {
      .min_delay_ticks = 0,
    };
    return &timer_info;
}

__LINK_C error_t hw_timer_schedule(hwtimer_id_t timer_id, hwtimer_tick_t t )
{
	DPRINT("hw_timer_schedule(%u, %u) @ %u", timer_id, tick, hw_timer_getvalue(timer_id));
	timer_schedule_tick[timer_id] = t;
	timer_schedule_active[timer_id] = true;
}

__LINK_C error_t hw_timer_init(hwtimer_id_t timer_id, uint8_t frequency, timer_callback_t compare_callback, timer_callback_t overflow_callback)
{
	DPRINT("hw_timer_init(%u, %u)", timer_id, frequency);

	timer_fired[timer_id] = compare_callback;
	timer_overflow[timer_id] = overflow_callback;

	// We do a one-time init of the async timer task first time this function is called
	if (!timers_init)
	{
		// This cloned process will handle all timers
		timers_init = true;
		clone(timer_handler, &stack_top[8192], CLONE_VM, NULL);
	}

	return 0;
}

__LINK_C bool hw_timer_is_overflow_pending(hwtimer_id_t id)
{
	return false; // We don't have interrupt pending in this emulation
}

__LINK_C error_t hw_timer_cancel(hwtimer_id_t timer_id)
{
	DPRINT("hw_timer_cancel(%u)", timer_id);
	timer_schedule_active[timer_id] = false;  // Just set the timer active flag
}
