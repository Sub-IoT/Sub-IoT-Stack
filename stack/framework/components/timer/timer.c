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

#include "timer.h"
#include "ng.h"
#include "hwtimer.h"
#include "hwatomic.h"
#include "debug.h"
#include "framework_defs.h"
#include "log.h"
#include "errors.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_TIMER_LOG_ENABLED)
  #define DPRINT(...) log_print_stack_string(LOG_STACK_FWK, __VA_ARGS__)
#else
  #define DPRINT(...)
#endif


#ifdef NODE_GLOBALS
    #warning Default Timer implementation used when NODE_GLOBALS is active. Are you sure this is what you want ??
#endif

#define HW_TIMER_ID 0

#define COUNTER_EVENTTIME_LIMIT TIMER_TICKS_PER_SEC * 20
#define COUNTER_OVERFLOW_INCREASE (UINT32_C(1) << (8*sizeof(hwtimer_tick_t)))

// define inline functions from timer.h as extern
extern inline error_t timer_post_task(task_t task, timer_tick_t time);
extern inline error_t timer_post_task_prio_delay(task_t task, timer_tick_t delay, uint8_t priority);
extern inline error_t timer_post_task_delay(task_t task, timer_tick_t delay);
extern inline error_t timer_add_event(timer_event* event);

static timer_event NGDEF(timers)[FRAMEWORK_TIMER_STACK_SIZE];
static volatile timer_tick_t NGDEF(next_event);
static volatile bool NGDEF(hw_event_scheduled);
static volatile timer_tick_t NGDEF(timer_offset);
static const hwtimer_info_t* timer_info;
static bool timer_busy_programming = false;
static bool fired_by_interrupt = true;
enum
{
    NO_EVENT = FRAMEWORK_TIMER_STACK_SIZE,
};

static void timer_overflow();
static void timer_fired();

__LINK_C void timer_init()
{
    for(uint32_t i = 0; i < FRAMEWORK_TIMER_STACK_SIZE; i++)
	NG(timers)[i].f = 0x0;

    NG(next_event) = NO_EVENT;
    NG(timer_offset) = 0;
    NG(hw_event_scheduled) = false;

    error_t err = hw_timer_init(HW_TIMER_ID, TIMER_RESOLUTION, &timer_fired, &timer_overflow);
    assert(err == SUCCESS);

    timer_info = hw_timer_get_info(HW_TIMER_ID);
}

error_t timer_init_event(timer_event* event, task_t callback)
{
    event->f = callback;
    event->arg = NULL;
    event->priority = MAX_PRIORITY;
    event->period = 0;
    return (sched_register_task(callback)); // register the function callback to be called at the end of the timeout
}

