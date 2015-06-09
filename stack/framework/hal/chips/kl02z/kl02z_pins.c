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

/*! \file kl02z_pins.c
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */


#include "MKL02Z4.h"
#include "hwgpio.h"

pin_id_t const A0 = {.port = (uint32_t)GPIOA, .pin = 0};
pin_id_t const A1 = {.port = (uint32_t)GPIOA, .pin = 1};
pin_id_t const A2 = {.port = (uint32_t)GPIOA, .pin = 2};
pin_id_t const A3 = {.port = (uint32_t)GPIOA, .pin = 3};
pin_id_t const A4 = {.port = (uint32_t)GPIOA, .pin = 4};
pin_id_t const A5 = {.port = (uint32_t)GPIOA, .pin = 5};
pin_id_t const A6 = {.port = (uint32_t)GPIOA, .pin = 6};
pin_id_t const A7 = {.port = (uint32_t)GPIOA, .pin = 7};

pin_id_t const B0 = {.port = (uint32_t)GPIOB, .pin = 0};
pin_id_t const B1 = {.port = (uint32_t)GPIOB, .pin = 1};
pin_id_t const B2 = {.port = (uint32_t)GPIOB, .pin = 2};
pin_id_t const B3 = {.port = (uint32_t)GPIOB, .pin = 3};
pin_id_t const B4 = {.port = (uint32_t)GPIOB, .pin = 4};
pin_id_t const B5 = {.port = (uint32_t)GPIOB, .pin = 5};
