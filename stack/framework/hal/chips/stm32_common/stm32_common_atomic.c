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

/*! \file stm32_common_atomic.c
 *
 *
 */

#include "hwatomic.h"
#include "stm32_device.h"

static volatile uint8_t nests = 0;

void start_atomic()
{  
  if(nests == 0)
    __disable_irq();
  ++nests;
}

void end_atomic()
{
  if(nests == 0)
    return;
  --nests;
  if(nests == 0)
    __enable_irq();
}

_Bool in_atomic()
{
  return (nests > 0);
}
