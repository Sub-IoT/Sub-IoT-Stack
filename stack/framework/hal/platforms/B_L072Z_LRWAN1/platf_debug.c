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
#include <debug.h>
#include "ports.h"
#include "errors.h"
#include "stm32_common_gpio.h"

#if defined(PLATFORM_USE_DEBUGPINS) && PLATFORM_NUM_DEBUGPINS > 0

// TODO refactor: can be common for all platforms after hw_gpio_configure_pin() is made public

void __hw_debug_init()
{
  for(int i = 0; i < PLATFORM_NUM_DEBUGPINS; i++)
	{
    error_t err = hw_gpio_configure_pin(debug_pins[i], false, GPIO_MODE_OUTPUT_PP, 0);
		assert(err == SUCCESS);
	}
}

void hw_debug_set(uint8_t pin_nr)
{
    if(pin_nr < PLATFORM_NUM_DEBUGPINS)
      hw_gpio_set(debug_pins[pin_nr]);
}

void hw_debug_clr(uint8_t pin_nr)
{
    if(pin_nr < PLATFORM_NUM_DEBUGPINS)
      hw_gpio_clr(debug_pins[pin_nr]);
}

void hw_debug_toggle(uint8_t pin_nr)
{
    if(pin_nr < PLATFORM_NUM_DEBUGPINS)
      hw_gpio_toggle(debug_pins[pin_nr]);
}

void hw_debug_mask(uint32_t mask)
{
  for(int i = 0; i < PLATFORM_NUM_DEBUGPINS; i++)
	{
		if(mask & (1<<i))
			hw_gpio_set(debug_pins[i]);
		else
			hw_gpio_clr(debug_pins[i]);
	}
}

#else

void __hw_debug_init() {}
void hw_debug_set(uint8_t pin_nr) {}
void hw_debug_clr(uint8_t pin_nr) {}
void hw_debug_toggle(uint8_t pin_nr) {}
void hw_debug_mask(uint32_t mask) {}

#endif
