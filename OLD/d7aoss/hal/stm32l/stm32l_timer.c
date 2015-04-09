/*
 *  Created on: May 9, 2013
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */
#include <stdbool.h>
#include <stdint.h>

#include "misc.h"
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_tim.h"

#include <../framework/log.h>
#include <../framework/timer.h>

#include <signal.h>
#include <time.h>

//timer_t timerid;

void hal_timer_init() {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* TIM4 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 31249; // base clock = 32000000/31250 = 1024Hz
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

	TIM_OC1Init(TIM4, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable );

	TIM_ARRPreloadConfig(TIM4, ENABLE);

	/* TIM Interrupt configuration */
	TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);

	/* ------------------------- NVIC Configuration ------------------------------ */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* TIM4 enable counter */
	TIM_Cmd(TIM4, ENABLE);
}

void hal_timer_enable_interrupt() {
	NVIC ->ISER[TIM4_IRQn >> 0x05] =
			(uint32_t) 0x01 << (TIM4_IRQn & (uint8_t) 0x1F);
}

void hal_timer_disable_interrupt() {
	NVIC ->ICER[TIM4_IRQn >> 0x05] =
			(uint32_t) 0x01 << (TIM4_IRQn & (uint8_t) 0x1F);
}

int16_t hal_timer_getvalue() {
	return TIM4->CNT;
}

void hal_timer_setvalue(uint16_t next_event) {
	TIM4->CCR1 = next_event;
	TIM4->CNT = 0;
}

void TIM4_IRQHandler(void) {
	if (TIM4 ->SR & TIM_IT_CC1 ) {
		timer_completed();
		TIM4 ->SR = (uint16_t) ~TIM_IT_CC1;
	}
}
