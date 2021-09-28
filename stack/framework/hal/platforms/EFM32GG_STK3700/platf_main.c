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

#include "scheduler.h"
#include "bootstrap.h"
#include "hwgpio.h"
#include "hwuart.h"
#include "hwleds.h"
#include "hwlcd.h"
#include "hwusb.h"
#include "efm32gg_mcu.h"
#include "hwdebug.h"
#include "hwwatchdog.h"
#include "platform.h"
#include "button.h"
#include "platform_sensors.h"
#include "em_gpio.h"
#include <debug.h>


#include "bsp_trace.h"

#include "console.h"

void __platform_init()
{
    __efm32gg_mcu_init();
    __gpio_init();
    __led_init();    // uses ports assigned to UART1 LOC3
    __lcd_init();

#ifdef USE_CC1101
    // configure the interrupt pins here, since hw_gpio_configure_pin() is MCU
    // specific and not part of the common HAL API
    hw_gpio_configure_pin(CC1101_GDO0_PIN, true, gpioModeInput, 0);
    hw_gpio_configure_pin(CC1101_GDO2_PIN, true, gpioModeInput, 0);
    // hw_gpio_configure_pin(CC1101_SPI_PIN_CS, false, gpioModePushPull, 1);
#endif
#ifdef USE_SX127X
    // configure the interrupt pins here, since hw_gpio_configure_pin() is MCU
    // specific and not part of the common HAL API
    hw_gpio_configure_pin(SX127x_DIO0_PIN, true, gpioModeInputPull, 0);
    hw_gpio_configure_pin(SX127x_DIO1_PIN, true, gpioModeInputPull, 0);
    hw_gpio_configure_pin(SX127x_DIO3_PIN, true, gpioModeInputPull, 0);
    hw_gpio_configure_pin(SX127x_RESET_PIN, false, gpioModePushPull, 1);
#endif
    __hw_debug_init();

    error_t err;
    err = hw_gpio_configure_pin(BUTTON0, true, gpioModeInput, 0); assert(err == SUCCESS); // TODO pull up or pull down to prevent floating
    err = hw_gpio_configure_pin(BUTTON1, true, gpioModeInput, 0); assert(err == SUCCESS); // TODO pull up or pull down to prevent floating

    __watchdog_init(); // TODO configure from cmake?
}

void __platform_post_framework_init()
{
    __ubutton_init();
	#ifdef PLATFORM_USE_USB_CDC

    //Moved here to make sure LF XO is set in timer.
    __usb_init_cdc();
	#endif
}

int main()
{
	//BSP_TraceProfilerSetup();
    // Only when using bootloader
	//SCB->VTOR=0x4000;

    //initialise the platform itself
	__platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();
    scheduler_run();
    return 0;
}
