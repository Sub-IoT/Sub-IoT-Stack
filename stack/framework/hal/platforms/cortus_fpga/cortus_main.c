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

#include "machine/gpio.h"

#include "scheduler.h"
#include "bootstrap.h"
#include "platform.h"

#include "hwgpio.h"
#include "hwleds.h"
#include "hwwatchdog.h"



void __platform_init()
{
    __gpio_init();
    console_init();
    __led_init();

    #ifdef USE_CC1101
    // configure the interrupt pins here, since hw_gpio_configure_pin() is MCU
    // specific and not part of the common HAL API
    hw_gpio_configure_pin(CC1101_GDO0_PIN, true, gpioModeInput, 0);
    // hw_gpio_configure_pin(CC1101_SPI_PIN_CS, false, gpioModePushPull, 1);
#endif

    __watchdog_init();
}

int main (void)
{

    __platform_init();

    __framework_bootstrap();

    //__platform_post_framework_init();

    scheduler_run();

    return 0;
}
