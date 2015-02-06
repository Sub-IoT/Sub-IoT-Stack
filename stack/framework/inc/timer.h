/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *		daniel.vandenakker@uantwerpen.be
 *
 */

/*! \file
 *
 * The framework provides an abstraction of the low-level hardware timers provided by the HAL. The framework
 * timer interface builds on the low-level HAL timer interface to provide more advanced capabilities. The 
 * major differences with the HAL timers are:
 *  - 32-bit instead of a 16-bit counter
 *  - Multiple timer events can be scheduled simultaneously
 *  - Events are executed by the scheduler in the main task loop and NOT during the timer interrupt
 *  - Support for multiple priorities
 *
 * The framework timer supports the same timer resulutions supported by the hal. By default
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
 * THIS IS THE EXACT SAME BEHAVIOR OF THE CURRENT TIMER IMPLEMENTATION IN THE D7AOSS STACK !!
 *
 */
#ifndef TIMER_H_
#define TIMER_H_

#include "types.h"
#include "scheduler.h"

typedef struct
{
    task_t f;
    int32_t next_event;
    uint8_t priority;
} timer_event;


/*! \brief Initialise the timer sub system
 *
 */
void timer_init();

/*! \brief Post a task <task> to be scheduled at a given <time> with a given <priority>
 *
 * \param task		The task to be executed at the given time. Please note that posting a task
 *			With the framework timers does NOT automatically register them with the scheduler.
 *			If the posted task is not registered with the scheduler, the task will not be 
 *			executed.
 * \param time		The exact clocktick at which the task is to be executed. This value is interpreted
 *			differently depending on the operation mode of the timers. 
 *			    - In 'Normal mode' this is interpreted as clockticks relative to the start 
 *			      of the program. If the task is posted at time <cur>, the posted task will 
 *			      be executed with a delay of (<time> - <cur>) clock ticks. 
 *			    - In 'Reset mode', the timer counter is reset to 0 before posting the task. As a
 *			      result <time> is interpreted as clockticks relative to the CURRENT time. This 
 *			      means that the posted task will always be executed in <time> clockticks 
 *			      regardless of the current value of the counter.
 *			If the specified <time> is in the past, the task is scheduled immediately with the
 *			task scheduler.
 * \param priority	The priority with which the task should be executed
 *
 * \returns error_t	SUCCESS if the task was posted successfully
 *			ENOMEM if the task could not be posted there are already too many tasks waiting for 
 * 			execution.
 *			EINVAL if an invalid priority was specified.
 *
 */
error_t timer_post_task(task_t task, int32_t time, uint8_t priority);


/*! \brief Post a task <task> to be scheduled with a certain <delay> with a given <priority>
 *
 * This function behaves in much the same way as timer_post_task, except that instead of specifying the 
 * time at which a task should be executed, the task is executed with a certain <delay>.
 *
 * \param task		The task to be executed at the given time. Please note that posting a task
 *			With the framework timers does NOT automatically register them with the scheduler.
 *			If the posted task is not registered with the scheduler, the task will not be 
 *			executed.
 * \param delay		The delay with which the task is to be executed. Unlike with the 'timer_post_task' 
 * 			function, the interpretation of the delay DOES NOT depend on the operation mode of 
 * 			the timers. In both operation modes, <delay> is interpreted as the number of clock 
 * 			ticks to wait before executing the task relative to the time at which the task was 
 * 			posted.
 * \param priority	The priority with which the task should be executed
 *
 * \returns error_t	SUCCESS if the task was posted successfully
 *			ENOMEM if the task could not be posted there are already too many tasks waiting for 
 * 			execution.
 *			EINVAL if an invalid priority was specified.
 *
 */
static inline error_t timer_post_task_delay(task_t task, int32_t delay, uint8_t priority)
{ 
#ifdef FRAMEWORK_TIMER_RESET_COUNTER
    timer_post_event(task,delay,priority);
#else
    timer_post_event(task,timer_get_counter_value()+delay, delay,priority);
#endif //FRAMEWORK_TIMER_RESET_COUNTER

}

/*! \brief Schedule a given <timer_event>
 *
 * This function is synonymous to calling
 * \code{.c}
 * 	timer_post_event(event->f, event->next_event, event->priority);
 * \endcode
 * 
 * \param event		The event to schedule
 * \returns error_t	The same values returned by timer_post_task
 */
static inline error_t timer_add_event( timer_event* event) { return timer_post_event(event->f, event->next_event, event->priority);}


/*! \brief Cancel a previously scheduled task
 *
 * \param task	The task to cancel.
 * 
 */
error_t timer_cancel_event(task_t task);

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
 * \return uint32_t	The current value of the counter
 *
 */
uint32_t timer_get_counter_value();

#endif /* TIMER_H_ */
