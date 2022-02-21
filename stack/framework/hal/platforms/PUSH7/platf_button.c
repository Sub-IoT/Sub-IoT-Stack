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

#include "platform.h"
#include "button.h"
#include "hwgpio.h"
#include "hwatomic.h"
#include "scheduler.h"
#include <string.h>
#include <debug.h>
#include "errors.h"
#include "stm32_common_gpio.h"

#if PLATFORM_NUM_BUTTONS != 3
  #error "PLATFORM_NUM_BUTTONS does not match the expected value. Update platform CMakeLists.txt or platform_userbutton.c"
#endif

typedef struct
{
    pin_id_t button_id;
    uint8_t number_of_calls;
} button_info_t;

ubutton_callback_t button_state_changed_callback;
button_info_t buttons[PLATFORM_NUM_BUTTONS];


static void button_callback(void* arg);
static void button_task();

__LINK_C void __ubutton_init()
{
	error_t err;
	buttons[0].button_id = BUTTON1;
	buttons[1].button_id = BUTTON2;
	buttons[2].button_id = BUTTON3;
	for(int i = 0; i < PLATFORM_NUM_BUTTONS; i++)
	{

		GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING_FALLING;
        GPIO_InitStruct.Pull  = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        hw_gpio_configure_pin_stm(buttons[i].button_id, &GPIO_InitStruct);

        err = hw_gpio_configure_interrupt(buttons[i].button_id, GPIO_FALLING_EDGE | GPIO_RISING_EDGE, &button_callback, &buttons[i].button_id);
		err = hw_gpio_enable_interrupt(buttons[i].button_id); assert(err == SUCCESS);
	}
	sched_register_task(&button_task);
}


__LINK_C error_t ubutton_register_callback(ubutton_callback_t desired_callback)
{
	button_state_changed_callback = desired_callback;
	return SUCCESS;
}

__LINK_C error_t ubutton_deregister_callback()
{
	button_state_changed_callback = NULL;
	return SUCCESS;
}

static void button_callback(void* arg)
{
	pin_id_t pin_id = *(pin_id_t*)arg;
	for(int i = 0; i < PLATFORM_NUM_BUTTONS; i++)
    {
		if(buttons[i].button_id == pin_id )
		{
			buttons[i].number_of_calls++;
		}
	}
	sched_post_task(&button_task);
}

void button_task()
{
	for(int i = 0; i < PLATFORM_NUM_BUTTONS; i++)
    {
		if(buttons[i].number_of_calls > 0)
		{
			uint8_t calls = buttons[i].number_of_calls;	
			buttons[i].number_of_calls = 0;	
			if(button_state_changed_callback)
				button_state_changed_callback(buttons[i].button_id, hw_gpio_get_in(buttons[i].button_id), calls);
			sched_post_task(&button_task);
			return;
		}
	}
}
