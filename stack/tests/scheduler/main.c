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
#include "scheduler.h"
#include "assert.h"
#include "errors.h"
#include "stdio.h"
#include <stdlib.h>

bool task1_called = false;
bool task2a_called = false;
bool task2b_called = false;
bool task3a_called = false;
bool task3b_called = false;
bool task4a_called = false;
bool task5_called[] = {false, false};
bool task6_called[] = {false, false, false, false};
bool task7_called[] = {false, false};


void task1(void* arg)
{
    task1_called = true;
}

void task2a(void* arg)
{
    task2a_called = true;
    assert(!sched_is_scheduled(&task2a));
}

void task2b(void* arg)
{
    assert(sched_post_task_prio(&task2a, 6, NULL) == SUCCESS);
    task2b_called = true;
}

void task3a(void* arg)
{
    task3a_called = true;
}

void task3b(void* arg)
{
    task3b_called = true;
    assert(sched_is_scheduled(&task3a));
    assert(sched_cancel_task(&task3a) == SUCCESS);
}

void task4a(void* arg)
{
    task4a_called = true;
    assert(arg == (void*)1234);
}

void task5(void* arg)
{
    unsigned long long index = (unsigned long long)arg;
    assert(index<2);
    task5_called[index] = true;
    if(index == 0)
    {
        assert(sched_post_task_prio(&task5, 6, (void*)1) == SUCCESS);
    }
}

void task6(void* arg)
{
    unsigned long long index = (unsigned long long)arg;
    assert(index<4);
    task6_called[index] = true;
}

void task7(void* arg)
{
    unsigned long long index = (unsigned long long)arg;
    assert(index<2);
    task7_called[index] = true;
    if(index == 0)
    {
        assert(sched_cancel_task_with_arg(&task7, (void*)1) == SUCCESS);
    }
}


void end_task(void*arg)
{
    assert(task1_called);
    assert(task2a_called);
    assert(task2b_called);
    assert(!task3a_called);
    assert(task3b_called);
    assert(task4a_called);
    assert(task5_called[0]);
    assert(task5_called[1]);
    assert(task6_called[0]);
    assert(task6_called[1]);
    assert(task6_called[2]);
    assert(task6_called[3]);
    assert(task7_called[0]);
    assert(!task7_called[1]);
    printf("All scheduler tests passed!\n");
    exit(0);
}

void bootstrap()
{
    assert(sched_post_task_prio(&task1, 6, NULL) == -EINVAL);
    assert(sched_cancel_task(&task1) == -EINVAL);
    assert(!sched_is_scheduled(&task1));
    assert(sched_register_task(&task1) == SUCCESS);
    assert(sched_cancel_task(&task1) == -EALREADY);
    assert(!sched_is_scheduled(&task1));
    assert(sched_register_task(&task1) == -EALREADY);
    assert(sched_register_task(&task2a) == SUCCESS);
    assert(sched_register_task(&task2b) == SUCCESS);
    assert(sched_register_task(&task3a) == SUCCESS);
    assert(sched_register_task(&task3b) == SUCCESS);
    assert(sched_register_task(&task4a) == SUCCESS);
    assert(sched_register_task(&task5) == SUCCESS);
    assert(sched_register_task(&end_task) == SUCCESS);

    assert(sched_post_task_prio(&task1, 6, NULL) == SUCCESS);
    assert(sched_post_task_prio(&task1, 6, NULL) == -EALREADY);

    assert(sched_post_task_prio(&task2b, 6, NULL) == SUCCESS);

    assert(sched_post_task_prio(&task3b, 6, NULL) == SUCCESS);
    assert(sched_post_task_prio(&task3a, 6, NULL) == SUCCESS);

    assert(sched_post_task_prio(&task4a, 6, (void*)1234) == SUCCESS);
    assert(sched_post_task_prio(&task4a, 6, (void*)4321) == -EALREADY);

    assert(sched_post_task_prio(&task5, 6, (void*)0) == SUCCESS);

    assert(sched_register_task_allow_multiple(&task6, true) == SUCCESS);
    assert(!sched_is_scheduled_with_arg(&task6, (void*)0));
    assert(sched_post_task_prio(&task6, 6, (void*)0) == SUCCESS);
    assert(sched_is_scheduled_with_arg(&task6, (void*)0));
    assert(sched_register_task_allow_multiple(&task6, true) == SUCCESS);
    assert(sched_post_task_prio(&task6, 6, (void*)0) == -EALREADY);
    assert(sched_post_task_prio(&task6, 6, (void*)1) == SUCCESS);
    assert(sched_is_scheduled_with_arg(&task6, (void*)1));
    assert(sched_register_task_allow_multiple(&task6, true) == SUCCESS);
    assert(sched_post_task_prio(&task6, 6, (void*)2) == -SUCCESS);
    assert(sched_register_task_allow_multiple(&task6, true) == SUCCESS);
    assert(sched_post_task_prio(&task6, 6, (void*)3) == -SUCCESS);
    assert(sched_post_task_prio(&task6, 6, (void*)4) == -ENOMEM);

    assert(sched_register_task_allow_multiple(&task7, true) == SUCCESS);
    assert(sched_post_task_prio(&task7, 6, (void*)0) == SUCCESS);
    assert(sched_register_task_allow_multiple(&task7, true) == SUCCESS);
    assert(sched_post_task_prio(&task7, 6, (void*)1) == SUCCESS);
    
    assert(sched_post_task_prio(&end_task, MIN_PRIORITY, NULL) == SUCCESS);

}
