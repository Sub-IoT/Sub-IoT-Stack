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


#include "stm32l0xx_mcu.h"
#include "bootstrap.h"
#include "hwgpio.h"
#include "hwleds.h"
#include "led.h"
#include "button.h"
#include "stm32l0xx_gpio.h"
#include "hwsystem.h"
#include "debug.h"
#include "stm32l0xx_pins.h"

#if defined(USE_SX127X) && defined(PLATFORM_SX127X_USE_RESET_PIN)
static void reset_sx127x()
{
  error_t e;
  e = hw_gpio_configure_pin(SX127x_RESET_PIN, false, GPIO_MODE_OUTPUT_PP, 0); assert(e == SUCCESS); // TODO platform specific
  hw_busy_wait(150);
  e = hw_gpio_configure_pin(SX127x_RESET_PIN, false, GPIO_MODE_INPUT, 1); assert(e == SUCCESS); // TODO platform specific
  hw_busy_wait(6000);
}
#endif

void __platform_init()
{
    __stm32l0xx_mcu_init();
    __gpio_init();

#if defined(USE_SX127X) && defined(PLATFORM_SX127X_USE_RESET_PIN)
    reset_sx127x();
#endif

}

void __platform_post_framework_init()
{
    __ubutton_init();
    led_init();
}

int main()
{
//    //initialise the platform itself
    __platform_init();
//    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
//    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();

    scheduler_run();
    return 0;
}
