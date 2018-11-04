/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2018 University of Antwerp
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

#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include "debug.h"
#include "pt.h"

static bool unblock_task1 = false;
static struct pt pt_task1;

static PT_THREAD(task1_thread(struct pt* pt))
{
  PT_BEGIN(pt);
  unblock_task1 = false;

  log_print_string("task1 blocked\n");
  PT_WAIT_UNTIL(pt, unblock_task1);
  log_print_string("task1 unblocked\n");

  PT_END(pt);
}

static void task1(void* ptr)
{
  log_print_string("start task1\n");
  PT_INIT(&pt_task1);

  sched_run_thread(&task1_thread, &pt_task1);

  log_print_string("end task1\n");
}

static void task2(void* ptr)
{
  log_print_string("task2: unblock task1\n");
  unblock_task1 = true;
}

void bootstrap()
{  
  sched_register_task(&task1);
  sched_register_task(&task2);

  sched_post_task(&task1);
  timer_post_task_delay(&task2, TIMER_TICKS_PER_SEC);
}
