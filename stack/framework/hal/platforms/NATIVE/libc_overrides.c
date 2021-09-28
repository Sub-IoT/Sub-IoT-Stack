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

#include <stdio.h>
#include "console.h"
#include "assert.h"

////Overwrite _write so 'printf''s get pushed over the uart
//int _write(int fd, char *ptr, int len)
//{
//#ifdef FRAMEWORK_LOG_OUTPUT_ON_RTT
//    SEGGER_RTT_Write(0, ptr, len);
//#else
//    console_print_bytes((uint8_t*)ptr, len);
//#endif

//    return len;
//}

void __assert_func( const char *file, int line, const char *func, const char *failedexpr)
{
    __assert(failedexpr, file, line);
}

void start_atomic(void) {}
void end_atomic(void) {}
