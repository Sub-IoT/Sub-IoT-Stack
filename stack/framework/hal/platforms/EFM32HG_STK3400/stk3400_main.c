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

#include "scheduler.h"
#include "bootstrap.h"
#include "hwgpio.h"
#include "hwuart.h"
#include "hwleds.h"
#include "hwlcd.h"
#include "hwusb.h"
#include "efm32hg_mcu.h"
#include "hwdebug.h"
#include "platform.h"
#include "userbutton.h"
#include "platform_sensors.h"
#include "em_gpio.h"
#include <debug.h>
#include "console.h"
#include "framework_defs.h"

void __platform_init()
{
    __efm32hg_mcu_init();
    __gpio_init();
    __led_init();
    __lcd_init();

#ifdef PLATFORM_USE_USB_CDC
    __usb_init_cdc();
#endif

#if defined(FRAMEWORK_LOG_ENABLED) || defined(FRAMEWORK_SHELL_ENABLED)
    // framework does not need console, if app needs it app should init this
    console_init();
#endif

#ifdef USE_CC1101
    // configure the interrupt pins here, since hw_gpio_configure_pin() is MCU specific and not part of the common HAL API
    hw_gpio_configure_pin(CC1101_GDO0_PIN, true, gpioModeInput, 0); // TODO pull up or pull down to prevent floating
    // hw_gpio_configure_pin(CC1101_SPI_PIN_CS, false, gpioModePushPull, 1);
    //hw_gpio_configure_pin(CC1101_GDO2_PIN, true, gpioModeInput, 0) // TODO pull up or pull down to prevent floating // TODO not used for now
#endif
    __hw_debug_init();

    error_t err;
    err = hw_gpio_configure_pin(BUTTON0, true, gpioModeInput, 0); assert(err == SUCCESS); // TODO pull up or pull down to prevent floating
    err = hw_gpio_configure_pin(BUTTON1, true, gpioModeInput, 0); assert(err == SUCCESS); // TODO pull up or pull down to prevent floating

    // TODO: only for cc430?
    // __watchdog_init(); // TODO configure from cmake?
}

void __platform_post_framework_init()
{
    __ubutton_init();

#ifdef PLATFORM_EFM32HG_STK3400_LCD_ENABLED
    uint64_t id = hw_get_unique_id();
    // nano spec of newlib does not support 64bit ...
    lcd_write_string("%.8x", (uint32_t)(id >> 32));
    lcd_write_string("%.8x", (uint32_t)id);
#endif
}

int main()
{
    //initialise the platform itself
	__platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();
    scheduler_run();
    return 0;
}
