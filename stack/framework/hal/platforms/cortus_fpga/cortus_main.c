
#include "machine/gpio.h"

#include "scheduler.h"
#include "bootstrap.h"
#include "platform.h"

#include "hwgpio.h"
#include "hwleds.h"
#include "hwwatchdog.h"



void __platform_init()
{
    __gpio_init();
    //__uart_init();
    __led_init();

#ifdef USE_CC1101
    // configure the interrupt pins here, since hw_gpio_configure_pin() is MCU
    // specific and not part of the common HAL API
    hw_gpio_configure_pin(CC1101_GDO0_PIN, true, gpioModeInput, 0);
    // hw_gpio_configure_pin(CC1101_SPI_PIN_CS, false, gpioModePushPull, 1);
#endif

    __watchdog_init();
}

int main (void)
{

    __platform_init();

    __framework_bootstrap();

    //__platform_post_framework_init();

    scheduler_run();

    return 0;
}
