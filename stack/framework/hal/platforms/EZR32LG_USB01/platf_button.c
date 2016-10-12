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

#include "button.h"
#include "hwgpio.h"
#include "hwatomic.h"
#include "scheduler.h"
#include <string.h>
#include <debug.h>
#include "em_gpio.h"

#if NUM_USERBUTTONS != 1
	#error "NUM_USERBUTTONS does not match the expected value. Update platform.h or platform_userbutton.c"
#endif

//yes, ok for perfect configurability this should be added to the cmake file but this will do for now
#define BUTTON_QUEUE_SIZE 10
typedef struct
{
	pin_id_t button_id;
	ubutton_callback_t callbacks[BUTTON_QUEUE_SIZE];
	uint8_t cur_callback_id;
	uint8_t num_registered_callbacks;
} button_info_t;

button_info_t buttons[NUM_USERBUTTONS];

static void button_callback(pin_id_t pin_id, uint8_t event_mask);
static void button_task();
__LINK_C void __ubutton_init()
{
	error_t err;
    err = hw_gpio_configure_pin(BUTTON0, true, gpioModeInput, 0); assert(err == SUCCESS); // TODO pull up or pull down to prevent floating
    //err = hw_gpio_configure_pin(BUTTON1, true, gpioModeInput, 0); assert(err == SUCCESS); // TODO pull up or pull down to prevent floating

	buttons[0].button_id = BUTTON0;
	//buttons[1].button_id = BUTTON1;
	for(int i = 0; i < NUM_USERBUTTONS; i++)
	{
		memset(buttons[i].callbacks, 0, sizeof(buttons[i].callbacks));
		buttons[i].cur_callback_id = BUTTON_QUEUE_SIZE;
		buttons[i].num_registered_callbacks = 0;
		error_t err = hw_gpio_configure_interrupt(buttons[i].button_id, &button_callback, GPIO_FALLING_EDGE);
		assert(err == SUCCESS);
	}
	sched_register_task(&button_task);
}

__LINK_C bool ubutton_pressed(button_id_t button_id)
{
	//note: we invert gpio_get_in since the GPIO pin is pulled low when the button is pressed
	return (button_id < NUM_USERBUTTONS) && (!hw_gpio_get_in(buttons[button_id].button_id));
}

__LINK_C error_t ubutton_register_callback(button_id_t button_id, ubutton_callback_t callback)
{
	if(button_id >= NUM_USERBUTTONS)
		return ESIZE;
	else if (callback == 0x0)
		return EINVAL;

	uint8_t empty_index = BUTTON_QUEUE_SIZE;
	for(int i = 0; i < BUTTON_QUEUE_SIZE; i++)
	{
		if(empty_index == BUTTON_QUEUE_SIZE && buttons[button_id].callbacks[i] == 0x0)
			empty_index = i;
		else if(buttons[button_id].callbacks[i] == callback)
			return EALREADY;
	}

	if(empty_index >= BUTTON_QUEUE_SIZE)
		return ENOMEM;

	start_atomic();
		buttons[button_id].callbacks[empty_index] = callback;
		buttons[button_id].num_registered_callbacks++;
		if(buttons[button_id].num_registered_callbacks == 1)
		{
			//this is the first listener to register --> enable the GPIO interrupt
			error_t err = hw_gpio_enable_interrupt(buttons[button_id].button_id);
			assert(err == SUCCESS);
		}
	end_atomic();
	return SUCCESS;
}

__LINK_C error_t ubutton_deregister_callback(button_id_t button_id, ubutton_callback_t callback)
{
	if(button_id >= NUM_USERBUTTONS)
		return ESIZE;
	else if (callback == 0x0)
		return EINVAL;

	uint8_t callback_index = BUTTON_QUEUE_SIZE;
	for(int i = 0; i < BUTTON_QUEUE_SIZE; i++)
	{
		if(buttons[button_id].callbacks[i] == callback)
		{
			callback_index = i;
			break;
		}
	}

	if(callback_index >= BUTTON_QUEUE_SIZE)
		return EALREADY;


	start_atomic();
		buttons[button_id].callbacks[callback_index] = 0x0;
		buttons[button_id].num_registered_callbacks--;
		if(buttons[button_id].num_registered_callbacks == 0)
		{
			//this is the last listener to deregister --> disable the GPIO interrupt
			error_t err = hw_gpio_disable_interrupt(buttons[button_id].button_id);
			assert(err == SUCCESS);
		}
	end_atomic();
	return SUCCESS;
}

static void button_callback(pin_id_t pin_id, uint8_t event_mask)
{
	for(int i = 0; i < NUM_USERBUTTONS;i++)
	{
		if(hw_gpio_pin_matches(buttons[i].button_id, pin_id))
		{
			//set cur_callback_id to 0 to trigger all registered callbacks and post a task to do the actual callbacks
			buttons[i].cur_callback_id = 0;
			sched_post_task(&button_task);
		}
	}
}

void button_task()
{
	button_id_t button_id = NUM_USERBUTTONS;
	ubutton_callback_t callback = 0x0;

	start_atomic();
	for(int i = 0; i < NUM_USERBUTTONS;i++)
	{
		for(;buttons[i].cur_callback_id < BUTTON_QUEUE_SIZE && buttons[i].callbacks[buttons[i].cur_callback_id] == 0x0; buttons[i].cur_callback_id++);
		if(buttons[i].cur_callback_id < BUTTON_QUEUE_SIZE)
		{
			callback = buttons[i].callbacks[buttons[i].cur_callback_id];
			button_id = i;
			buttons[i].cur_callback_id++;
			break;
		}
	}
	end_atomic();

	if(button_id < NUM_USERBUTTONS && callback != 0x0)
	{
		//reschedule the task to do the next callback (if needed)
		sched_post_task(&button_task);
		callback(button_id);
	}
}

