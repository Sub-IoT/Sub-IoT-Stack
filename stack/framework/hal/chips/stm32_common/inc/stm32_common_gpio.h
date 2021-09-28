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

#ifndef __STM32_COMMON_GPIO_H_
#define __STM32_COMMON_GPIO_H_

#include "hwgpio.h"
#include "stm32_device.h"

#define PLATFORM_NUM_TIMERS 1

#define GPIO_PORT(pin_id) ((pin_id >> 10) & 0xF)
#define GPIO_PIN(pin_id)  (pin_id & 0xF)
#define GPIO_PORT_MASK(pin_id)    ((GPIO_PORT(pin_id) == (0))? 0U :\
                                      (GPIO_PORT(pin_id) == (1))? 1U :\
                                      (GPIO_PORT(pin_id) == (2))? 2U :\
                                      (GPIO_PORT(pin_id) == (3))? 3U :\
                                      (GPIO_PORT(pin_id) == (4))? 4U :\
                                      (GPIO_PORT(pin_id) == (7))? 5U : 6U)

/* \brief Implementation of hw_gpio_configure_pin
 *
 * TODO
 */
__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint32_t mode, unsigned int out);
__LINK_C error_t hw_gpio_configure_pin_stm(pin_id_t pin_id, GPIO_InitTypeDef* init_options);


#endif //__STM32_COMMON_GPIO_H_
