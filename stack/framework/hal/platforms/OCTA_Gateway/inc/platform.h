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
#include "efm32lg_mcu.h"

#ifndef PLATFORM_OCTA_GATEWAY
    #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_OCTA_MINI to be defined
#endif


#include "efm32lg_chip.h"


#define HW_USE_HFXO

/********************
 * LED DEFINITIONS *
 *******************/

#define LED0				PIN(0,8)
#define LED1				PIN(0,9)
#define LED2				PIN(1,11)//A10
#define LED_GREEN           1
#define LED_ORANGE          2
#define LED_RED             0


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

#define BUTTON0				PIN(3, 7)
#define BUTTON1				PIN(2, 10)

/*************************
 * DEBUG PIN DEFINITIONS *
 ************************/

#define DEBUG_PIN_NUM 0
//#define DEBUG0	0
//#define DEBUG1	0
//#define DEBUG2	0

#endif
