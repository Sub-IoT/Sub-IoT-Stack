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
#include "ezr32lg_mcu.h"

#ifndef PLATFORM_EZR32LG_WSTK6200A
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_EZR32LG_WSTK6200A to be defined
#endif


#include <ezr32lg_chip.h>

#define HW_USE_HFXO
#define HW_USE_LFXO

/********************
 * LED DEFINITIONS *
 *******************/

#define LED0	PIN(5, 6)
#define	LED1	PIN(5, 7)

/********************
 *  USB SUPPORT      *
 ********************/

#define USB_DEVICE

/********************
 * UART DEFINITIONS *
 *******************/

// UART0 location #1: PE0 and PE1
//#define UART_PIN_TX         E0           // PE0
//#define UART_PIN_RX         E1          // PE1

/*******************
 *   VCOM          *
 ******************/

#define VCOM_ENABLE	PIN(0, 12)
// USE CONSOLE_UART 3 / LOCATION 1



/*************************
 * DEBUG PIN DEFINITIONS *
 ************************/

#define DEBUG_PIN_NUM 4
#define DEBUG0	PIN(2, 6)      // P0
#define DEBUG1	PIN(2, 7)      // P2
#define DEBUG2	PIN(1, 11)     // P8
#define DEBUG3	PIN(5, 3)      // P10

/**************************
 * USERBUTTON DEFINITIONS *
 *************************/

#define BUTTON0				PIN(4, 3)
#define BUTTON1				PIN(4, 2)



#define HAS_LCD

#endif
