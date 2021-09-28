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

/*! \file efm32hg_watchdog.c
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#include "hwwatchdog.h"
#include "em_wdog.h"
#include <assert.h>

void __watchdog_init()
{
    WDOG_Init_TypeDef wdog_init = WDOG_INIT_DEFAULT;
    WDOG_Init(&wdog_init);
}

void hw_watchdog_feed()
{
    WDOG_Feed();
}
