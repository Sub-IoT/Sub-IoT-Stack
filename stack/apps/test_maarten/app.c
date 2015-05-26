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

#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include <assert.h>
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>

#include "hwlcd.h"
#include "hwadc.h"


bool sensing = false;
uint16_t counter = 0;

#ifdef PLATFORM_EFM32GG_STK3700
#include "userbutton.h"
#include "platform_sensors.h"

void userbutton_callback(button_id_t button_id)
{
	log_print_string("Button PB%u pressed.", button_id);
	led_toggle(1);

}

#endif

void measureTemperature()
{
	float temp = tempsensor_read_celcius();

	int i = (int)(temp * 10);
	char string[8];
	snprintf(string, 8, "%2d,%1d%%C", (i/10), abs(i%10));
	lcd_write_string(string);

	log_print_string("Temperature %s", string);
}

void timer0_callback()
{
	char string[8];
	int i;

	led_toggle(0);
	timer_post_task_delay(&timer0_callback, TIMER_TICKS_PER_SEC);
	log_print_string("Toggled led %d", 0);

	measureTemperature();

	/*
	if (sensing)
	{
		lightsensor_dissable();
	}
	else
	{
		lightsensor_enable();
	}

	sensing = !sensing;
	*/
}

void timer1_callback()
{
	led_toggle(1);
	timer_post_task_delay(&timer1_callback, 0x0000FFFF + (uint32_t)100);
	log_print_string("Toggled led %d", 1);
}


void bootstrap()
{
	led_on(0);
	led_on(1);

	/*
	* When using a debugger it is practical to uncomment the following three
	* lines to force host to re-enumerate the device.
	*/
//	USBD_Disconnect();
//	USBTIMER_DelayMs(1000);
//	USBD_Connect();

	log_print_string("Device booted at time: %d\n", timer_get_counter_value());


	/* Enable peripheral clocks */


	internalTempSensor_init();

    sched_register_task(&timer0_callback);
    //sched_register_task(&timer1_callback);

    timer_post_task_delay(&timer0_callback, TIMER_TICKS_PER_SEC);
    //timer_post_task_delay(&timer1_callback, 0x0000FFFF + (uint32_t)100);

#ifdef PLATFORM_EFM32GG_STK3700
    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);
#endif


    led_off(0);
    led_off(1);

    measureTemperature();




}

