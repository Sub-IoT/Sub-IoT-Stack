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

#include "timer.h"
#include "ng.h"
#include "hwtimer.h"
#include "hwatomic.h"
#include "debug.h"
#include "framework_defs.h"
#include "log.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_TIMER_LOG_ENABLED)
  #define DPRINT(...) log_print_stack_string(LOG_STACK_FWK, __VA_ARGS__)
#else
  #define DPRINT(...)
#endif


#ifdef NODE_GLOBALS
    #warning Default Timer implementation used when NODE_GLOBALS is active. Are you sure this is what you want ??
#endif

#define HW_TIMER_ID 0

#define COUNTER_OVERFLOW_INCREASE (UINT32_C(1) << (8*sizeof(hwtimer_tick_t)))

static timer_event NGDEF(timers)[FRAMEWORK_TIMER_STACK_SIZE];
static volatile timer_tick_t NGDEF(next_event);
static volatile bool NGDEF(hw_event_scheduled);
static volatile timer_tick_t NGDEF(timer_offset);
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

}

#ifdef FRAMEWORK_TIMER_RESET_COUNTER

	static inline void reset_counter()
	{
		//this function should only be called from an atomic context
		NG(timer_offset) = 0;
		hw_timer_counter_reset(HW_TIMER_ID);
	}
	static bool reset_timers()
	{
		//this function should only be called from an atomic context
		if(NG(next_event) == NO_EVENT)
		{
			reset_counter();
			return true;
		}

		timer_tick_t cur_value = timer_get_counter_value();
		reset_counter();
		for(uint32_t i = 0; i < FRAMEWORK_TIMER_STACK_SIZE; i++)
		{
			if(NG(timers)[i].f != 0x0)
			{
				//prevent the value from 'underflowing'
				if(cur_value > NG(timers)[i].next_event)
					NG(timers)[i].next_event = 0;
				else
					NG(timers)[i].next_event -= cur_value;
			}
		}
		return true;
	}
#else
	static inline bool reset_timers() {return false;}
#endif //FRAMEWORK_TIMER_RESET_COUNTER

static void configure_next_event();
__LINK_C error_t timer_post_task_prio(task_t task, timer_tick_t fire_time, uint8_t priority)
{
    error_t status = ENOMEM;
    bool timers_reset = reset_timers();
    if (priority > MIN_PRIORITY)
        return EINVAL;

    DPRINT("fire_time  <%lu>" , fire_time);

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
                NG(timers)[empty_index].next_event = fire_time;
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
    }
    else
        goto end;

config:

    //if there is no event scheduled, this event will run before the next scheduled event
    //or we reset the timers: trigger a reconfiguration of the next scheduled event
    {
        bool do_config = NG(next_event) == NO_EVENT || timers_reset;

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
            do_config = next_fire_delay < old_fire_delay;
        }

        if (do_config)
            configure_next_event();

        status = SUCCESS;
    }

end:
    end_atomic();
    return status;
}

__LINK_C error_t timer_cancel_task(task_t task)
{
    error_t status = EALREADY;
    
    start_atomic();

    for(uint32_t i = 0; i < FRAMEWORK_TIMER_STACK_SIZE; i++)
    {
      if(NG(timers)[i].f == task)
      {
        NG(timers)[i].f = 0x0;
        //if we were the first event to fire --> trigger a reconfiguration
        if(NG(next_event) == i)
          configure_next_event();

        status = SUCCESS;
        break;
      }
    }
    end_atomic();

    return status;
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
	counter = NG(timer_offset) + hw_timer_getvalue(HW_TIMER_ID);
	//increase the counter with COUNTER_OVERFLOW_INCREASE
	//if an overflow is pending. (This is to compensate for the 
	//fact that NG(timer_offset) is not updated until the overflow
	//interrupt is actually fired
	if(hw_timer_is_overflow_pending(HW_TIMER_ID))
	    counter += COUNTER_OVERFLOW_INCREASE;
    end_atomic();
    return counter;
}

static uint32_t get_next_event()
{
    //this function should only be called from an atomic context
    reset_timers();
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

static void configure_next_event()
{
    //this function should only be called from an atomic context
	timer_tick_t next_fire_time;
    do
    {
		//find the next event that has not yet passed, and schedule
		//the 'late' events while we're at it
		NG(next_event) = get_next_event();

		if(NG(next_event) != NO_EVENT)
		{
			next_fire_time = NG(timers)[NG(next_event)].next_event;
			if ( (((int32_t)next_fire_time) - ((int32_t)timer_get_counter_value())) <= 0 )
			{
				sched_post_task_prio(NG(timers)[NG(next_event)].f, NG(timers)[NG(next_event)].priority);
				NG(timers)[NG(next_event)].f = 0x0;
			}
		}
    }
    while(NG(next_event) != NO_EVENT && ( (((int32_t)next_fire_time) - ((int32_t)timer_get_counter_value())) <= 0  ) );

    //at this point NG(next_event) is eiter equal to NO_EVENT (no tasks left)
    //or we have the next event we can schedule
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
    	timer_tick_t fire_delay = (next_fire_time - timer_get_counter_value());
		//if the timer should fire in less ticks than supported by the HW timer --> schedule it
		//(otherwise it is scheduled from timer_overflow when needed)
		if(fire_delay < COUNTER_OVERFLOW_INCREASE)
		{
			NG(hw_event_scheduled) = true;
			hw_timer_schedule_delay(HW_TIMER_ID, (hwtimer_tick_t)fire_delay);
#ifndef NDEBUG	    
			//check that we didn't try to schedule a timer in the past
			//normally this shouldn't happen but it IS theoretically possible...
			fire_delay = (next_fire_time - timer_get_counter_value());
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
		}
    }
}
static void timer_overflow()
{
    NG(timer_offset) += COUNTER_OVERFLOW_INCREASE;
    if(NG(next_event) != NO_EVENT && 		//there is an event scheduled at THIS timer level
	(!NG(hw_event_scheduled)) &&		//but NOT at the hw timer level
		NG(timers)[NG(next_event)].next_event <= (NG(timer_offset) + COUNTER_OVERFLOW_INCREASE) //and the next trigger will happen before the next overflow
	)
    {
		//normally this shouldn't happen. Put an assert here just to make sure
		assert(NG(timers)[NG(next_event)].next_event >= NG(timer_offset));
		timer_tick_t fire_time = (NG(timers)[NG(next_event)].next_event - NG(timer_offset));

		//fire time already passed
		if(fire_time <= hw_timer_getvalue(HW_TIMER_ID))
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
    assert(NG(next_event) != NO_EVENT);
    assert(NG(timers)[NG(next_event)].f != 0x0);
    sched_post_task_prio(NG(timers)[NG(next_event)].f, NG(timers)[NG(next_event)].priority);
    NG(timers)[NG(next_event)].f = 0x0;
    configure_next_event();
}
