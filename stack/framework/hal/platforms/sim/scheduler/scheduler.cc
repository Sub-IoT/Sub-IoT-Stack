//
// OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
// lowpower wireless sensor communication
//
// Copyright 2015 University of Antwerp
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "scheduler.h"
#include "HalModule.h"
#include <omnetpp.h>

extern "C" void scheduler_init()
{
	HalModule::getActiveModule()->scheduler_init();
}

/* \brief The main task loop of the scheduler
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
extern "C" void scheduler_run()
{
	opp_error("scheduler_run called when running in a sim environment");
}

extern "C" error_t sched_register_task(task_t task)
{
	return HalModule::getActiveModule()->sched_register_task(task);
}

extern "C" error_t sched_post_task_prio(task_t task, uint8_t priority)
{
	return HalModule::getActiveModule()->sched_post_task_prio(task, priority);
}

extern "C" error_t sched_cancel_task(task_t task)
{
	return HalModule::getActiveModule()->sched_cancel_task(task);
}

extern "C" bool sched_is_scheduled(task_t task)
{
	return HalModule::getActiveModule()->sched_is_scheduled(task);
}
