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

#ifndef PLATFORM_OCTA_GATEWAY
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_OCTA_MINI to be defined
#endif


#include "efm32lg_chip.h"


#define HW_USE_HFXO
/********************
 * UART DEFINITIONS *
 *******************/
// console configuration
#define CONSOLE_UART        2 //US0
#define CONSOLE_LOCATION    2 //loc2
#define CONSOLE_BAUDRATE    115200//PLATFORM_OCTA_GATEWAY_UART_BAUDRATE

/********************
 * LED DEFINITIONS *
 *******************/

#define HW_NUM_LEDS 3
#define LED0				A8
#define LED1				A9
#define LED2				A10

// CC1101 PIN definitions
#define CC1101_SPI_USART    1
#define CC1101_SPI_BAUDRATE 6000000
#define CC1101_SPI_LOCATION 1
#define CC1101_SPI_PIN_CS   D3
#define CC1101_GDO0_PIN     D6
#define CC1101_GDO2_PIN     C6

#ifdef SPI_USE_DMA

    /* DMA Channel configuration */
    #define DMA_CHANNEL_TX      0
    #define DMA_CHANNEL_RX      1
    #define DMA_CHANNELS        2
    #define DMA_REQ_RX          DMAREQ_USART1_RXDATAV
    #define DMA_REQ_TX          DMAREQ_USART1_TXBL
#endif


/**************************
 * USERBUTTON DEFINITIONS *
 *************************/

#define NUM_USERBUTTONS 	2
#define BUTTON0				A7
#define BUTTON1				A11

/*************************
 * DEBUG PIN DEFINITIONS *
 ************************/

#define DEBUG_PIN_NUM 0
//#define DEBUG0	0
//#define DEBUG1	0
//#define DEBUG2	0

#endif
