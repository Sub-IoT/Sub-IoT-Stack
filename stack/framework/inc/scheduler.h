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

/*! \file scheduler.h
 * \addtogroup scheduler
 * \ingroup framework
 * @{
 * \brief Specifies the API to the priority scheduler of the framework
 *
 * TODO: add more explanations on how the scheduler works (eg FIFO, strict priority queueing), how is control is yielded to the application
 * \author daniel.vandenakker@uantwerpen.be
 */
#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "link_c.h"
#include "types.h"
#include "errors.h"

/*! \brief Type definition for tasks
 *
 */
typedef void (*task_t)();

/*! \brief Initialise the scheduler sub system. 
 *
 * This function is called while bootstrapping the framework. On no account should you call this function 
 * yourself.
 *
 */
__LINK_C void scheduler_init();

/*! \brief The main task loop of the scheduler
 *
 * This function executes the main task loop of the scheduler. It iteratively executes tasks (according to 
 * priority) and puts the MCU to sleep when there are no tasks pending. Please note that this function
 * NEVER returns.
 * 
 * This function should be be called exactly once from the main() function of the specific platform, after 
 * the platform and framework have been initialised. On NO ACCOUNT should this function be called by the 
 * application. 
 *
 */
__LINK_C void scheduler_run();

enum
{
	/*! \brief The minimum allowed priority for scheduled tasks
	 *
	 */
	MIN_PRIORITY = 7,
	/*! \brief The maximum allowed priority for scheduled tasks
	 *
	 */
	MAX_PRIORITY= 0,
	/*! \brief The default priority
	 *
	 */
	DEFAULT_PRIORITY = MIN_PRIORITY,
};

/*! \brief Register a task with the task scheduler.
 *
 *  If the task could not be registered due to memory constraints (This problem can be alleviated by increasing the SCHEDULER_MAX_TASKS CMake parameter) this will assert
 *	Also, when the task was already registered this function will assert.
 *
 * \param task		The task to register
 *
 * \return error_t 	SUCCESS if the task was registered successfully
 */
__LINK_C error_t sched_register_task(task_t task);

/*! \brief Post a task with the given priority
 *
 * \param task		The task to be executed by the scheduler
 * \param priority	The priority of the task
 *
 * \return error_t	SUCCESS if the task was successfully scheduled
 *			EINVAL if the task was not registered with the scheduler
 *			ESIZE if the priority is not between MAX_PRIORITY and MIN_PRIORITY
 *			EALREADY if the task was already scheduled. If this is the case,
 *			the task will be executed but only once.
 */
__LINK_C error_t sched_post_task_prio(task_t task, uint8_t priority);

/*! \brief Post a task at the default priority
 *
 * \param task		The task to be executed by the scheduler
 *
 * \return error_t	SUCCESS if the task was successfully scheduled
 *			EINVAL if the task was not registered with the scheduler
 *			EALREADY if the task was already scheduled. If this is the case,
 *			the task will be executed but only once.
 */
static inline error_t sched_post_task(task_t task) { return sched_post_task_prio(task,DEFAULT_PRIORITY);}

/*! \brief Cancel an already scheduled task
 *
 * \param task		The task to cancel
 *
 * \return error_t	SUCCESS if the task was cancelled successfully
 * 			EINVAL if the task was not registered with the scheduler
 *			EALREADY if the task was not scheduled or has already been executed			
 */
__LINK_C error_t sched_cancel_task(task_t task);

/*! \brief Check whether a task is scheduled to be executed
 *
 * \return bool		TRUE if the task is scheduled, FALSE otherwise
 */
__LINK_C bool sched_is_scheduled(task_t task);

#endif /* SCHEDULER_H_ */

/** @}*/
