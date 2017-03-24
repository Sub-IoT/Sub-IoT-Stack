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

/*! \file stm32l152_gpio.c

 *
 */

#include "types.h"
#include "hwgpio.h"
#include "stm32l1xx_hal_conf.h"
#include "stm32l1xx_hal.h"
#include "hwatomic.h"
#include "assert.h"
//#include "stm32l1xx_hal_gpio.h"
//#include "stm32l1xx_hal_rcc.h"

typedef struct
{
    gpio_inthandler_t callback;
    uint32_t interrupt_port;
} gpio_interrupt_t;


GPIO_TypeDef* ports[8] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH};

//the list of configured interrupts
static gpio_interrupt_t interrupts[16];

static uint16_t gpio_pins_configured[6];

__LINK_C void __gpio_init()
{
    for(int i = 0; i < 16; i++)
    {
        interrupts[i].callback = 0x0;
        interrupts[i].interrupt_port = 0xFF; //signal that a port has not yet been chosen
    }
    for(int i = 0; i < 6; i++)
        gpio_pins_configured[i] = 0;


    /* GPIO Ports Clock Enable */
    //todo: only used clk
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /* Initialize GPIO interrupt dispatcher */
    //GPIOINT_Init();
}

__LINK_C error_t hw_gpio_configure_pin(pin_id_t pin_id, bool int_allowed, uint32_t mode, unsigned int out)
{
    //do early-stop error checking
    //if((gpio_pins_configured[pin_id.port] & (1<<pin_id.pin)))
    //return EALREADY;
    //else if(int_allowed && (interrupts[pin_id.pin].interrupt_port != 0xFF))
    //return EBUSY;

    //set the pin to be configured
    //gpio_pins_configured[pin_id.port] |= (1<<pin_id.pin);

    //configure the pin itself
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = 1 << pin_id.pin;
    GPIO_InitStruct.Mode = mode;
    if  (GPIO_InitStruct.Mode == GPIO_MODE_IT_RISING|| GPIO_InitStruct.Mode == GPIO_MODE_IT_FALLING || GPIO_InitStruct.Mode == GPIO_MODE_IT_RISING_FALLING)
    {
    	GPIO_InitStruct.Pull = GPIO_NOPULL;
    } else {
    	GPIO_InitStruct.Pull = GPIO_PULLDOWN; // todo ??
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    }
    HAL_GPIO_Init(ports[pin_id.port], &GPIO_InitStruct);

    //if interrupts are allowed: set the port to use
    if(int_allowed)
        interrupts[pin_id.pin].interrupt_port = pin_id.port;

    return SUCCESS;
}


__LINK_C error_t hw_gpio_configure_pin_stm(pin_id_t pin_id, void* GPIO_InitStruct)
{
	GPIO_InitTypeDef* initStruct = (GPIO_InitTypeDef*) GPIO_InitStruct;

	HAL_GPIO_Init(ports[pin_id.port], initStruct);

	if  (initStruct->Mode == GPIO_MODE_IT_RISING || initStruct->Mode == GPIO_MODE_IT_FALLING || initStruct->Mode == GPIO_MODE_IT_RISING_FALLING)
	{
		interrupts[pin_id.pin].interrupt_port = pin_id.port;
	}

	return SUCCESS;
}

