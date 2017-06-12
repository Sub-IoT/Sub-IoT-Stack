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
#endif
