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


#include "stm32l152_mcu.h"
#include "bootstrap.h"
#include "hwgpio.h"
#include "hwleds.h"
#include "led.h"
#include "button.h"
#include "stm32l1xx_hal_gpio.h"

void __platform_init()
{
    __stm32l152_mcu_init();
    __gpio_init();
    __led_init();
#ifdef USE_CC1101
    // configure the interrupt pins here, since hw_gpio_configure_pin() is MCU
    // specific and not part of the common HAL API
    hw_gpio_configure_pin(CC1101_GDO0_PIN, true, GPIO_MODE_IT_FALLING, 0);
#endif

}

void __platform_post_framework_init()
{
    __ubutton_init();
    led_init();
}

//Quick hack, approximately 1ms delay


int main()
{
//    //initialise the platform itself
    __platform_init();
//    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
//    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();

//    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;  // enable the clock to GPIOD
//    __asm("dsb");                         // stall instruction pipeline, until instruction completes, as
//                                          //    per Errata 2.1.13, "Delay after an RCC peripheral clock enabling"
//    GPIOD->MODER = (1 << 26);             // set pin 13 to be general purpose output
//
//    for (;;) {
//       int i = 0;
//       ms_delay(1000);
//       GPIOD->ODR ^= (1 << 13);           // Toggle the pin
//    }

//    while (1)
//    {
//    	if (!hw_gpio_get_in(BUTTON0)) {
//    		hw_gpio_set(LED0);
//    	} else {
//    		hw_gpio_clr(LED0);
//    	}
//    	//HAL_Delay(500);
//     }


    scheduler_run();
    return 0;
}
