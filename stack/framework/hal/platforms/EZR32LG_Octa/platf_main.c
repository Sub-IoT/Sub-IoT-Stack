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
#include "led.h"
#include "hwusb.h"
#include "ezr32lg_mcu.h"
#include "hwdebug.h"
#include "hwwatchdog.h"
#include "platform.h"
#include "button.h"
#include "em_gpio.h"
#include <debug.h>

#include "em_cmu.h"
#include "em_chip.h"


#include "console.h"


#include "bsp_trace.h"

#if defined(USE_SX127X) && defined(PLATFORM_SX127X_USE_RESET_PIN)
static void reset_sx127x()
{
  error_t e;
  e = hw_gpio_configure_pin(SX127x_RESET_PIN, false, gpioModePushPull, 0); assert(e == SUCCESS); // TODO platform specific
  hw_busy_wait(150);
  e = hw_gpio_configure_pin(SX127x_RESET_PIN, false, gpioModeInputPull, 1); assert(e == SUCCESS); // TODO platform specific
  hw_busy_wait(6000);
}
#endif

void __platform_init()
{
	__ezr32lg_mcu_init();
    __gpio_init();

#ifdef USE_SX127X
    // configure the interrupt pins here, since hw_gpio_configure_pin() is MCU
    // specific and not part of the common HAL API
    hw_gpio_configure_pin(SX127x_DIO0_PIN, true, gpioModeInputPull, 0);
    hw_gpio_configure_pin(SX127x_DIO1_PIN, true, gpioModeInputPull, 0);
#ifdef PLATFORM_SX127X_USE_DIO3_PIN
    hw_gpio_configure_pin(SX127x_DIO3_PIN, true, gpioModeInputPull, 0);
#endif
#ifdef PLATFORM_SX127X_USE_RESET_PIN
    hw_gpio_configure_pin(SX127x_RESET_PIN, false, gpioModePushPull, 1);
    reset_sx127x()
#endif
#endif

    __hw_debug_init();

    __watchdog_init(); // TODO configure from cmake?
}

void __platform_post_framework_init()
{
    __ubutton_init();
    led_init();

#ifdef PLATFORM_USE_USB_CDC
    __usb_init_cdc();
#endif
}

int main()
{
	//BSP_TraceProfilerSetup();
    // Only when using bootloader
	//SCB->VTOR=0x4000;

	//activate VCOM

    //initialise the platform itself
	__platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();
    scheduler_run();
    return 0;
}
