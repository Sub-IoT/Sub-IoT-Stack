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
#define USE_USB_CDC

/********************
 * UART DEFINITIONS *
 *******************/

#define UART_BAUDRATE PLATFORM_EFM32GG_STK3700_UART_BAUDRATE



#define UART_CHANNEL        UART0
#define UART_CLOCK          cmuClock_UART0
#define UART_ROUTE_LOCATION UART_ROUTE_LOCATION_LOC1

// UART0 location #1: PE0 and PE1
#define UART_PIN_TX         E0           // PE0
#define UART_PIN_RX         E1          // PE1


/********************
 * SPI DEFINITIONS *
 *******************/

/* SPI Channel configuration */
#define SPI_CHANNEL         USART1                      // SPI Channel
#define SPI_BAUDRATE        9600                    // SPI Frequency
#define SPI_CLOCK           cmuClock_USART1             // SPI Clock
#define SPI_ROUTE_LOCATION  USART_ROUTE_LOCATION_LOC1   // SPI GPIO Routing

/* SPI Ports and Pins for the selected route location above.
 * See the datasheet for the availiable routes and corresponding GPIOs */
#define SPI_PIN_MOSI        D0
#define SPI_PIN_MISO        D1
#define SPI_PIN_CLK         D2
#define SPI_PIN_CS          D3

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
#define BUTTON0				B9
#define BUTTON1				B10

// CC1101 PIN definitions
#ifdef USE_CC1101
    #define CC1101_GDO0_PIN C3
    #define CC1101_GDO2_PIN C4
#endif

#endif
