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

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include "stm32_common_mcu.h"
#include "platform_defs.h"

#ifndef PLATFORM_NUCLEO_L152RE
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_NUCLEO_L152RE to be defined
#endif

#ifdef USE_SX127X
  #define SX127x_SPI_INDEX    0
  #define SX127x_SPI_PIN_CS PIN(1, 6)
  #define SX127x_SPI_BAUDRATE 8000000
  #define SX127x_DIO0_PIN PIN(0, 10)
  #define SX127x_DIO1_PIN PIN(1, 3)
  #ifdef PLATFORM_SX127X_USE_DIO3_PIN
    #define SX127x_DIO3_PIN PIN(1, 4)
  #endif
  #ifdef PLATFORM_SX127X_USE_RESET_PIN
    #define SX127x_RESET_PIN PIN(0, 0)
  #endif
  #ifdef PLATFORM_SX127X_USE_MANUAL_RXTXSW_PIN
    #define SX127x_MANUAL_RXTXSW_PIN PIN(2, 1)
  #endif
#endif

#define PLATFORM_NUM_TIMERS 1

#endif
