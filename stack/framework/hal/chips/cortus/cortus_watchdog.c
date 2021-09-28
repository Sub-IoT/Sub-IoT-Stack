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

/*! \file cortus_watchdog.c
 *
 *  \author soomee
 *
 */



#include "hwwatchdog.h"
#include "machine/wdt.h"



void __watchdog_init()
{
   wdt->key = 0x700edc33;
   //wdt->value = 1000;
   //wdt->value = 0x64000000; // 1600M cycles @ 6.25MHz (efm32gg: 256K cycles @ 1kHz)
   wdt->value = 0xc8000000; // 3200M cycles @ 12.5MHz (efm32gg: 256K cycles @ 1kHz)

   wdt->key = 0x700edc33;
   wdt->status = 1;

   wdt->key = 0x700edc33;
   wdt->restart = 1;

   wdt->key = 0x700edc33;
   wdt->sel_clk = 2; // 12.5MHz

   wdt->key = 0x700edc33;
   wdt->enable = 1;
}



void hw_watchdog_feed()
{
   wdt->key = 0x700edc33;
   wdt->restart = 1;
}
