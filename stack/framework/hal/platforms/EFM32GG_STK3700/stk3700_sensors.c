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
	void internalTempSensor_init(void);
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

void internalTempSensor_init(void)
{
	adc_calibrate();

	// Initialises ADC
	adc_init(adcReference1V25, adcInputSingleTemp, 100);


	/* This is a work around for Chip Rev.D Errata, Revision 0.6. */
	/* Check for product revision 16 and 17 and set the offset */
	/* for ADC0_TEMP_0_READ_1V25. */
	uint8_t prod_rev = (DEVINFO->PART & _DEVINFO_PART_PROD_REV_MASK) >> _DEVINFO_PART_PROD_REV_SHIFT;

	if( (prod_rev == 16) || (prod_rev == 17) )
	{
		temp_offset = 112;
	}
	else
	{
		temp_offset = 0;
	}
}

float hw_get_internal_temperature()
{
	//todo: take into account warmup time
	adc_start();

	/* Wait in EM1 for ADC to complete */
	while(!adc_ready){};
	adc_clear_interrupt();

	/* Read sensor value */
	/* According to rev. D errata ADC0_TEMP_0_READ_1V25 should be decreased */
	/* by the offset  but it is the same if ADC reading is increased - */
	/* reference manual 28.3.4.2. */
	uint32_t temp = adc_get_value() + temp_offset;
	return convertAdcToCelsius(temp);
}

float convertAdcToCelsius(int32_t adcSample)
{
  float temp;
  /* Factory calibration temperature from device information page. */
  float cal_temp_0 = (float)((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK)
                             >> _DEVINFO_CAL_TEMP_SHIFT);

  float cal_value_0 = (float)((DEVINFO->ADC0CAL2
                               & _DEVINFO_ADC0CAL2_TEMP1V25_MASK)
                              >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);

  /* Temperature gradient (from datasheet) */
  float t_grad = -6.27;

  temp = (cal_temp_0 - ((cal_value_0 - adcSample)  / t_grad));

  return temp;
}

