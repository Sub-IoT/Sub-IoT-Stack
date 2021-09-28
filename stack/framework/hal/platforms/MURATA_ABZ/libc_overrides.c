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
#include <stdio.h>
#include "timer.h"
#include "SEGGER_RTT.h"
#include "console.h"
#include "stm32l0xx.h"


//Overwrite _write so 'printf''s get pushed over the uart
int _write(int fd, char *ptr, int len)
{
#ifdef FRAMEWORK_LOG_OUTPUT_ON_RTT
    SEGGER_RTT_Write(0, ptr, len);
#else
    console_print_bytes((uint8_t*)ptr, len);
#endif

    return len;
}

//Overwrite _exit so we don't get a fault that's impossible to debug
void _exit(int exit)
{
    start_atomic();
    //wait forever while the interrupts are disabled
    while(1)
    {
    }
    end_atomic();
}

//we override __assert_func to flash the leds (so we know something bad has happend)
//and to repeat the error message repeatedly (so we have a chance to attach the device to a serial console before the error message is gone)
void __assert_func( const char *file, int line, const char *func, const char *failedexpr)
{
    start_atomic();

    while(1)
    {
#if defined(FRAMEWORK_LOG_ENABLED)
        printf("assertion \"%s\" failed: file \"%s\", line %d%s%s\n",failedexpr, file, line, func ? ", function: " : "", func ? func : "");
#endif
        __BKPT (0); // break into debugger, when attached

    }

    end_atomic();
}
