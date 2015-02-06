#include "scheduler.h"
#include "bootstrap.h"
#include "hwuart.h"
#include "hwleds.h"
#include "efm32gg_mcu.h"

void __platform_init()
{
    __efm32gg_mcu_init();
    __uart_init();
    __led_init();
}

int main()
{
    __platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    //scheduler_init();
    __framework_bootstrap();
    scheduler_run();
    return 0;
}

////Overwrite _write so 'printf''s get pushed over the uart
//int _write(int fd, char *ptr, int len)
//{
//  uart_transmit_message(ptr, len);
//  return len;
//}
