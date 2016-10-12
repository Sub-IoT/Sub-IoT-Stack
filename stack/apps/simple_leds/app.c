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

#include "hwleds.h"
#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include "debug.h"
#include "platform.h"
#include "hwwatchdog.h"

#if NUM_USERBUTTONS > 0
#include "button.h"

void userbutton_callback(button_id_t button_id)
{
	log_print_string("Button PB%u pressed.", button_id);
}

#endif

void led_off_callback()
{
	led_off(0);
	log_print_string("Toggled off %d", 0);

}

void led_on_callback()
{
	led_on(0);
	timer_post_task_delay(&led_on_callback, TIMER_TICKS_PER_SEC);
	timer_post_task_delay(&led_off_callback, TIMER_TICKS_PER_SEC*0.050);
	log_print_string("Toggled on %d", 0);

	hw_watchdog_feed();

}



void timer1_callback()
{
	led_toggle(1);
	timer_post_task_delay(&timer1_callback, 0x0000FFFF + (uint32_t)100);
	log_print_string("Toggled led %d", 1);
}

void bootstrap()
{
	led_on(0);
	led_on(1);

	log_print_string("Device booted at time: %d\n", timer_get_counter_value());

    sched_register_task(&led_on_callback);
    sched_register_task(&led_off_callback);
    sched_register_task(&timer1_callback);

    timer_post_task_delay(&led_on_callback, TIMER_TICKS_PER_SEC);
    timer_post_task_delay(&timer1_callback, 0x0000FFFF + (uint32_t)100);

#if NUM_USERBUTTONS > 0
    int i= 0;
    for (i=0;i<NUM_USERBUTTONS;i++)
	{
    	ubutton_register_callback(i, &userbutton_callback);
	}
#endif

    led_off(0);
    led_off(1);


}

