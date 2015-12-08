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

#define INPUT_PORT D8

#ifdef PLATFORM_GECKO
#include "userbutton.h"

void userbutton_callback(button_id_t button_id)
{
	log_print_string("Button PB%u pressed.", button_id);
}

#endif

void timer0_callback()
{
	led_toggle(0);
	timer_post_task_delay(&timer0_callback, TIMER_TICKS_PER_SEC);
	log_print_string("Toggled led %d", 0);
}

void timer1_callback()
{
	led_toggle(1);
	timer_post_task_delay(&timer1_callback, 0x0000FFFF + (uint32_t)100);
	log_print_string("Toggled led %d", 1);
}

static void led_callback(pin_id_t pin_id, uint8_t event_mask)
{
	led_toggle(1);
}

void bootstrap()
{
	led_on(0);
	led_on(1);

	log_print_string("Device booted at time: %d\n", timer_get_counter_value());

    sched_register_task(&timer0_callback);
    sched_register_task(&timer1_callback);

    //timer_post_task_delay(&timer0_callback, TIMER_TICKS_PER_SEC);
    //timer_post_task_delay(&timer1_callback, 0x0000FFFF + (uint32_t)100);

    error_t ret = hw_gpio_configure_pin(INPUT_PORT, 1, 1, 0);
    assert(ret == SUCCESS);

    ret = hw_gpio_configure_interrupt(INPUT_PORT, &led_callback, GPIO_RISING_EDGE | GPIO_FALLING_EDGE);
    assert(ret == SUCCESS);

    ret = hw_gpio_enable_interrupt(INPUT_PORT);
    assert(ret == SUCCESS);



#ifdef PLATFORM_GECKO
    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);
#endif

    led_off(0);
    led_off(1);

}