__LINK_C error_t hw_gpio_set(pin_id_t pin_id)
{
    HAL_GPIO_WritePin(ports[pin_id.port], 1 << pin_id.pin, GPIO_PIN_SET);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_clr(pin_id_t pin_id)
{
    HAL_GPIO_WritePin(ports[pin_id.port], 1 << pin_id.pin, GPIO_PIN_RESET);
    return SUCCESS;
}

__LINK_C error_t hw_gpio_toggle(pin_id_t pin_id)
{
    HAL_GPIO_TogglePin(ports[pin_id.port], 1 << pin_id.pin);
    return SUCCESS;
}

__LINK_C bool hw_gpio_get_out(pin_id_t pin_id)
{
    // todo check pin is not input pin
    return (HAL_GPIO_ReadPin(ports[pin_id.port], 1 << pin_id.pin) == GPIO_PIN_SET);
}

__LINK_C bool hw_gpio_get_in(pin_id_t pin_id)
{
    return (HAL_GPIO_ReadPin(ports[pin_id.port], 1 << pin_id.pin) == GPIO_PIN_SET);
}

static void gpio_int_callback(uint8_t pin)
{
    //we use emlib's GPIO interrupt handler which does NOT
    //disable the interrupts by default --> disable them here to get the same behavior !!
    start_atomic();
	assert(interrupts[pin].callback != 0x0);
	pin_id_t id = {interrupts[pin].interrupt_port, pin};
	//report an event_mask of '0' since the only way to check which event occurred
	//is to check the state of the pin from the interrupt handler and
    //since the execution of interrupt handlers may be 'delayed' this method is NOT reliable.
    // TODO find out if there is no way to do this reliable on efm32gg
    interrupts[pin].callback(id,0);
    end_atomic();
}

__LINK_C error_t hw_gpio_configure_interrupt(pin_id_t pin_id, gpio_inthandler_t callback, uint8_t event_mask)
{
	if(interrupts[pin_id.pin].interrupt_port != pin_id.port)
	    	return EOFF;
	    else if(callback == 0x0 || event_mask > (GPIO_RISING_EDGE | GPIO_FALLING_EDGE))
	    	return EINVAL;

	    error_t err;
	    start_atomic();
		//do this check atomically: interrupts[..] callback is altered by this function
		//so the check belongs in the critical section as well
	    if(interrupts[pin_id.pin].callback != 0x0 && interrupts[pin_id.pin].callback != callback)
		    err = EBUSY;
		else
		{
		    interrupts[pin_id.pin].callback = callback;
//	    	GPIOINT_CallbackRegister(pin_id.pin, &gpio_int_callback);
//		    GPIO_IntConfig(pin_id.port, pin_id.pin,
//				!!(event_mask & GPIO_RISING_EDGE),
//				!!(event_mask & GPIO_FALLING_EDGE),
//				false);
		    err = SUCCESS;
		}
	    end_atomic();
	    return err;
}
__LINK_C error_t hw_gpio_enable_interrupt(pin_id_t pin_id)
{
	if (pin_id.pin >= 5 && pin_id.pin <= 9)
	{
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
		return SUCCESS;
	}

	if (pin_id.pin >= 10 && pin_id.pin <= 15)
	{
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
		return SUCCESS;
	}
    return FAIL;
}

__LINK_C error_t hw_gpio_disable_interrupt(pin_id_t pin_id)
{
	//TODO: check if no other pins are still using the interrupt
	if (pin_id.pin >= 5 && pin_id.pin <= 9)
	{
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
		return SUCCESS;
	}
	if (pin_id.pin >= 10 && pin_id.pin <= 15)
	{
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
		return SUCCESS;
	}
	return FAIL;
}

void EXTI9_5_IRQHandler(void)
{
	// will check the different pins here instead of using the HAL

	uint8_t pin_id = 5;
	for (;pin_id <= 9; pin_id++)
	{
		uint16_t pin = 1 << pin_id;
		if(__HAL_GPIO_EXTI_GET_IT(pin) != RESET)
		{
			__HAL_GPIO_EXTI_CLEAR_IT(pin);
			gpio_int_callback(pin_id);
			return;
		}
	}
}

void EXTI15_10_IRQHandler(void)
{
	// will check the different pins here instead of using the HAL

		uint8_t pin_id = 10;
		for (;pin_id <= 15; pin_id++)
		{
			uint16_t pin = 1 << pin_id;
			if(__HAL_GPIO_EXTI_GET_IT(pin) != RESET)
			{
				__HAL_GPIO_EXTI_CLEAR_IT(pin);
				gpio_int_callback(pin_id);
				return;
			}
		}
}




