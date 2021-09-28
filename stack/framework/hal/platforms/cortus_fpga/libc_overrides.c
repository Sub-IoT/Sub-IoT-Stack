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

#include "hwuart.h"
#include "hwatomic.h"
#include "hwleds.h"
#include "log.h"

//we override __assert_func to abort the CPU and get the stack trace
void __assert_func( const char *file, int line, const char *func, const char *failedexpr)
{
    start_atomic();
    led_on(0);
    led_on(1);
    abort();
}

size_t strnlen(const char *str, size_t maxlen)
{
  const char *start = str;

  while (maxlen-- > 0 && *str)
    str++;

  return str - start;
}