static bool configure_next_event();
__LINK_C error_t timer_post_task_prio(task_t task, timer_tick_t fire_time, uint8_t priority, timer_tick_t period, void *arg)
{
    error_t status = ENOMEM;
    if (priority > MIN_PRIORITY)
        return EINVAL;

    DPRINT("fire_time  <%lu>" , fire_time);

    if (fire_time == 0)
    {
        DPRINT("No delay, so the timer event callback is scheduled immediately");
        return (sched_post_task_prio(task, priority, arg));
    }

    bool conf_atomic_ended = false;
    start_atomic();
    uint32_t empty_index = FRAMEWORK_TIMER_STACK_SIZE;
    for (uint32_t i = 0; i < FRAMEWORK_TIMER_STACK_SIZE; i++)
    {
        if (NG(timers)[i].f == 0x0 && empty_index == FRAMEWORK_TIMER_STACK_SIZE)
        {
            empty_index = i;
        }
        else if (NG(timers)[i].f == task)
        {
            // it is allowed to update only the fire time
            if (NG(timers)[i].priority == priority)
            {
                NG(timers)[i].period = period;
                NG(timers)[i].next_event = fire_time;
                empty_index = i;
                goto config;
            }
            else
            //for now: do not allow an event to be scheduled more than once
            //otherwise we risk having the same task being scheduled twice and only executed once
            //because the scheduler disallows the same task to be scheduled multiple times
                status = EALREADY;
            break;
        }
    }

    if (status != EALREADY && empty_index != FRAMEWORK_TIMER_STACK_SIZE)
    {
        NG(timers)[empty_index].f = task;
        NG(timers)[empty_index].next_event = fire_time;
        NG(timers)[empty_index].priority = priority;
        NG(timers)[empty_index].arg = arg;
        NG(timers)[empty_index].period = period;
    }
    else
        goto end;

config:

    //if there is no event scheduled, this event will run before the next scheduled event
    {
        bool do_config = NG(next_event) == NO_EVENT;

        if (!do_config)
        {
            uint32_t counter = timer_get_counter_value();

            DPRINT("timer_post_task_prio counter value <%lu>" , counter);

            //if the new event should fire sooner than the old event --> trigger reconfig
            //this is done using signed ints (compared to the current counter)
            //to ensure propper handling of timer overflows
            int32_t next_fire_delay = ((int32_t)fire_time) - ((int32_t)counter);

            DPRINT("next_fire_delay <%lu>" , next_fire_delay);

            int32_t old_fire_delay = ((int32_t)NG(timers)[NG(next_event)].next_event) - ((int32_t)counter);
            do_config = (next_fire_delay <= old_fire_delay) || NG(next_event) == empty_index; //when same index is overwritten, also update
        }

        if (do_config) {
            conf_atomic_ended = configure_next_event();
        }
            

        status = SUCCESS;
    }

end:
    if(!conf_atomic_ended) { //if configure_next_event gets run, then atomic is ended in there. Otherwise we should end it here.
        end_atomic();
    }
    
    return status;
}

__LINK_C error_t timer_cancel_task(task_t task)
{
    error_t status = EALREADY;
    
    bool conf_atomic_ended = false;

    start_atomic();

    for(uint32_t i = 0; i < FRAMEWORK_TIMER_STACK_SIZE; i++)
    {
      if(NG(timers)[i].f == task)
      {
        NG(timers)[i].f = 0x0;
        //if we were the first event to fire --> trigger a reconfiguration
        if(NG(next_event) == i) {
            conf_atomic_ended = configure_next_event();
        }
          

        status = SUCCESS;
        break;
      }
    }
    if(!conf_atomic_ended) { //if configure_next_event gets run, then atomic is ended in there. Otherwise we should end it here.
        end_atomic(); 
    }
    

    return status;
}

error_t timer_add_event(timer_event* event)
{
    return timer_post_task_prio(event->f, timer_get_counter_value() + event->next_event, event->priority, event->period, event->arg);
}

void timer_cancel_event(timer_event* event)
{
    timer_cancel_task(event->f);
    sched_cancel_task(event->f);
}

__LINK_C bool timer_is_task_scheduled(task_t task)
{
    bool present = false;

    start_atomic();

     for (uint32_t i = 0; i < FRAMEWORK_TIMER_STACK_SIZE; i++)
     {
       if (NG(timers)[i].f == task)
       {
          present = true;
          break;
       }
     }
     end_atomic();

     return present;
}

__LINK_C timer_tick_t timer_get_counter_value()
{
	timer_tick_t counter;
    start_atomic();
    timer_tick_t hw_timer_value = hw_timer_getvalue(HW_TIMER_ID);
	counter = NG(timer_offset) + hw_timer_value;
	//increase the counter with COUNTER_OVERFLOW_INCREASE
	//if an overflow is pending. (This is to compensate for the 
	//fact that NG(timer_offset) is not updated until the overflow
	//interrupt is actually fired
    //the overflow can occur between reading out the timer here above
    //and checking the overflow pending bit. If this occurs, hw_timer_value
    //contains the max value before overflow and we will add COUNTER_OVERFLOW_INCREASE
    //bellow as well which mean that the returned time is off by COUNTER_OVERFLOW_INCREASE.
    //So only add COUNTER_OVERFLOW_INCREASE if hw_timer_value is not to large.
	if(hw_timer_is_overflow_pending(HW_TIMER_ID) && (hw_timer_value < (COUNTER_OVERFLOW_INCREASE - 10)))
	    counter += COUNTER_OVERFLOW_INCREASE;
    end_atomic();
    return counter;
}

