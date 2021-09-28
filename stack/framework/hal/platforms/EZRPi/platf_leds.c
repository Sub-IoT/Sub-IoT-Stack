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

/*! \file platf_leds.c
 *
 *  \author contact@christophe.vg
 *
 */

#include <debug.h>

#include "em_gpio.h"

#include "hwleds.h"

#include "timer.h"

#include "platform.h"

#include "led.h"

#if PLATFORM_NUM_LEDS != 1
	#error PLATFORM_NUM_LEDS does not match the expected value. Update platform.h or platf_leds.c
#endif

void __led_init() {
	error_t err = hw_gpio_configure_pin(LED0, false, gpioModePushPull, 0);
	assert(err == SUCCESS);
}

void led_on(uint8_t led_nr) {
	hw_gpio_set(LED0);
}

void led_off(unsigned char led_nr) {
	hw_gpio_clr(LED0);
}

void led_toggle(unsigned char led_nr) {
  hw_gpio_toggle(LED0);
}

// flashing support

static void end_flash()  { led_off(LED_BLUE);  }

void led_flash() {
  led_on(LED_BLUE);
  timer_post_task_delay(&end_flash, FLASH_DURATION);
}

bool led_init() {
  sched_register_task(&end_flash);
  return true;
}
