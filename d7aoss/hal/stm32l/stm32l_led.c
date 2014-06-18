/*
 * led.c
 *
 *  Created on: 21-feb-2010
 *      Author: armin
 */
#include "stm32l1xx.h"
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_rcc.h"

#include <leds.h>

static volatile GPIO_TypeDef* _led_ports[4];
static volatile uint16_t _led_pins[4];


void led_config(unsigned char index, GPIO_TypeDef* port, uint16_t pin, uint16_t clk) {
	GPIO_InitTypeDef GPIO_InitStructure;

	_led_ports[index] = port;
	_led_pins[index] = pin;
	/* Enable the GPIO_LED Clock */
	RCC_AHBPeriphClockCmd(clk, ENABLE);

	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(port, &GPIO_InitStructure);
}

void led_init()
{
	led_config(1, GPIOB, GPIO_Pin_6, RCC_AHBPeriph_GPIOB );
	led_config(0, GPIOB, GPIO_Pin_7, RCC_AHBPeriph_GPIOB );
	led_config(2, GPIOA, GPIO_Pin_2, RCC_AHBPeriph_GPIOA );
	led_config(3, GPIOA, GPIO_Pin_3, RCC_AHBPeriph_GPIOA );

}


void led_on(unsigned char index) {
	_led_ports[index]->BSRRL = _led_pins[index];
}

void led_toggle(unsigned char index) {
	if (_led_ports[index]->ODR & _led_pins[index]) {
		led_off(index);
	}
	else {
		led_on(index);
	}
}

void led_off(unsigned char index) {
	_led_ports[index]->BSRRH = _led_pins[index];
}
