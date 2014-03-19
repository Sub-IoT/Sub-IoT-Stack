/*
 *  Created on: May 9, 2013
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */
#include <stm32l1xx_exti.h>
#include <stm32l1xx_pwr.h>
#include <stm32l1xx_rcc.h>
#include <stm32l1xx_rtc.h>
#include <misc.h>

#include <leds.h>
#include <rtc.h>

#include "interrupts.h"

void rtc_init_counter_mode() {
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	RTC_InitTypeDef RTC_InitStructure;
	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC */
    PWR_RTCAccessCmd(ENABLE);

	  /* Reset RTC Domain */
	  RCC_RTCResetCmd(ENABLE);
	  RCC_RTCResetCmd(DISABLE);

	/* Enable the LSE OSC */
	RCC_LSEConfig(RCC_LSE_ON );

	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY ) == RESET) {
	}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE );
	/* Set the RTC time base to 1s */
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	RTC_InitStructure.RTC_SynchPrediv = 0x00FF;
	RTC_Init(&RTC_InitStructure);

	/* Enable the RTC Clock */
	RCC_RTCCLKCmd(ENABLE);
	/* Wait for RTC APB registers synchronization */
	RTC_WaitForSynchro();

	/* EXTI configuration *******************************************************/
	EXTI_ClearITPendingBit(EXTI_Line20 );
	EXTI_InitStructure.EXTI_Line = EXTI_Line20;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable the RTC Wakeup Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the RTC WakeUp Clock source: CK_SPRE (1Hz) */
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits );
	RTC_SetWakeUpCounter(0x0);

	/* Enable the RTC Wakeup Interrupt */
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	rtc_disable_interrupt();
	/* Enable Wakeup Counter */
	RTC_WakeUpCmd(ENABLE);

}

void rtc_enable_interrupt() {
	NVIC ->ISER[RTC_WKUP_IRQn >> 0x05] = (uint32_t) 0x01
			<< (RTC_WKUP_IRQn & (uint8_t) 0x1F);
}

void rtc_disable_interrupt() {
	NVIC ->ICER[RTC_WKUP_IRQn >> 0x05] = (uint32_t) 0x01
			<< (RTC_WKUP_IRQn & (uint8_t) 0x1F);
}

void rtc_start() {
// FIXME start rtc
}

void rtc_stop() {
// FIXME stop rtc
}
void RTC_WKUP_IRQHandler(void) {
	if (RTC_GetITStatus(RTC_IT_WUT ) != RESET) {
		interrupt_flags |= INTERRUPT_RTC;
		RTC_ClearITPendingBit(RTC_IT_WUT );
		EXTI_ClearITPendingBit(EXTI_Line20 );
	}
}
