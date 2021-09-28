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

/*! \file platform_leds.c
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author daniel.vandenakker@uantwerpen.be
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include "hwleds.h"
#include "platform.h"
#include "em_gpio.h"
#include <debug.h>
#include "timer.h"
#include "led.h"
#include "errors.h"

#if PLATFORM_NUM_LEDS < 1
  #error PLATFORM_NUM_LEDS does not match the expected value. Update platform.h or platform_leds.c
#endif
static pin_id_t leds[ PLATFORM_NUM_LEDS ];

void __led_init()
{
	leds[0] = LED0;
#if PLATFORM_NUM_LEDS >= 2
	leds[1] = LED1;
#endif
#if PLATFORM_NUM_LEDS >= 3
	leds[2] = LED2;
#endif
  for(int i = 0; i < PLATFORM_NUM_LEDS; i++)
	{
		error_t err = hw_gpio_configure_pin(leds[i], false, gpioModePushPull, 0);
		assert(err == SUCCESS);
	}
}

void led_on(uint8_t led_nr)
{
    if(led_nr < PLATFORM_NUM_LEDS)
    	hw_gpio_set(leds[led_nr]);
}

void led_off(unsigned char led_nr)
{
    if(led_nr < PLATFORM_NUM_LEDS)
    	hw_gpio_clr(leds[led_nr]);
}

void led_toggle(unsigned char led_nr)
{
    if(led_nr < PLATFORM_NUM_LEDS)
    	hw_gpio_toggle(leds[led_nr]);
}


// flashing support

static void end_flash_green()  { led_off(LED_GREEN);  }
static void end_flash_orange()   { led_off(LED_ORANGE); }
static void end_flash_red()    { led_off(LED_RED);    }

void led_flash_green() {
  led_on(LED_GREEN);
  timer_post_task_delay(&end_flash_green, FLASH_DURATION);
}

void led_flash_orange() {
  led_on(LED_ORANGE);
  timer_post_task_delay(&end_flash_orange, FLASH_DURATION);
}

void led_flash_red() {
  led_on(LED_RED);
  timer_post_task_delay(&end_flash_red, FLASH_DURATION);
}

bool led_init() {
  sched_register_task(&end_flash_green);
  sched_register_task(&end_flash_orange);
  sched_register_task(&end_flash_red);
  return true;
}

