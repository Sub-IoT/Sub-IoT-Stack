/*
 * button.c
 *
 *  Created on: 19-mrt-2010
 *      Author: armin
 */
#include <stdbool.h>
#include <stm32l1xx.h>
#include <stm32l1xx_gpio.h>
#include <stm32l1xx_exti.h>
#include <stm32l1xx_rcc.h>
#include <stm32l1xx_syscfg.h>
#include <misc.h>

#include <leds.h>
#include <button.h>

#include "interrupts.h"

void button_init() {

	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource2 );

	/* Configure pins as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Configure interrupts
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

}

void button_enable_interrupts() {
	NVIC ->ISER[EXTI2_IRQn >> 0x05] = (uint32_t) 0x01
			<< (EXTI2_IRQn & (uint8_t) 0x1F);
}

void button_disable_interrupts() {
	NVIC ->ICER[EXTI2_IRQn >> 0x05] = (uint32_t) 0x01
			<< (EXTI2_IRQn & (uint8_t) 0x1F);
}

void button_clear_interrupt_flag() {
	EXTI ->PR = EXTI_Line2;
}

unsigned char button_is_active(unsigned char button) {
	unsigned char result = 0;
	switch (button) {
	case 1:
		result = GPIOB ->IDR & GPIO_Pin_2;
		break;
	default:
		result = 0;
		break;
	}
	return result;
}

void EXTI2_IRQHandler(void) {
	if (EXTI ->PR & EXTI_Line2 ) {

		if (GPIOB ->IDR & GPIO_Pin_2 ) {
			interrupt_flags |= INTERRUPT_BUTTON1;
		}
		/* Clear the  EXTI line 4 pending bit */
		EXTI ->PR = EXTI_Line2;
	}

}

