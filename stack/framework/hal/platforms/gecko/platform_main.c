#include "scheduler.h"
#include "bootstrap.h"
#include "hwgpio.h"
#include "hwuart.h"
#include "hwleds.h"
#include "efm32gg_mcu.h"
#include "hwdebug.h"
#include "platform.h"
#include "userbutton.h"
#include "em_gpio.h"
#include <assert.h>

void __platform_init()
{
    __efm32gg_mcu_init();
    __gpio_init();
    __uart_init();
    __led_init();

#ifdef USE_CC1101
    //TODO: add calls to hw_gpio_configure_pin for the pins used by the CC1101 driver
    //(and possibly the spi interface)
#endif
    __hw_debug_init();

    error_t err;
    err = hw_gpio_configure_pin(BUTTON0, true, gpioModeInput,0); assert(err == SUCCESS);
    err = hw_gpio_configure_pin(BUTTON1, true, gpioModeInput,0); assert(err == SUCCESS);
}

void __platform_post_framework_init()
{
    __ubutton_init();
}

int main()
{
    //initialise the platform itself
	__platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();
    scheduler_run();
    return 0;
}
