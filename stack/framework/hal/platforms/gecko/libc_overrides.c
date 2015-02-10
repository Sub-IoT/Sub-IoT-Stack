#include "hwuart.h"
#include "hwatomic.h"
#include "hwleds.h"
#include <stdio.h>
//Overwrite _write so 'printf''s get pushed over the uart
int _write(int fd, char *ptr, int len)
{
  uart_transmit_message(ptr, len);
  return len;
}

//Overwrite _exit so we don't get a fault that's impossible to debug
void _exit(int exit)
{
    start_atomic();
	//wait forever while the interrupts are disabled
        while(1)
        {
        	//blink the leds so we know _exit has been called
        	for(uint32_t i = 0; i < 0x1FFFFF; i++){}
			led_toggle(0);
			led_toggle(1);
        }
    end_atomic();
}

//we override __assert_func to flash the leds (so we know something bad has happend)
//and to repeat the error message repeatedly (so we have a chance to attach the device to a serial console before the error message is gone)
void __assert_func( const char *file, int line, const char *func, const char *failedexpr)
{
	start_atomic();
	led_on(0);
	led_on(1);
	while(1)
	{
		printf("assertion \"%s\" failed: file \"%s\", line %d%s%s\n",failedexpr, file, line, func ? ", function: " : "", func ? func : "");
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
