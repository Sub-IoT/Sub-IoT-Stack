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

#include "hwdebug.h"
#include "platform.h"
#include "hwgpio.h"
#include "em_gpio.h"
#include <debug.h>

#ifdef PLATFORM_EFM32GG_STK3700_DEBUGPINS

#if DEBUG_PIN_NUM != 3
	#error DEBUG_PIN_NUM does not match the expected value. Update platform.h or platform_debug.c
#endif

//ideally this would be defined using a 'foreach' macro that would automatically define debug_pins
//to the correct number of pins based on DEBUG_PIN_NUM, but that would add tremendous amounts of unnecessary complication
static pin_id_t debug_pins[DEBUG_PIN_NUM];


void __hw_debug_init()
{
	debug_pins[0] = DEBUG0;
	debug_pins[1] = DEBUG1;
	debug_pins[2] = DEBUG2;
	for(int i = 0; i < DEBUG_PIN_NUM; i++)
	{
		error_t err = hw_gpio_configure_pin(debug_pins[i], false, gpioModePushPull, 0);
		assert(err == SUCCESS);
	}
}

void hw_debug_set(uint8_t pin_id)
{
	if(pin_id < DEBUG_PIN_NUM)
		hw_gpio_set(debug_pins[pin_id]);
}

void hw_debug_clr(uint8_t pin_id)
{
	if(pin_id < DEBUG_PIN_NUM)
		hw_gpio_clr(debug_pins[pin_id]);
}

void hw_debug_toggle(uint8_t pin_id)
{
	if(pin_id < DEBUG_PIN_NUM)
		hw_gpio_toggle(debug_pins[pin_id]);
}

void hw_debug_mask(uint32_t mask)
{
	for(int i = 0; i < DEBUG_PIN_NUM; i++)
	{
		if(mask & (1<<i))
			hw_gpio_set(debug_pins[i]);
		else
			hw_gpio_clr(debug_pins[i]);
	}
}

#else

void __hw_debug_init() {}
void hw_debug_set(uint8_t pin_id) {}
void hw_debug_clr(uint8_t pin_id) {}
void hw_debug_toggle(uint8_t pin_id) {}
void hw_debug_mask(uint32_t mask) {}

#endif
