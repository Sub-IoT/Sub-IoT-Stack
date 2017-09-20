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

#ifndef __STM32L0xx_GPIO_H_
#define __STM32L0xx_GPIO_H_

#include "stm32l0xx_hal_gpio.h"

#define PLATFORM_NUM_TIMERS 1

#define GPIO_PORT(pin_id) ((pin_id >> 10) & 0xF)
#define GPIO_PIN(pin_id)  (pin_id & 0xF)

/* \brief Implementation of hw_gpio_configure_pin
 *
 * TODO
 */
__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint32_t mode, unsigned int out);
__LINK_C error_t hw_gpio_configure_pin_stm(pin_id_t pin_id, GPIO_InitTypeDef* init_options);


#endif //__STM32L0xx_GPIO_H_
