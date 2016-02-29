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

/* \file
 *
 * Interface to the sensors of the gecko board. This file is NOT a part of the
 * 'HAL' interface since not every platform will have sensors
 *
 * The EFM32GG 'Giant Gecko' board has the following sensors
 * - Ambient lightsensor
 *
 */
#ifndef __PLATFORM_SENSORS_H_
#define __PLATFORM_SENSORS_H_

#include "link_c.h"
#include "types.h"
#include "platform.h"


__LINK_C void initSensors();

__LINK_C  void getHumidityAndTemperature(uint32_t *rhData, int32_t *tData);
/*! \brief Initialises the sensors
 * 			- I/O for light sensor
 */
//__LINK_C void lightsensor_init();
//__LINK_C void lightsensor_enable();
//__LINK_C void lightsensor_dissable();
//__LINK_C uint32_t lightsensor_read();


/*! \brief Initializes ADC to measure the internal temperature
 */
__LINK_C void internalTempSensor_init();

/*! \brief Measures the interternal temperature using the ADC and converts it to Celcius
 *	\note It expects the user to initialise the ADC using internalTempSensor_init()
 */
__LINK_C float tempsensor_read_celcius();

/*! \brief Convert a temperature ADC sample to temperature taking into account the factory calibration
 * 	\note See section 2.3.4 in the reference manual for details on this calculation
 *
 * 	\param adcSample Raw value from ADC to be converted to Celsius
 * 	\return The temperature in degrees Celsius.
 */
__LINK_C float convertAdcToCelsius(int32_t adcSample);

#endif
