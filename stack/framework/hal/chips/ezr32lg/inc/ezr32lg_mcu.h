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

#ifndef __EZR32WG_MCU_H_
#define __EZR32WG_MCU_H_

#include "em_int.h"
#include "hwgpio.h"

#define PIN(port, pin)  ((port << 4) | pin)
#define PIN_UNDEFINED 0xFFFFFFFF

enum
{
    GPIO_PORTA = 0,
    GPIO_PORTB,
    GPIO_PORTC,
    GPIO_PORTD,
    GPIO_PORTE,
    GPIO_PORTF,
};


typedef struct {
    uint32_t location;
    pin_id_t mosi;
    pin_id_t miso;
    pin_id_t clk;
} spi_port_t;


//General definition for EZR32 interrupt handlers
#define INT_HANDLER(handler)	static inline void __ ## handler();\
    void handler(){INT_Disable();__ ## handler();INT_Enable();}\
    static inline void __ ## handler()

void __ezr32lg_mcu_init();

#endif
