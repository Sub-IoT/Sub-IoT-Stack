#include "hwuart.h"
#include "hwatomic.h"
//Overwrite _write so 'printf''s get pushed over the uart
int _write(int fd, char *ptr, int len)
{
  uart_transmit_message(ptr, len);
  return len;
}

void _exit(int)
{
    start_atomic();
	//wait forever while the interrupts are disabled
        while(1){}
    end_atomic();
}