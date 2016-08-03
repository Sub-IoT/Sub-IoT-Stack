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

/*! \file timer.h
 * \defgroup frameworktimer Framework Timer
 * \ingroup framework
 * @{
 * \brief The framework timer provides an abstraction of the low-level hardware timers provided by the HAL. 
 * 		The framework timer interface builds on the low-level HAL timer interface to provide more advanced capabilities. 
 * 
 * The major differences with the HAL timers are:
 *  - 32-bit counter instead of a 16-bit counter
 *  - Multiple timer events can be scheduled simultaneously
 *  - Events are executed by the scheduler in the main task loop and NOT during the timer interrupt
 *  - Support for multiple priorities
 *
 * The framework timer supports the same timer resolutions supported by the hal. By default
 * a binary millisecond timer interval is used (HWTIMER_FREQ_MS), but this can be changed
 * by setting the FRAMEWORK_TIMER_RESOLUTION property in CMake.
 *
 * The framework timers have two modes of operation 'Reset mode' and 'Normal mode'. 
 * In 'Normal mode' the 32-bit counter of the timer (see timer_get_counter()) counts from 0 to MAX_INT
 * and only then loops back to zero. Depending on the selected frequency this yield a loop time between 
 * 1,5 days (32KHz timer) and 48 days (1MS ticks). In 'Normal mode' timer_get_counter_value() therefore always 
 * returns the time since system bootup (or since the last overflow)
 *
 * When FRAMEWORK_TIMER_RESET_COUNTER is defined the timers operate in 'Reset Mode'.
 * In 'Reset Mode' the timer counter is reset every time:
 *  - A new event is scheduled
 *  - A timer event fires.
 * The meta-data of the timers is updated accordingly to ensure that the timing
 * of the events is not affected by the operation. In 'Reset mode' timer_get_counter_value() returns the 
 * number of ticks since the last time since the counter was reset. 
 * 
 * \author maarten.weyn@uantwerpen.be
 * \author daniel.vandenakker@uantwerpen.be
 *
 */
#ifndef TIMER_H_
#define TIMER_H_

#include "link_c.h"
#include "types.h"
#include "scheduler.h"
#include "hwtimer.h"
#include "framework_defs.h"

typedef uint32_t timer_tick_t;

typedef struct
{
    task_t f;
    timer_tick_t next_event;
    uint8_t priority;
} timer_event;

//a bit of dirty macro evaluation to prepend HWTIMER_FREQ_ to the value of 'FRAMEWORK_TIMER_RESOLUTION'
#define ___CONCAT2(a,b) a ## b
#define ___CONCAT(a, b) ___CONCAT2(a,b)
/*! \brief The frequency of the hardware timer used to power the framework timer
 *
 * This value can be configured by setting the 'FRAMEWORK_TIMER_RESOLUTION' CMake property
 */
#define TIMER_RESOLUTION ___CONCAT(HWTIMER_FREQ_, FRAMEWORK_TIMER_RESOLUTION)

/*! \brief The number of ticks the framework timer generates every second
 *
 */
#define TIMER_TICKS_PER_SEC ___CONCAT(HWTIMER_TICKS_, FRAMEWORK_TIMER_RESOLUTION)



/*! \brief Initialise the timer sub system
 *
 */
__LINK_C void timer_init();

/*! \brief Retrieve the current counter value of the timer
 *
 * When the timers are operating in 'Normal mode', the returned counter value
 * is the number of clock ticks since the device booted or since the last overflow.
 * (As discussed above an overflow takes between 1,5 days and 48 days to occur depending on the 
 * frequence of the timer)
 *
 * When the timers are operating in 'Reset mode', the returned counter value is the number of 
 * clock ticks since the last time a task was posted OR the last time a posted task was scheduled.
 *
 * \return timer_tick_t	The current value of the counter. Please note that
 *
 */
__LINK_C timer_tick_t timer_get_counter_value();

/*! \brief Post a task to be scheduled at a given time with a given priority
 *
 * The time parameter denotes the clock tick at which the task is to be scheduled
 * with the task scheduler. time may be a value that is in the future or in the past wrt.
 * the current value of 'timer_get_counter_value()' (denoted as cur_time).
 * If time denotes a clock tick in the future, the framework timer delays scheduling the task until
 * cur_time == time. If time denotes a clock tick in the past, the task is scheduled
 * immediately. Depending on the frequency of the timer, the priority of the task and the amount of tasks
 * currently scheduled, another few clock ticks may pass before the task is actually executed.
 * (If you need more precise timing, use a hardware timer)
 *
 * To ensure proper operation of the framework timer in the event of an overflow, time is
 * interpreted in a circular fashion wrt. the current counter value. A specified time is deemed
 * to be in the past when, using SIGNED integer arithmetic: 'time - cur_time < 0'.
 * This equates to checking whether time < cur_time, except that it also works when the timer is about
 * to overflow.
 *
 * When the framework timer is operating in 'Reset mode' (as opposed to 'Normal mode') the value of the counter is
 * reset to '0' right before the task is posted. The supplied time value is then treated in the normal manner,
 * but since the counter itself is reset, time can also be interpreted as the number of clock ticks to wait
 * (from posting the task) before scheduling the task with the task scheduler.
 *
 * Please note that posting a task with the framework timers does NOT automatically register
 * it with the scheduler. If the posted task is not registered with the scheduler, the task
 * will not be executed.
 *
 * \param task		The task to be scheduled at the given time.
 * \param time		The time at which to schedule the task for execution.
 * \param priority	The priority with which the task should be executed
 *
 * \returns error_t	SUCCESS if the task was posted successfully
 *					ENOMEM if the task could not be posted there are already too
 *						   many tasks waiting for execution.
 * 					EALREADY if the task was already scheduled.
 *					EINVAL if an invalid priority was specified.
 *
 */
