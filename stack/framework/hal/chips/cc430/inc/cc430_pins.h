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

#ifndef __CC430_PINS_H_
#define __CC430_PINS_H_

#include "hwgpio.h"

//GPIO port/pin definitions for the CC430F513x

//port 1
extern pin_id_t const P1_0;
extern pin_id_t const P1_1;
extern pin_id_t const P1_2;
extern pin_id_t const P1_3;
extern pin_id_t const P1_4;
extern pin_id_t const P1_5;
extern pin_id_t const P1_6;
extern pin_id_t const P1_7;

//port2
extern pin_id_t const P2_0;
extern pin_id_t const P2_1;
extern pin_id_t const P2_2;
extern pin_id_t const P2_3;
extern pin_id_t const P2_4;
extern pin_id_t const P2_5;
extern pin_id_t const P2_6;
extern pin_id_t const P2_7;

//port3
extern pin_id_t const P3_0;
extern pin_id_t const P3_1;
extern pin_id_t const P3_2;
extern pin_id_t const P3_3;
extern pin_id_t const P3_4;
extern pin_id_t const P3_5;
extern pin_id_t const P3_6;
extern pin_id_t const P3_7;

//port5
extern pin_id_t const P5_0;
extern pin_id_t const P5_1;

//portJ
extern pin_id_t const PJ_0;
extern pin_id_t const PJ_1;
extern pin_id_t const PJ_2;
extern pin_id_t const PJ_3;

#endif //__CC430_PINS_H_
