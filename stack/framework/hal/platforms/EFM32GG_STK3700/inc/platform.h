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

#include "platform_defs.h"

#ifndef PLATFORM_EFM32GG_STK3700
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_EFM32GG_STK3700 to be defined
#endif

#define HW_USE_HFXO
#define HW_USE_LFXO

#include "efm32gg_chip.h"




//INT_HANDLER

/********************
 *  USB SUPPORT      *
 ********************/

#define USB_DEVICE

/*************************
 * DEBUG PIN DEFINITIONS *
 ************************/

#define DEBUG_PIN_NUM 3
#define DEBUG0	D5
#define DEBUG1	D6
#define DEBUG2	D7

/**************************
 * USERBUTTON DEFINITIONS *
 *************************/

#define BUTTON0				B9
#define BUTTON1				B10

// CC1101 PIN definitions
#ifdef USE_CC1101
#define CC1101_SPI_USART    1
#define CC1101_SPI_BAUDRATE 6000000
#define CC1101_SPI_LOCATION 1
#define CC1101_SPI_PIN_CS   D3
#define CC1101_GDO0_PIN     C3
#define CC1101_GDO2_PIN     C4
#endif

#ifdef USE_SX127X
#define SX127x_SPI_INDEX    1
#define SX127x_SPI_LOCATION 1
#define SX127x_SPI_PIN_CS   D3
#define SX127x_SPI_BAUDRATE 10000000
#define SX127x_DIO0_PIN C5
#define SX127x_DIO1_PIN B12
#ifdef PLATFORM_SX127X_USE_DIO3_PIN
  #define SX127x_DIO3_PIN C3
#endif
#ifdef PLATFORM_SX127X_USE_RESET_PIN
  #define SX127x_RESET_PIN D4
#endif
#endif
#define HAS_LCD

#endif
