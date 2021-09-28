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

#include "bootstrap.h"
#include "hwgpio.h"
#include "hwleds.h"
#include "led.h"
#include "button.h"
#include "stm32_device.h"
#include "stm32_common_gpio.h"
#include "platform.h"
#include "debug.h"
#include "hwdebug.h"
#include "errors.h"

#if defined(USE_SX127X) && defined(PLATFORM_SX127X_USE_RESET_PIN)
static void reset_sx127x()
{
  error_t e;
  e = hw_gpio_configure_pin(SX127x_RESET_PIN, false, GPIO_MODE_OUTPUT_PP, 0); assert(e == SUCCESS);
  hw_busy_wait(150);
  e = hw_gpio_configure_pin(SX127x_RESET_PIN, false, GPIO_MODE_INPUT, 1); assert(e == SUCCESS);
  hw_busy_wait(10000);
}
#endif

void __platform_init()
{
    __stm32_common_mcu_init();
    __gpio_init();

#ifdef USE_SX127X

    // configure the interrupt pins here, since hw_gpio_configure_pin() is MCU
    // specific and not part of the common HAL API
    hw_gpio_configure_pin(SX127x_DIO0_PIN, true, GPIO_MODE_INPUT, 0);
    hw_gpio_configure_pin(SX127x_DIO1_PIN, true, GPIO_MODE_INPUT, 0);
#ifdef PLATFORM_SX127X_USE_DIO3_PIN
    hw_gpio_configure_pin(SX127x_DIO3_PIN, true, GPIO_MODE_INPUT, 0);
#endif
#ifdef PLATFORM_SX127X_USE_RESET_PIN
    hw_gpio_configure_pin(SX127x_RESET_PIN, false, GPIO_MODE_OUTPUT_PP, 1);
    reset_sx127x();
#endif
#ifdef PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN
    hw_gpio_configure_pin(SX127x_MANUAL_RXTXSW_PIN, false, GPIO_MODE_OUTPUT_PP, 0);
#endif
#endif

#if defined(USE_SX127X) && defined(PLATFORM_SX127X_USE_RESET_PIN)
    reset_sx127x();
#endif

    HAL_EnableDBGSleepMode(); // TODO impact on power?
}

void __platform_post_framework_init()
{
}


int main()
{
    // initialise the platform itself
    __platform_init();
    // do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    // initialise platform functionality that depends on the framework
    __platform_post_framework_init();

    scheduler_run();
    return 0;
}
