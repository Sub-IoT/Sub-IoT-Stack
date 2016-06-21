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

#ifndef __CORTUS_PINS_H_
#define __CORTUS_PINS_H_
#include "hwgpio.h"
// GPIO1 for led and cc1101
extern pin_id_t const A0;   // GDO0
extern pin_id_t const A1;   // GDO2
extern pin_id_t const A2;   // 
extern pin_id_t const A3;   // 
extern pin_id_t const A4;   // 
extern pin_id_t const A5;   // 
extern pin_id_t const A6;   // 
extern pin_id_t const A7;   // 
extern pin_id_t const A8;   // 
extern pin_id_t const A9;   // 
extern pin_id_t const A10;  //     
extern pin_id_t const A11;  //     
extern pin_id_t const A12;  //     
extern pin_id_t const A13;  //     
extern pin_id_t const A14;  //     
extern pin_id_t const A15;  //     
extern pin_id_t const A16;  // led[0]    
extern pin_id_t const A17;  // led[1]    
extern pin_id_t const A18;  // led[2]    
extern pin_id_t const A19;  // led[3]    
extern pin_id_t const A20;  // led[4]    
extern pin_id_t const A21;  // led[5]    
extern pin_id_t const A22;  // led[6]    
extern pin_id_t const A23;  // led[7]    
extern pin_id_t const A24;  //     
extern pin_id_t const A25;  //     
extern pin_id_t const A26;  //     
extern pin_id_t const A27;  //     
extern pin_id_t const A28;  //     
extern pin_id_t const A29;  //     
extern pin_id_t const A30;  //     
extern pin_id_t const A31;  //     

// SPI2 and buttons
extern pin_id_t const B0;   // sclk (output)
extern pin_id_t const B1;   // csn  (output)
extern pin_id_t const B2;   // so   (input)
extern pin_id_t const B3;   // si   (output)
extern pin_id_t const B4;   // available to use
extern pin_id_t const B5;   // available to use
extern pin_id_t const B6;   // available to use
extern pin_id_t const B7;   // available to use
extern pin_id_t const B8;   // available to use
extern pin_id_t const B9;   // BUTTON0
extern pin_id_t const B10;   // BUTTON1
extern pin_id_t const B11;   // available to use
extern pin_id_t const B12;   // available to use
extern pin_id_t const B13;   // available to use
extern pin_id_t const B14;   // available to use
extern pin_id_t const B15;   // available to use

#endif //__CORTUS_PINS_H_
