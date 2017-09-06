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

#ifndef PLATFORM_NUCLEO_L073RZ
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_NUCLEO_STM32L152 to be defined
#endif

#include <stm32l0xx_chip.h>


/********************
 * LED DEFINITIONS *
 *******************/

#define HW_NUM_LEDS 1
#define LED0 A5
#define LED_GREEN 0


/********************
 * UART DEFINITIONS *
 *******************/

// console configuration
#define CONSOLE_UART        PLATFORM_NUCLEO_L073RZ_CONSOLE_UART
#define CONSOLE_BAUDRATE    PLATFORM_NUCLEO_L073RZ_CONSOLE_BAUDRATE
#define CONSOLE_LOCATION	0

/*************************
 * DEBUG PIN DEFINITIONS *
 ************************/

// #define DEBUG_PIN_NUM 4
// #define DEBUG0	D4
// #define DEBUG1	D5
// #define DEBUG2	D6
// #define DEBUG3	D7

/**************************
 * USERBUTTON DEFINITIONS *
 *************************/

#define NUM_USERBUTTONS 	1
#define BUTTON0				C13

#ifdef USE_SX127X
  #define SX127x_SPI_INDEX    0
  #define SX127x_SPI_LOCATION 0
  #define SX127x_SPI_PIN_CS   B6
  #define SX127x_SPI_BAUDRATE 8000000
  #define SX127x_DIO0_PIN A10
  #define SX127x_DIO1_PIN B3
  #ifdef PLATFORM_SX127X_USE_DIO3_PIN
    #define SX127x_DIO3_PIN B4
  #endif
  #ifdef PLATFORM_SX127X_USE_RESET_PIN
    #define SX127x_RESET_PIN A0
  #endif
#endif

#define PLATFORM_NUM_TIMERS 1

#endif
