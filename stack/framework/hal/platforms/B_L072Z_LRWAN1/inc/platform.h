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

#ifndef PLATFORM_B_L072Z_LRWAN1
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_NUCLEO_STM32L152 to be defined
#endif

#include <stm32l0xx_pins.h>
//#include <stm32l0xx_chip.h>


/********************
 * LED DEFINITIONS *
 *******************/

#define HW_NUM_LEDS 4
#define LED1 B5
#define LED2 A5
#define LED3 B6
#define LED4 B7
#define LED_GREEN 0

/********************
 * UART DEFINITIONS *
 *******************/

// console configuration
#define CONSOLE_UART        PLATFORM_B_L072Z_LRWAN1_CONSOLE_UART
#define CONSOLE_BAUDRATE    PLATFORM_B_L072Z_LRWAN1_CONSOLE_BAUDRATE
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
#define BUTTON0                 B2

#ifdef USE_SX127X
  // TODO tmp
  #define SX127x_SPI_INDEX  0
  #define SX127x_SPI_LOCATION 0
  #define SX127x_SPI_PIN_CS  A15
  #define SX127x_SPI_BAUDRATE 8000000
  #define SX127x_DIO0_PIN B4
  #define SX127x_DIO1_PIN B1
  #ifdef PLATFORM_SX127X_USE_DIO3_PIN
    #define SX127x_DIO3_PIN C13
  #endif
  #ifdef PLATFORM_SX127X_USE_RESET_PIN
    #define SX127x_RESET_PIN C0
  #endif
#endif

#define PLATFORM_USE_ABZ // this platform is based on the Murata ABZ module
// Antenna switching uses 3 pins on murata ABZ module
#define ABZ_ANT_SW_RX_PIN A1
#define ABZ_ANT_SW_TX_PIN C2
#define ABZ_ANT_SW_PA_BOOST_PIN C1

#define PLATFORM_NUM_TIMERS 1

#endif
