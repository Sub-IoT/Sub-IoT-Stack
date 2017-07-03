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

/*! \file stm32l0xx_pins.c
 *
 */


#include "hwgpio.h"
#include "stm32l0xx_hal_gpio.h"

enum
{
  GPIO_PORTA = 0,
  GPIO_PORTB,
  GPIO_PORTC,
  GPIO_PORTD,
  GPIO_PORTE,
};


pin_id_t const A0 = {.port = GPIO_PORTA, .pin = 0};
pin_id_t const A1 = {.port = GPIO_PORTA, .pin = 1};
pin_id_t const A2 = {.port = GPIO_PORTA, .pin = 2};
pin_id_t const A3 = {.port = GPIO_PORTA, .pin = 3};
pin_id_t const A4 = {.port = GPIO_PORTA, .pin = 4};
pin_id_t const A5 = {.port = GPIO_PORTA, .pin = 5};
pin_id_t const A6 = {.port = GPIO_PORTA, .pin = 6};
pin_id_t const A7 = {.port = GPIO_PORTA, .pin = 7};
pin_id_t const A8 = {.port = GPIO_PORTA, .pin = 8};
pin_id_t const A9 = {.port = GPIO_PORTA, .pin = 9};
pin_id_t const A10 = {.port = GPIO_PORTA, .pin = 10};
pin_id_t const A11 = {.port = GPIO_PORTA, .pin = 11};
pin_id_t const A12 = {.port = GPIO_PORTA, .pin = 12};
pin_id_t const A13 = {.port = GPIO_PORTA, .pin = 13};
pin_id_t const A14 = {.port = GPIO_PORTA, .pin = 14};
pin_id_t const A15 = {.port = GPIO_PORTA, .pin = 15};
// PORT B
pin_id_t const B0 = {.port = GPIO_PORTB, .pin = 0};
pin_id_t const B1 = {.port = GPIO_PORTB, .pin = 1};
pin_id_t const B2 = {.port = GPIO_PORTB, .pin = 2};
pin_id_t const B3 = {.port = GPIO_PORTB, .pin = 3};
pin_id_t const B4 = {.port = GPIO_PORTB, .pin = 4};
pin_id_t const B5 = {.port = GPIO_PORTB, .pin = 5};
pin_id_t const B6 = {.port = GPIO_PORTB, .pin = 6};
pin_id_t const B7 = {.port = GPIO_PORTB, .pin = 7};
pin_id_t const B8 = {.port = GPIO_PORTB, .pin = 8};
pin_id_t const B9 = {.port = GPIO_PORTB, .pin = 9};
pin_id_t const B10 = {.port = GPIO_PORTB, .pin = 10};
pin_id_t const B11 = {.port = GPIO_PORTB, .pin = 11};
pin_id_t const B12 = {.port = GPIO_PORTB, .pin = 12};
pin_id_t const B13 = {.port = GPIO_PORTB, .pin = 13};
pin_id_t const B14 = {.port = GPIO_PORTB, .pin = 14};
pin_id_t const B15 = {.port = GPIO_PORTB, .pin = 15};
//port C
pin_id_t const C0 = {.port = GPIO_PORTC, .pin = 0};
pin_id_t const C1 = {.port = GPIO_PORTC, .pin = 1};
pin_id_t const C2 = {.port = GPIO_PORTC, .pin = 2};
pin_id_t const C3 = {.port = GPIO_PORTC, .pin = 3};
pin_id_t const C4 = {.port = GPIO_PORTC, .pin = 4};
pin_id_t const C5 = {.port = GPIO_PORTC, .pin = 5};
pin_id_t const C6 = {.port = GPIO_PORTC, .pin = 6};
pin_id_t const C7 = {.port = GPIO_PORTC, .pin = 7};
pin_id_t const C8 = {.port = GPIO_PORTC, .pin = 8};
pin_id_t const C9 = {.port = GPIO_PORTC, .pin = 9};
pin_id_t const C10 = {.port = GPIO_PORTC, .pin = 10};
pin_id_t const C11 = {.port = GPIO_PORTC, .pin = 11};
pin_id_t const C12 = {.port = GPIO_PORTC, .pin = 12};
pin_id_t const C13 = {.port = GPIO_PORTC, .pin = 13};

//port D
pin_id_t const D2 = {.port = GPIO_PORTD, .pin = 2};



// PORT B
extern pin_id_t const B0;
extern pin_id_t const B1;
extern pin_id_t const B2;
extern pin_id_t const B3;
extern pin_id_t const B4;
extern pin_id_t const B5;
extern pin_id_t const B6;
extern pin_id_t const B7;
extern pin_id_t const B8;
extern pin_id_t const B9;
extern pin_id_t const B10;
extern pin_id_t const B13;
extern pin_id_t const B14;
extern pin_id_t const B15;
// port C
extern pin_id_t const C0;
extern pin_id_t const C1;
extern pin_id_t const C2;
extern pin_id_t const C3;
extern pin_id_t const C4;
extern pin_id_t const C5;
extern pin_id_t const C6;
extern pin_id_t const C7;
extern pin_id_t const C8;
extern pin_id_t const C12;
// port D
extern pin_id_t const D0;
extern pin_id_t const D1;
extern pin_id_t const D2;
extern pin_id_t const D3;
extern pin_id_t const D4;
extern pin_id_t const D5;
extern pin_id_t const D6;
extern pin_id_t const D7;
extern pin_id_t const D8;
extern pin_id_t const D9;
extern pin_id_t const D10;
extern pin_id_t const D13;
extern pin_id_t const D14;
extern pin_id_t const D15;
// port E
extern pin_id_t const E0;
extern pin_id_t const E1;
extern pin_id_t const E2;
extern pin_id_t const E3;
extern pin_id_t const E4;
extern pin_id_t const E5;
extern pin_id_t const E6;
extern pin_id_t const E7;
extern pin_id_t const E8;
extern pin_id_t const E9;
extern pin_id_t const E10;
extern pin_id_t const E13;
extern pin_id_t const E14;
extern pin_id_t const E15;
