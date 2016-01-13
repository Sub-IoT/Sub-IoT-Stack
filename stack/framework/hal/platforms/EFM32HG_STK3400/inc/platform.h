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

#ifndef PLATFORM_EFM32HG_STK3400
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_EFM32GG_STK3700 to be defined
#endif


#include "efm32hg_chip.h"

/********************
 *      X-TAL       *
 *******************/

#define HW_USE_HFXO
#define HW_USE_LFXO

/********************
 * LED DEFINITIONS *
 *******************/

#define HW_NUM_LEDS 2


//INT_HANDLER

/********************
 *  USB SUPPORT      *
 ********************/

#define USB_DEVICE
//#define USE_USB_CDC - defined in cmake

/********************
 * UART DEFINITIONS *
 *******************/

#define UART_ENABLED

#define UART_BAUDRATE PLATFORM_EFM32HG_STK3400_UART_BAUDRATE



#define UART_CHANNEL        USART1
#define UART_CLOCK          cmuClock_USART1
#define UART_ROUTE_LOCATION USART_ROUTE_LOCATION_LOC2


#define UART_PIN_TX         D7
#define UART_PIN_RX         D6


/********************
 * SPI DEFINITIONS *
 *******************/

/* SPI Channel configuration */
#define SPI_CHANNEL         USART0                      // SPI Channel
#define SPI_BAUDRATE        6000000                    // SPI Frequency
#define SPI_CLOCK           cmuClock_USART0             // SPI Clock
#define SPI_ROUTE_LOCATION  USART_ROUTE_LOCATION_LOC0   // SPI GPIO Routing

/* SPI Ports and Pins for the selected route location above.
 * See the datasheet for the availiable routes and corresponding GPIOs */
#define SPI_PIN_MOSI        E10
#define SPI_PIN_MISO        E11
#define SPI_PIN_CLK         E12
#define SPI_PIN_CS          E13

#ifdef SPI_USE_DMA

    /* DMA Channel configuration */
    #define DMA_CHANNEL_TX      0
    #define DMA_CHANNEL_RX      1
    #define DMA_CHANNELS        2
    #define DMA_REQ_RX          DMAREQ_USART1_RXDATAV
    #define DMA_REQ_TX          DMAREQ_USART1_TXBL
#endif

/*************************
 * DEBUG PIN DEFINITIONS *
 ************************/

#define DEBUG_PIN_NUM 4
#define DEBUG0	D4
#define DEBUG1	D5
#define DEBUG2	D6
#define DEBUG3	D7

/**************************
 * USERBUTTON DEFINITIONS *
 *************************/

#define NUM_USERBUTTONS 	2
#define BUTTON0				C9
#define BUTTON1				C10

// CC1101 PIN definitions
#ifdef USE_CC1101
    #define CC1101_GDO0_PIN C1
    #define CC1101_GDO2_PIN C2
#endif

#define HAS_LCD

#endif