static uint32_t get_next_event()
{
    //this function should only be called from an atomic context
    int32_t min_delay;
    uint32_t next_fire_event = NO_EVENT;
    uint32_t counter = timer_get_counter_value();

    for(uint32_t i = 0; i < FRAMEWORK_TIMER_STACK_SIZE; i++)
    {
    	if(NG(timers)[i].f == 0x0)
    		continue;
    	//trick borrowed from AODV: by using signed integers in this way
    	//we know that if the event has already passed delay_ticks will be < 0
    	// --> events are sorted from past -> future regardless of any (pending) overflows
    	int32_t delay_ticks = ((int32_t)NG(timers)[i].next_event) - ((int32_t)counter);
    	if(next_fire_event == NO_EVENT || delay_ticks < min_delay)
		{
    		min_delay = delay_ticks;
			next_fire_event = i;
		}
    }
    return next_fire_event;
}

static bool configure_next_event()
{
    //this function should only be called from an atomic context
	timer_tick_t next_fire_time;
    timer_tick_t current_time = timer_get_counter_value();

    timer_busy_programming = true;

    do
    {
		//find the next event that has not yet passed, and schedule
		//the 'late' events while we're at it
		NG(next_event) = get_next_event();

		if(NG(next_event) != NO_EVENT)
		{
			next_fire_time = NG(timers)[NG(next_event)].next_event;
            // fire if the fire time is less than 20 seconds ago since this means we should have fired already (timer_calculate_difference takes overflow into account)          
            if(timer_calculate_difference(next_fire_time, current_time + timer_info->min_delay_ticks) < COUNTER_EVENTTIME_LIMIT)
			{
                DPRINT("will be late, sched immediately\n\n");
                if(NG(timers)[NG(next_event)].f == 0)
                    DPRINT("function was empty, skipping");
                else {
                    fired_by_interrupt = false;
                    timer_fired();
                }
			}
		}
    }
    while(NG(next_event) != NO_EVENT && ( (((int32_t)next_fire_time) - ((int32_t)current_time)  - timer_info->min_delay_ticks) <= 0  ) );

    // if recursive event was scheduled immediately, don't set hw timer delay until last time in configure next event
    if(!fired_by_interrupt)
        return false;

    //at this point NG(next_event) is eiter equal to NO_EVENT (no tasks left)
    //or we have the next event we can schedule
    bool called_atomic = false;
    if(NG(next_event) == NO_EVENT)
    {
		//cancel the timer in case it is still running (can happen if we're called from timer_cancel_event)
		NG(hw_event_scheduled) = false;
		hw_timer_cancel(HW_TIMER_ID);
    }
    else
    {
		//calculate schedule time relative to current time rather than
		//latest overflow time, to counteract any delays in updating counter_offset
		//(eg when we're scheduling an event from an interrupt and thereby delaying
		//the updating of counter_offset)
      timer_tick_t fire_delay = (next_fire_time - current_time);
		//if the timer should fire in less ticks than supported by the HW timer --> schedule it
		//(otherwise it is scheduled from timer_overflow when needed)
		if((fire_delay + hw_timer_getvalue(HW_TIMER_ID)) < COUNTER_OVERFLOW_INCREASE)
		{
			NG(hw_event_scheduled) = true;
            end_atomic(); //stop atomic when scheduling a new timer because this needs to wait for a interrupt before writing
            called_atomic = true;
			hw_timer_schedule_delay(HW_TIMER_ID, (hwtimer_tick_t)fire_delay);
#ifndef NDEBUG	    
			//check that we didn't try to schedule a timer in the past
			//normally this shouldn't happen but it IS theoretically possible...
      fire_delay = (next_fire_time - current_time - timer_info->min_delay_ticks);
			//fire_delay should be in [0,COUNTER_OVERFLOW_INCREASE]. if this is not the case, it is because timer_get_counter() is
			//now larger than next_fire_event, which means we 'missed' the event
			assert(((int32_t)fire_delay) > 0);
#endif
		}
		else
		{
			//set hw_event_scheduled explicitly to false to allow timer_overflow
			//to schedule the event when needed
			NG(hw_event_scheduled) = false;

            // Cancel the timer, the next event should only be set after an overflow
		    hw_timer_cancel(HW_TIMER_ID);
		}
    }
    timer_busy_programming = false;
    return called_atomic;
}

