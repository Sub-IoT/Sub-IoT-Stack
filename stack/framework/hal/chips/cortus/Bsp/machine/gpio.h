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

#ifndef _GPIO_H
#define _GPIO_H
#include <machine/sfradr.h>

typedef struct Gpio
{
    volatile unsigned out;
    volatile unsigned in;
    volatile unsigned dir;
    volatile unsigned old_in;
    volatile unsigned mask;
    volatile unsigned level_sel;
    volatile unsigned rs_edge_sel;
    volatile unsigned fl_edge_sel;
    volatile unsigned edge;
} Gpio;

/** GPIO ports identificator. */
typedef enum
{
  gpioPortA = SFRADR_GPIO1, /**< Port A */
  gpioPortB = SFRADR_GPIO2, /**< Port B */
} GPIO_Port_TypeDef;

/** Pin mode. For more details on each mode, please refer to the APS peripheral reference manual. */

#define HAVE_GPIO_MODE_T
typedef enum
{
  /** Input  without pull up*/
  gpioModeInput = 0,
  /** Input  with pull up*/
  gpioModeInputPull = 0xFF,
  /** output  with pull up*/
  gpioModePushPull = 1,
} GPIO_Mode_TypeDef;

#ifdef __APS__
#define gpio1 ((Gpio *)SFRADR_GPIO1)
#define gpio2 ((Gpio *)SFRADR_GPIO2)
#else
extern Gpio __gpio1;
extern Gpio __gpio2;
#define gpio1 (&__gpio1)
#define gpio2 (&__gpio2)
#endif
#endif
