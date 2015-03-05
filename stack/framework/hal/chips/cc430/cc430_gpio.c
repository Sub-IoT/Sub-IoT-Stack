#include "hwgpio.h"
#include "gpio.h"

__LINK_C void __gpio_init()
{
    // TODO
}

__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint8_t mode, unsigned int out)
{
    // TODO
}

__LINK_C error_t hw_gpio_set(pin_id_t pin_id)
{
    // TODO
    GPIO_setOutputHighOnPin(pin_id.port, pin_id.pin);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_clr(pin_id_t pin_id)
{
    // TODO
    GPIO_setOutputLowOnPin(pin_id.port, pin_id.pin);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_toggle(pin_id_t pin_id)
{
    // TODO
    GPIO_toggleOutputOnPin(pin_id.port, pin_id.pin);
    return SUCCESS;
}

__LINK_C bool hw_gpio_get_out(pin_id_t pin_id)
{
    // TODO
}

__LINK_C bool hw_gpio_get_in(pin_id_t pin_id)
{
    // TODO
}

static void gpio_int_callback(uint8_t pin)
{
    // TODO
}

__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id, gpio_inthandler_t callback, uint8_t event_mask)
{
    // TODO
}
__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id)
{
    // TODO
}

__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id)
{
    // TODO
}
