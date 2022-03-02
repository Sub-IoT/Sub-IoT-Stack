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

#include "button.h"
#include "errors.h"
#include "hwatomic.h"
#include "hwgpio.h"
#include "platform.h"
#include "scheduler.h"
#include "stm32_common_gpio.h"
#include "timer.h"
#include <debug.h>
#include <string.h>

#if PLATFORM_NUM_BUTTONS != 3
#error "PLATFORM_NUM_BUTTONS does not match the expected value. Update platform CMakeLists.txt or platform_userbutton.c"
#endif

typedef struct {
    pin_id_t button_id;
    timer_t pressed_down_time;
    bool last_known_state;
    bool triggered;
} button_info_t;


ubutton_callback_t button_state_changed_callback;
button_info_t buttons[PLATFORM_NUM_BUTTONS];

static void button_callback(void* arg);
static void button_task();

__LINK_C void __ubutton_init()
{
    error_t err;
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    buttons[0].button_id = BUTTON1;
    buttons[1].button_id = BUTTON2;
    buttons[2].button_id = BUTTON3;
    for (int i = 0; i < PLATFORM_NUM_BUTTONS; i++) 
	{
        hw_gpio_configure_pin_stm(buttons[i].button_id, &GPIO_InitStruct);
        err = hw_gpio_configure_interrupt(buttons[i].button_id, GPIO_FALLING_EDGE | GPIO_RISING_EDGE, &button_callback, &buttons[i].button_id);
        assert(err == SUCCESS);
    }
    sched_register_task(&button_task);
}

__LINK_C error_t ubutton_register_callback(ubutton_callback_t desired_callback)
{
    error_t err;
    button_state_changed_callback = desired_callback;
    for (int i = 0; i < PLATFORM_NUM_BUTTONS; i++) 
	{
        err = hw_gpio_enable_interrupt(buttons[i].button_id);
        assert(err == SUCCESS);
    }
    return SUCCESS;
}

__LINK_C error_t ubutton_deregister_callback()
{
    error_t err;
    button_state_changed_callback = NULL;
    for (int i = 0; i < PLATFORM_NUM_BUTTONS; i++) 
	{
        err = hw_gpio_disable_interrupt(buttons[i].button_id);
        assert(err == SUCCESS);
    }
    return SUCCESS;
}

static void button_callback(void* arg)
{
    pin_id_t pin_id = *(pin_id_t*)arg;
    for (int i = 0; i < PLATFORM_NUM_BUTTONS; i++) 
	{
        if(buttons[i].button_id == pin_id) 
		{
            buttons[i].triggered = true;
        }
    }
	if(button_state_changed_callback)
    	sched_post_task(&button_task);
}

void button_task()
{
    for (int i = 0; i < PLATFORM_NUM_BUTTONS; i++) 
	{
        if(buttons[i].triggered) 
		{
            timer_t elapsed = 0;
            uint8_t all_button_state = 0;
            buttons[i].triggered = false;
            bool previous_state = buttons[i].last_known_state;
            buttons[i].last_known_state = hw_gpio_get_in(buttons[i].button_id);

			//ensure we don't repeat the same action
            if (buttons[i].last_known_state == previous_state) 
                return;

			//Keep track of pressed time and calculate time difference when letting go
            if (buttons[i].last_known_state) 
			{
                buttons[i].pressed_down_time = timer_get_counter_value();
            } 
			else
			{
                elapsed = timer_get_counter_value() - buttons[i].pressed_down_time;
                buttons[i].pressed_down_time = 0;
            }

			// gather all button states
            for (uint8_t i = 0; i < PLATFORM_NUM_BUTTONS; i++) 
			{
                all_button_state += buttons[i].last_known_state << i;
            }

			uint8_t elapsed_deciseconds = (elapsed/100) > 255 ? 255 : (uint8_t)(elapsed/100);
			
			button_state_changed_callback(i, buttons[i].last_known_state, elapsed_deciseconds, all_button_state);
            sched_post_task(&button_task);
            return;
        }
    }
}
