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

/*! \file stm32_common_watchdog.c
 *  \author glenn.ergeerts@uantwerpen.be
 */

#include "hwwatchdog.h"
#include "stm32_device.h"

#include <assert.h>

#define WATCHDOG_TIMEOUT_SECONDS 18 

static IWDG_HandleTypeDef iwdg_hal_hadle = {.Instance = NULL};
void __watchdog_init()
{
    IWDG_InitTypeDef iwdg_init_options;
    /*
     * Settings below disable window comparison (window exuals the reload value)
     * the Watchdog will reset the MCU after: 256*4096/37000 ~ 28s
     */
    iwdg_init_options.Prescaler = IWDG_PRESCALER_256;
    iwdg_init_options.Reload = 0xFFF;
#ifdef STM32L0
    iwdg_init_options.Window = 0xFFF;
#endif
    iwdg_hal_hadle.Init = iwdg_init_options;
    iwdg_hal_hadle.Instance = IWDG;
    HAL_IWDG_Init(&iwdg_hal_hadle);
}

void hw_watchdog_feed()
{
    HAL_IWDG_Refresh(&iwdg_hal_hadle);
}

uint8_t hw_watchdog_get_timeout(void)
{
    return WATCHDOG_TIMEOUT_SECONDS;
}