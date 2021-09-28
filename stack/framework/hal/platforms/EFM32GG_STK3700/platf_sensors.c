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

#include "platform_sensors.h"
#include "hwgpio.h"
#include "hwadc.h"
#include "hwatomic.h"
#include "scheduler.h"
#include "em_gpio.h"
#include "em_adc.h"
#include "em_cmu.h"
#include <string.h>
#include <debug.h>

//TODO Lightsensor uses ACMP, LESENSE, PRS
//#define LIGHT_SENSOR_ENABLE_PORTPIN		D6
////#define LIGHT_SENSOR_ENABLE_PORT		gpioPortD
//#define LIGHT_SENSOR_READ_PORT			gpioPortC
//#define LIGHT_SENSOR_READ_PORTPIN		C6

static uint32_t temp_offset;

void initSensors()
{
}

//void lightsensor_init()
//{
//	/* Configure the drive strength of the ports for the light sensor. */
//	//GPIO_DriveModeSet(LIGHT_SENSOR_ENABLE_PORTPIN, gpioDriveModeStandard);
//	//GPIO_DriveModeSet(LIGHT_SENSOR_READ_PORT, gpioDriveModeStandard);
//
//	// LIGHT SENSOR
//	error_t err = hw_gpio_configure_pin(LIGHT_SENSOR_ENABLE_PORTPIN, false, gpioModeDisabled, 0);
//	err = hw_gpio_configure_pin(LIGHT_SENSOR_READ_PORTPIN, false, gpioModeDisabled, 0);
//	assert(err == SUCCESS);
//
//
//}
//
//void lightsensor_enable()
//{
//	hw_gpio_set(LIGHT_SENSOR_ENABLE_PORTPIN);
//}
//
//void lightsensor_dissable()
//{
//	hw_gpio_clr(LIGHT_SENSOR_ENABLE_PORTPIN);
//}
//
//uint32_t lightsensor_read()
//{
//
//}
