#include "hwgpio.h"
#include <assert.h>

__LINK_C error_t hw_gpio_set(pin_id_t pin_id)
{
	assert(false);
	return EOFF;
}

__LINK_C error_t hw_gpio_clr(pin_id_t pin_id)
{
	assert(false);
	return EOFF;
}

__LINK_C error_t hw_gpio_toggle(pin_id_t pin_id)
{
	assert(false);
	return EOFF;
}

__LINK_C bool hw_gpio_get_out(pin_id_t pin_id)
{
	assert(false);
	return EOFF;
}

__LINK_C bool hw_gpio_get_in(pin_id_t pin_id)
{
	assert(false);
	return EOFF;
}

__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id, gpio_inthandler_t callback, uint8_t event_mask)
{
	assert(false);
	return EOFF;
}
__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id)
{
	assert(false);
	return EOFF;
}
__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id)
{
	assert(false);
	return EOFF;
}
__LINK_C void __gpio_init(){}
