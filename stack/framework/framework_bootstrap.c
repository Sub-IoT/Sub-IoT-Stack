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

#include "scheduler.h"
#include "timer.h"
#include "hwsystem.h"
#include "random.h"
#include "log.h"
void bootstrap();
void __framework_bootstrap()
{
    //initialise the scheduler & timers
    scheduler_init();
    timer_init();
    //initialise libc RNG with the unique device id
    set_rng_seed(hw_get_unique_id());
    //reset the log counter
    log_counter_reset();

    //register the user bootstrap function();
    sched_register_task(&bootstrap);
    sched_post_task(&bootstrap);
}
