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

/*
 * \author	maarten.weyn@uantwerpen.be
 */

#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include <assert.h>
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>

#include "hwlcd.h"
#include "d7ap_stack.h"
#include "dll.h"

void start_foreground_scan()
{
    // TODO we start FG scan manually now, later it should be started by access profile automatically
    dll_start_foreground_scan();
}

void bootstrap()
{

    d7ap_stack_init();
    sched_register_task(&start_foreground_scan);
    sched_post_task(&start_foreground_scan);

    lcd_write_string("started");
}