timer_tick_t timer_get_current_time_difference(timer_tick_t start_time) {
    return timer_calculate_difference(start_time, timer_get_counter_value());
}

timer_tick_t timer_calculate_difference(timer_tick_t start_time, timer_tick_t stop_time)
{
    if(start_time <= stop_time)
        return stop_time - start_time;
    // if a rollover happened, add both parts together
    else
        return (UINT32_MAX - start_time) + 1 + stop_time; 
}

static void timer_overflow()
{
    NG(timer_offset) += COUNTER_OVERFLOW_INCREASE;
    if(NG(next_event) != NO_EVENT && 		//there is an event scheduled at THIS timer level
	(!NG(hw_event_scheduled)) &&		//but NOT at the hw timer level
		( (NG(timers)[NG(next_event)].next_event >= NG(timer_offset))
	    && (NG(timers)[NG(next_event)].next_event <= (NG(timer_offset) + COUNTER_OVERFLOW_INCREASE - 1) ) ) //and the next trigger will happen before the next overflow
	)
    {
		timer_tick_t fire_time = (NG(timers)[NG(next_event)].next_event - NG(timer_offset));

		//fire time already passed
		if(fire_time <= (hw_timer_getvalue(HW_TIMER_ID) + timer_info->min_delay_ticks))
			timer_fired();
		else
		{
			NG(hw_event_scheduled) = true;
			hw_timer_schedule(HW_TIMER_ID, fire_time);
			//again: this should normally not happen: (but it's possible in theory)
			//so: add an assert to make sure. If it ever triggers,
			//we'll have to insert a workaround
			assert(fire_time >= hw_timer_getvalue(HW_TIMER_ID));
		}
    }
}

static void timer_fired()
{
    if(timer_busy_programming && fired_by_interrupt)
        return;
    assert(NG(next_event) != NO_EVENT);
    assert(NG(timers)[NG(next_event)].f != 0x0);
    timer_tick_t current_time = timer_get_counter_value();
#ifdef FRAMEWORK_LOG_ENABLED
    // if event got fired to early, show error logging
    if((current_time + timer_info->min_delay_ticks) < NG(timers)[NG(next_event)].next_event)
        log_print_error_string("timer fired too early with current time %i + min delay ticks %i < next event %i: function 0x%X",
            current_time, timer_info->min_delay_ticks, NG(timers)[NG(next_event)].next_event, NG(timers)[NG(next_event)].f);
    else if(current_time > (NG(timers)[NG(next_event)].next_event + 5))
        log_print_error_string("timer fired too late with current time %i > next event %i + 5: function 0x%X",
            current_time, timer_info->min_delay_ticks, NG(timers)[NG(next_event)].next_event, NG(timers)[NG(next_event)].f);
#endif
    // check if the current task is the watchdog bump task and if we're not nearly reaching the reset
    timer_tick_t repost_time_diff = sched_check_software_watchdog(NG(timers)[NG(next_event)].f, current_time);

    sched_post_task_prio(
        NG(timers)[NG(next_event)].f, NG(timers)[NG(next_event)].priority, NG(timers)[NG(next_event)].arg);

    if(repost_time_diff)
        NG(timers)[NG(next_event)].next_event = current_time + repost_time_diff;
    else if(NG(timers)[NG(next_event)].period > 0)
        NG(timers)[NG(next_event)].next_event = current_time + NG(timers)[NG(next_event)].period;
    else
        NG(timers)[NG(next_event)].f = 0x0;

    if(fired_by_interrupt)
        configure_next_event();
    else
        fired_by_interrupt = true;
}