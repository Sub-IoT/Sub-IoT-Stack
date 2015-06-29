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
#include "d7ap_stack.h"
#include "fs.h"

#ifndef PLATFORM_EFM32GG_STK3700
	#error Mismatch between the configured platform and the actual platform. Expected PLATFORM_EFM32GG_STK3700 to be defined
#endif

#include "userbutton.h"
#include "platform_sensors.h"
#include "platform_lcd.h"

static int16_t temperature = 0;

void userbutton_callback(button_id_t button_id)
{
	log_print_string("Button PB%u pressed.", button_id);
	led_toggle(button_id);

	char string[9];
	snprintf(string, 7, "Btn %u", button_id);
	lcd_write_string(string);

	fs_write_file(0x40, 2, (uint8_t*)&button_id, 1); // File 0x40 is configured to use D7AActP trigger an ALP action which broadcasts this file data on Access Class 0
}

void measureTemperature()
{
	float temp = tempsensor_read_celcius();

	int i = (int)(temp * 10);
	lcd_write_temperature(i*10, 1);
	temperature = temp * 100;
	
	log_print_string("Temperature %2d,%2d C", (i/10), abs(i%10));
}

void execute_sensor_measurement()
{
	led_toggle(0);
	timer_post_task_delay(&execute_sensor_measurement, TIMER_TICKS_PER_SEC * 5);

	measureTemperature();

	fs_write_file(0x40, 0, (uint8_t*)&temperature, 2); // File 0x40 is configured to use D7AActP trigger an ALP action which broadcasts this file data on Access Class 0
}

void bootstrap()
{
	log_print_string("Device booted at time: %d\n", timer_get_counter_value());

    d7ap_stack_init();

	internalTempSensor_init();
	measureTemperature();

    ubutton_register_callback(0, &userbutton_callback);
    ubutton_register_callback(1, &userbutton_callback);

    sched_register_task((&execute_sensor_measurement));
    timer_post_task_delay(&execute_sensor_measurement, TIMER_TICKS_PER_SEC * 5);

    lcd_write_string("DASH7");
}