__LINK_C error_t timer_post_task_prio(task_t task, timer_tick_t time, uint8_t priority);

/*! \brief Post a task <task> to be scheduled at a given <time> with the default priority.
 *
 * This function is equivalent to
 * \code{.c}
 * 	timer_post_task_prio(task,time,DEFAULT_PRIORITY);
 * \endcode
 *
 * See the comments above 'timer_post_task_prio()' for a more detailed explanation.
 *
 * \param task		The task to be scheduled at the given time.
 *
 * \param time		The time at which to schedule the task.
 *
 * \returns error_t	SUCCESS if the task was posted successfully
 *					ENOMEM if the task could not be posted there are already too
 *						   many tasks waiting for execution.
 * 					EALREADY if the task was already scheduled.
 */
static inline error_t timer_post_task(task_t task, timer_tick_t time) { return timer_post_task_prio(task,time,DEFAULT_PRIORITY);}


/*! \brief Post a task <task> to be scheduled with a certain <delay> with a given <priority>
 *
 * This function behaves in much the same way as timer_post_task_prio, except that instead of specifying the
 * time at which a task should be scheduled, the task is always scheduled <delay> ticks into the future,
 * regardless of the 'operation mode' of the framework timer.
 *
 * <delay> MUST be a value between 0 and 2^31. If <delay> is outside this range, the behavior
 * of the framework timer is undefined.
 *
 * See the comments above 'timer_post_task_prio()' for a more detailed explanation on the operation of the timers.
 *
 * \param task		The task to be executed.
 * \param delay		The delay with which the task is to be executed.
 * \param priority	The priority with which the task should be executed
 *
 * \returns error_t	SUCCESS if the task was posted successfully
 *					ENOMEM if the task could not be posted there are already too
 *						   many tasks waiting for execution.
 *					EINVAL if an invalid priority was specified.
 *					EALREADY if the task was already scheduled.
 */
static inline error_t timer_post_task_prio_delay(task_t task, timer_tick_t delay, uint8_t priority)
{ 
#ifdef FRAMEWORK_TIMER_RESET_COUNTER
    return timer_post_task_prio(task,delay,priority);
#else
    return timer_post_task_prio(task,timer_get_counter_value()+delay, priority);
#endif //FRAMEWORK_TIMER_RESET_COUNTER

}
/*! \brief Post a task to be scheduled with a certain delay> with the default priority.
 *
 * This function is equivalent to
 * \code{.c}
 * 	timer_post_task_prio_delay(task,time,DEFAULT_PRIORITY);
 * \endcode
 *
 * \param task		The task to be executed.
 * \param delay		The delay with which the task is to be executed.
 *
 * \returns error_t	SUCCESS if the task was posted successfully
 *					ENOMEM if the task could not be posted there are already too
 *						   many tasks waiting for execution.
 * 					EALREADY if the task was already scheduled.
 */
static inline error_t timer_post_task_delay(task_t task, timer_tick_t time) { return timer_post_task_prio_delay(task,time,DEFAULT_PRIORITY);}

/*! \brief Schedule a given <timer_event>
 *
 * This function is equivalent to
 * \code{.c}
 *		timer_post_task_prio(event->f, event->next_event, event->priority);
 * \endcode
 * 
 * \param event		The event to schedule
 * \returns error_t	SUCCESS if the task was posted successfully
 *					ENOMEM if the task could not be posted there are already too
 *						   many tasks waiting for execution.
 *					EINVAL if an invalid priority was specified.
 *					EALREADY if the task was already scheduled.
 */
static inline error_t timer_add_event( timer_event* event) { return timer_post_task_prio(event->f, event->next_event, event->priority);}


/*! \brief Cancel a previously scheduled task
 *
 * \param task	The task to cancel.
 * 
 * \return error_t	SUCCESS if the task was successfully canceled
 * 					EALREADY if the task was not scheduled and therefore not canceled
 *
 */
__LINK_C error_t timer_cancel_task(task_t task);

/*! \brief check if a task is already scheduled with a delay
 *
 * \param task	The task to verify.
 *
 * \return bool	true if the task is present in the timer event queue
 * 				false if the task was not scheduled
 *
 */
__LINK_C bool timer_is_task_scheduled(task_t task);

#endif /* TIMER_H_ */

/** @}*/
