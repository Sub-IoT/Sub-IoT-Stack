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

/*! \file efm32gg_pins.c
 *
 *  \author daniel.vandenakker@uantwerpen.be
 *
 */


#include <em_gpio.h>
#include "hwgpio.h"
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

pin_id_t const C0 = {.port = gpioPortC, .pin = 0};
pin_id_t const C1 = {.port = gpioPortC, .pin = 1};
pin_id_t const C2 = {.port = gpioPortC, .pin = 2};
pin_id_t const C3 = {.port = gpioPortC, .pin = 3};
pin_id_t const C4 = {.port = gpioPortC, .pin = 4};
pin_id_t const C5 = {.port = gpioPortC, .pin = 5};
pin_id_t const C6 = {.port = gpioPortC, .pin = 6};
pin_id_t const C7 = {.port = gpioPortC, .pin = 7};
pin_id_t const C8 = {.port = gpioPortC, .pin = 8};
pin_id_t const C9 = {.port = gpioPortC, .pin = 9};
pin_id_t const C10 = {.port = gpioPortC, .pin = 10};
pin_id_t const C11 = {.port = gpioPortC, .pin = 11};
pin_id_t const C12 = {.port = gpioPortC, .pin = 12};
pin_id_t const C13 = {.port = gpioPortC, .pin = 13};
pin_id_t const C14 = {.port = gpioPortC, .pin = 14};
pin_id_t const C15 = {.port = gpioPortC, .pin = 15};

pin_id_t const D0 = {.port = gpioPortD, .pin = 0};
pin_id_t const D1 = {.port = gpioPortD, .pin = 1};
pin_id_t const D2 = {.port = gpioPortD, .pin = 2};
pin_id_t const D3 = {.port = gpioPortD, .pin = 3};
pin_id_t const D4 = {.port = gpioPortD, .pin = 4};
pin_id_t const D5 = {.port = gpioPortD, .pin = 5};
pin_id_t const D6 = {.port = gpioPortD, .pin = 6};
pin_id_t const D7 = {.port = gpioPortD, .pin = 7};
pin_id_t const D8 = {.port = gpioPortD, .pin = 8};
pin_id_t const D9 = {.port = gpioPortD, .pin = 9};
pin_id_t const D10 = {.port = gpioPortD, .pin = 10};
pin_id_t const D11 = {.port = gpioPortD, .pin = 11};
pin_id_t const D12 = {.port = gpioPortD, .pin = 12};
pin_id_t const D13 = {.port = gpioPortD, .pin = 13};
pin_id_t const D14 = {.port = gpioPortD, .pin = 14};
pin_id_t const D15 = {.port = gpioPortD, .pin = 15};

pin_id_t const E0 = {.port = gpioPortE, .pin = 0};
pin_id_t const E1 = {.port = gpioPortE, .pin = 1};
pin_id_t const E2 = {.port = gpioPortE, .pin = 2};
pin_id_t const E3 = {.port = gpioPortE, .pin = 3};
pin_id_t const E4 = {.port = gpioPortE, .pin = 4};
pin_id_t const E5 = {.port = gpioPortE, .pin = 5};
pin_id_t const E6 = {.port = gpioPortE, .pin = 6};
pin_id_t const E7 = {.port = gpioPortE, .pin = 7};
pin_id_t const E8 = {.port = gpioPortE, .pin = 8};
pin_id_t const E9 = {.port = gpioPortE, .pin = 9};
pin_id_t const E10 = {.port = gpioPortE, .pin = 10};
pin_id_t const E11 = {.port = gpioPortE, .pin = 11};
pin_id_t const E12 = {.port = gpioPortE, .pin = 12};
pin_id_t const E13 = {.port = gpioPortE, .pin = 13};
pin_id_t const E14 = {.port = gpioPortE, .pin = 14};
pin_id_t const E15 = {.port = gpioPortE, .pin = 15};

pin_id_t const F0 = {.port = gpioPortF, .pin = 0};
pin_id_t const F1 = {.port = gpioPortF, .pin = 1};
pin_id_t const F2 = {.port = gpioPortF, .pin = 2};
pin_id_t const F3 = {.port = gpioPortF, .pin = 3};
pin_id_t const F4 = {.port = gpioPortF, .pin = 4};
pin_id_t const F5 = {.port = gpioPortF, .pin = 5};
pin_id_t const F6 = {.port = gpioPortF, .pin = 6};
pin_id_t const F7 = {.port = gpioPortF, .pin = 7};
pin_id_t const F8 = {.port = gpioPortF, .pin = 8};
pin_id_t const F9 = {.port = gpioPortF, .pin = 9};
pin_id_t const F10 = {.port = gpioPortF, .pin = 10};
pin_id_t const F11 = {.port = gpioPortF, .pin = 11};
pin_id_t const F12 = {.port = gpioPortF, .pin = 12};
pin_id_t const F13 = {.port = gpioPortF, .pin = 13};
pin_id_t const F14 = {.port = gpioPortF, .pin = 14};
pin_id_t const F15 = {.port = gpioPortF, .pin = 15};
