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
#include "hwusb.h"
#include "ezr32lg_mcu.h"
#include "hwdebug.h"
#include "hwwatchdog.h"
#include "platform.h"
#include "userbutton.h"
#include "em_gpio.h"
#include <debug.h>

#include "em_cmu.h"
#include "em_chip.h"


#include "console.h"


#include "bsp_trace.h"


void __platform_init()
{
	__ezr32lg_mcu_init();
    __gpio_init();
    __led_init();
    console_init();

    __hw_debug_init();

    __watchdog_init(); // TODO configure from cmake?
}

void __platform_post_framework_init()
{

#ifdef PLATFORM_USE_USB_CDC
    __usb_init_cdc();
#endif

    __ubutton_init();

}

int main(){
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
