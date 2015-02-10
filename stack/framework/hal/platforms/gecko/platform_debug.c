#include "hwdebug.h"
#include "platform.h"

#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_system.h"
#include "em_chip.h"

#ifdef PLATFORM_GECKO_DEBUGPINS

void __hw_debug_init()
{
	CMU_ClockEnable(cmuClock_GPIO, true);
	for(uint8_t i = DEBUG_PIN_START; i < DEBUG_PIN_START + DEBUG_PIN_NUM; i++)
		GPIO_PinModeSet(DEBUG_PORT, i, gpioModeDisabled, 0);
}

void hw_debug_set(uint8_t pin_id)
{
	if(pin_id < DEBUG_PIN_START || pin_id >= (DEBUG_PIN_START + DEBUG_PIN_NUM))
		return;
	GPIO_PinOutSet(DEBUG_PORT, pin_id);
}

void hw_debug_clr(uint8_t pin_id)
{
	if(pin_id < DEBUG_PIN_START || pin_id >= (DEBUG_PIN_START + DEBUG_PIN_NUM))
			return;
	GPIO_PinOutClear(DEBUG_PORT, pin_id);
}

void hw_debug_toggle(uint8_t pin_id)
{
	if(pin_id < DEBUG_PIN_START || pin_id >= (DEBUG_PIN_START + DEBUG_PIN_NUM))
		return;
	GPIO_PinOutToggle(DEBUG_PORT, pin_id);
}

void hw_debug_mask(uint32_t mask)
{
	//limit the mask to the bits that are used for debugging
	uint32_t real_mask = (mask & ((1<<DEBUG_PIN_NUM)-1))<<DEBUG_PIN_START;
	//use the set & clear registers so there is no chance of
	//affecting any pin settings we don't control
	GPIO->P[DEBUG_PORT].DOUTSET = real_mask;
	GPIO->P[DEBUG_PORT].DOUTCLR = (((1<<DEBUG_PIN_NUM)-1)<<DEBUG_PIN_START) & (~real_mask);
}

#else

void __hw_debug_init() {}
void hw_debug_set(uint8_t pin_id) {}
void hw_debug_clr(uint8_t pin_id) {}
void hw_debug_toggle(uint8_t pin_id) {}
void hw_debug_mask(uint32_t mask) {}

#endif
