#include "scheduler.h"
#include "bootstrap.h"
#include "hwuart.h"
#include "hwleds.h"
#include "efm32gg_mcu.h"
#include "hwdebug.h"

void __platform_init()
{
    __efm32gg_mcu_init();
    __uart_init();
    __led_init();
    __hw_debug_init();
}

int main()
{
    //initialise the platform itself
	__platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    scheduler_run();
    return 0;
}
