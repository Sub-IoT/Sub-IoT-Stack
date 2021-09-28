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

/*! \file cortus_system.c
 *
 *  \author soomee
 *
 */

#include "hwsystem.h"


void hw_enter_lowpower_mode(uint8_t mode)
{
   // TODO
}

uint64_t hw_get_unique_id()
{
   // TODO
   return 0x1122334455667788;
}

void hw_busy_wait(int16_t microseconds)
{
   volatile int i = 0;
#if 0
   uint32_t comp = (uint32_t) microseconds * 12500000 / 1000000; // core clock = 12.5MHz
#else
   uint32_t comp = (uint32_t) microseconds * 125 / 10;
#endif

   while(i!=comp)
      i++;
}

void hw_reset()
{
   // TODO
   start();
}

float hw_get_internal_temperature()
{
   // TODO
   return 0;
}

uint32_t hw_get_battery(void)
{
   // TODO
   return 0;
}

system_reboot_reason_t hw_system_reboot_reason()
{
  return REBOOT_REASON_NOT_IMPLEMENTED;
}
