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

#include "timer.h"
#include "HalModule.h"

extern "C" void timer_init()
{
	HalModule::getActiveModule()->timer_init();
}
extern "C" timer_tick_t timer_get_counter_value()
{
	return HalModule::getActiveModule()->timer_get_counter_value();
}
extern "C" error_t timer_post_task_prio(task_t task, timer_tick_t time, uint8_t priority)
{
	return HalModule::getActiveModule()->timer_post_task_prio(task, time, priority);
}
extern "C" error_t timer_cancel_task(task_t task)
{
	return HalModule::getActiveModule()->timer_cancel_task(task);
}
