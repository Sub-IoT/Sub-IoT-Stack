/* OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 Christophe VG <contact@christophe.vg>
 * Copyright 2017 University of Antwerp
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

/*! \file platform.h
 *
 *  \author contact@christophe.vg
 *
 */

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include "platform_defs.h"

#ifndef PLATFORM_EZRPI
  #error Mismatch between the configured platform and the actual platform. Expected PLATFORM_EZRPi to be defined
#endif

#include <ezr32lg_chip.h>

#define HW_USE_HFXO
#define HW_USE_LFXO

// LED

#define HW_NUM_LEDS         1
#define LED_BLUE            0
#define LED0	              E2

// console, baudrate can be configured externally
#define CONSOLE_UART        0
#define CONSOLE_LOCATION    1
#define CONSOLE_BAUDRATE    PLATFORM_EZRPI_CONSOLE_BAUDRATE

// SPI RF

#define si4455_GDO0_PIN     A15
#define si4455_GDO1_PIN     E14
#define si4455_SDN_PIN      E8

#endif
