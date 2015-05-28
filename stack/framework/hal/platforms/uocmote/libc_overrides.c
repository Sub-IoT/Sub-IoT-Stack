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

#include "hwuart.h"
#include "hwatomic.h"
#include "hwleds.h"
#include "log.h"
#include <stdio.h>

//Overwrite _write so 'printf''s get pushed over the uart

// TODO
//int _write(int fd, char *ptr, int len)
//{
//  uart_transmit_message(ptr, len);
//  return len;
//}


//we override __assert_func to flash the leds (so we know something bad has happend)
//and to repeat the error message repeatedly (so we have a chance to attach the device to a serial console before the error message is gone)
void __assert_func( const char *file, int line, const char *func, const char *failedexpr)
{
	start_atomic();
	led_on(0);
	led_on(1);
	while(1)
	{
        log_print_string("assertion failed: file \"%s\", line %d%s%s\n",failedexpr, file, line, func ? ", function: " : "", func ? func : "");
		for(uint32_t j = 0; j < 20; j++)
		{
			//blink at twice the frequency of the _exit call, so we can identify which of the two events has occurred
			for(uint32_t i = 0; i < 0xFFFFF; i++){}
			led_toggle(0);
			led_toggle(1);
		}
	}
	end_atomic();
}
