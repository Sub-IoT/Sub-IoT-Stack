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

/*! \file cortus_pins.c
 *
 *  \author junghoon
 *
 */


//#include <em_gpio.h>
#include "hwgpio.h"
#include "machine/gpio.h"

//Definition of the pin_id's for the GPIO ports of the EFM32gg
pin_id_t const A0 = {.port = gpioPortA, .pin = 0};
pin_id_t const A1 = {.port = gpioPortA, .pin = 1};
pin_id_t const A2 = {.port = gpioPortA, .pin = 2};
pin_id_t const A3 = {.port = gpioPortA, .pin = 3};
pin_id_t const A4 = {.port = gpioPortA, .pin = 4};
pin_id_t const A5 = {.port = gpioPortA, .pin = 5};
pin_id_t const A6 = {.port = gpioPortA, .pin = 6};
pin_id_t const A7 = {.port = gpioPortA, .pin = 7};
pin_id_t const A8 = {.port = gpioPortA, .pin = 8};
pin_id_t const A9 = {.port = gpioPortA, .pin = 9};
pin_id_t const A10 = {.port = gpioPortA, .pin = 10};
pin_id_t const A11 = {.port = gpioPortA, .pin = 11};
pin_id_t const A12 = {.port = gpioPortA, .pin = 12};
pin_id_t const A13 = {.port = gpioPortA, .pin = 13};
pin_id_t const A14 = {.port = gpioPortA, .pin = 14};
pin_id_t const A15 = {.port = gpioPortA, .pin = 15};
pin_id_t const A16 = {.port = gpioPortA, .pin = 16};
pin_id_t const A17 = {.port = gpioPortA, .pin = 17};
pin_id_t const A18 = {.port = gpioPortA, .pin = 18};
pin_id_t const A19 = {.port = gpioPortA, .pin = 19};
pin_id_t const A20 = {.port = gpioPortA, .pin = 20};
pin_id_t const A21 = {.port = gpioPortA, .pin = 21};
pin_id_t const A22 = {.port = gpioPortA, .pin = 22};
pin_id_t const A23 = {.port = gpioPortA, .pin = 23};
pin_id_t const A24 = {.port = gpioPortA, .pin = 24};
pin_id_t const A25 = {.port = gpioPortA, .pin = 25};
pin_id_t const A26 = {.port = gpioPortA, .pin = 26};
pin_id_t const A27 = {.port = gpioPortA, .pin = 27};
pin_id_t const A28 = {.port = gpioPortA, .pin = 28};
pin_id_t const A29 = {.port = gpioPortA, .pin = 29};
pin_id_t const A30 = {.port = gpioPortA, .pin = 30};
pin_id_t const A31 = {.port = gpioPortA, .pin = 31};

#if 0
pin_id_t const B0 = {.port = gpioPortB, .pin = 0};
pin_id_t const B1 = {.port = gpioPortB, .pin = 1};
pin_id_t const B2 = {.port = gpioPortB, .pin = 2};
pin_id_t const B3 = {.port = gpioPortB, .pin = 3};
pin_id_t const B4 = {.port = gpioPortB, .pin = 4};
pin_id_t const B5 = {.port = gpioPortB, .pin = 5};
pin_id_t const B6 = {.port = gpioPortB, .pin = 6};
pin_id_t const B7 = {.port = gpioPortB, .pin = 7};
pin_id_t const B8 = {.port = gpioPortB, .pin = 8};
pin_id_t const B9 = {.port = gpioPortB, .pin = 9};
pin_id_t const B10 = {.port = gpioPortB, .pin = 10};
pin_id_t const B11 = {.port = gpioPortB, .pin = 11};
pin_id_t const B12 = {.port = gpioPortB, .pin = 12};
pin_id_t const B13 = {.port = gpioPortB, .pin = 13};
pin_id_t const B14 = {.port = gpioPortB, .pin = 14};
pin_id_t const B15 = {.port = gpioPortB, .pin = 15};
pin_id_t const B16 = {.port = gpioPortB, .pin = 16};
pin_id_t const B17 = {.port = gpioPortB, .pin = 17};
pin_id_t const B18 = {.port = gpioPortB, .pin = 18};
pin_id_t const B19 = {.port = gpioPortB, .pin = 19};
pin_id_t const B20 = {.port = gpioPortB, .pin = 20};
pin_id_t const B21 = {.port = gpioPortB, .pin = 21};
pin_id_t const B22 = {.port = gpioPortB, .pin = 22};
pin_id_t const B23 = {.port = gpioPortB, .pin = 23};
pin_id_t const B24 = {.port = gpioPortB, .pin = 24};
pin_id_t const B25 = {.port = gpioPortB, .pin = 25};
pin_id_t const B26 = {.port = gpioPortB, .pin = 26};
pin_id_t const B27 = {.port = gpioPortB, .pin = 27};
pin_id_t const B28 = {.port = gpioPortB, .pin = 28};
pin_id_t const B29 = {.port = gpioPortB, .pin = 29};
pin_id_t const B30 = {.port = gpioPortB, .pin = 30};
pin_id_t const B31 = {.port = gpioPortB, .pin = 31};
#endif

