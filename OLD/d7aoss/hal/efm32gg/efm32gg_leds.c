/*! \file efm32gg_leds.c
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#include "leds.h"

#include "em_cmu.h"
#include "em_gpio.h"

typedef struct
{
  GPIO_Port_TypeDef port;
  unsigned int pin;
} leds_t;

static const leds_t leds[ 2 ] = { { gpioPortE, 2 }, { gpioPortE, 3 } };

void led_init()
{
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
    for (int i = 0; i < 2; i++)
    {
      GPIO_PinModeSet(leds[i].port, leds[i].pin, gpioModePushPull, 0);
    }
}

void led_on(unsigned char led_nr)
{
    GPIO_PinOutSet(leds[led_nr].port, leds[led_nr].pin);
}

void led_off(unsigned char led_nr)
{
    GPIO_PinOutClear(leds[led_nr].port, leds[led_nr].pin);
}

void led_toggle(unsigned char led_nr)
{
    GPIO_PinOutToggle(leds[led_nr].port, leds[led_nr].pin);
}

void led_blink(unsigned char led_id)
{
  // TODO
}