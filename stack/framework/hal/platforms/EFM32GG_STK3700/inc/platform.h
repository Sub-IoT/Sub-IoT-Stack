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

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include "platform_defs.h"

#ifndef PLATFORM_EFM32GG_STK3700
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_EFM32GG_STK3700 to be defined
#endif

#define HW_USE_HFXO
#define HW_USE_LFXO

#include "efm32gg_chip.h"


/********************
 * LED DEFINITIONS *
 *******************/

#define HW_NUM_LEDS 2


//INT_HANDLER

/********************
 *  USB SUPPORT      *
 ********************/

#define USB_DEVICE

/********************
 * UART DEFINITIONS *
 *******************/

// console configuration
#define CONSOLE_UART        PLATFORM_EFM32GG_STK3700_CONSOLE_UART
#define CONSOLE_LOCATION    PLATFORM_EFM32GG_STK3700_CONSOLE_LOCATION
#define CONSOLE_BAUDRATE    PLATFORM_EFM32GG_STK3700_CONSOLE_BAUDRATE

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

#define NUM_USERBUTTONS 	2
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
#define SX127x_SPI_USART    1
#define SX127x_SPI_LOCATION 1
#define SX127x_SPI_PIN_CS   D3
#define SX127x_SPI_BAUDRATE 10000000
#define SX127x_DIO0_PIN C5
#define SX127x_RESET_PIN D4
#endif
#define HAS_LCD

#endif
