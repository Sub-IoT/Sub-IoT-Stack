/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2010, 2012, Roel Verdult, Armin van der Togt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef _STM32L1xx_MD_HANDLERS_H
#define _STM32L1xx_MD_HANDLERS_H

#ifdef STM32L1XX_MD
// The GCC compiler defines the current architecture derived from the -mcpu argument.
// When target cpu is the cortex-m3, it automatically defines __ARM_ARCH_7M__
#if !defined(__ARM_ARCH_7M__)
#error "The target ARM cpu must be Cortex-M3 compatible (-mcpu=cortex-m3)"
#endif

// Declare a weak alias macro as described in the GCC manual[1][2]
#define WEAK_ALIAS(f) __attribute__ ((weak, alias (#f)))
#define SECTION(s) __attribute__ ((section(s)))

#define BootRAM 0xF1E0F85F

/******************************************************************************
 * Forward undefined IRQ handlers to an infinite loop function. The Handlers
 * are weakly aliased which means that (re)definitions will override these.
 *****************************************************************************/

void irq_undefined(void) {
	// Do nothing when occurred interrupt is not defined, just keep looping
	while (1)
		;
}

void WWDG_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void PVD_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void TAMPER_STAMP_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void RTC_WKUP_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void FLASH_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void RCC_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void EXTI0_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void EXTI1_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void EXTI2_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void EXTI3_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void EXTI4_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void DMA1_Channel1_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void DMA1_Channel2_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void DMA1_Channel3_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void DMA1_Channel4_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void DMA1_Channel5_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void DMA1_Channel6_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void DMA1_Channel7_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void ADC1_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void USB_HP_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void USB_LP_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void DAC_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void COMP_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void EXTI9_5_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void LCD_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void TIM9_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void TIM10_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void TIM11_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void TIM2_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void TIM3_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void TIM4_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void I2C1_EV_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void I2C1_ER_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void I2C2_EV_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void I2C2_ER_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void SPI1_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void SPI2_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void USART1_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void USART2_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void USART3_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void EXTI15_10_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void RTC_Alarm_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void USB_FS_WKUP_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void TIM6_IRQHandler(void) WEAK_ALIAS(irq_undefined);
void TIM7_IRQHandler(void) WEAK_ALIAS(irq_undefined);

/*****************************************************************************
 * Forward undefined fault handlers to an infinite loop function. The Handlers
 * are weakly aliased which means that (re)definitions will override these.
 ****************************************************************************/

void fault_undefined() {
	// Do nothing when occurred interrupt is not defined, just keep looping
	while (1)
		;
}

void NMI_Handler(void) WEAK_ALIAS(fault_undefined);
void HardFault_Handler(void) WEAK_ALIAS(fault_undefined);
void MemManage_Handler(void) WEAK_ALIAS(fault_undefined);
void BusFault_Handler(void) WEAK_ALIAS(fault_undefined);
void UsageFault_Handler(void) WEAK_ALIAS(fault_undefined);
void SVC_Handler(void) WEAK_ALIAS(fault_undefined);
void DebugMon_Handler(void) WEAK_ALIAS(fault_undefined);
void PendSV_Handler(void) WEAK_ALIAS(fault_undefined);
void SysTick_Handler(void) WEAK_ALIAS(fault_undefined);

// Prototype the entry values, which are handled by the linker script
extern void* __StackTop;
extern void boot_entry(void);

// Defined irq vectors using simple c code following the description in a white
// paper from ARM[3] and code example from Simonsson Fun Technologies[4].
// These vectors are placed at the memory location defined in the linker script
//void* __attribute__ ((used)) g_pfnVectors[] SECTION(".isr_vector") =
void* g_pfnVectors[] SECTION(".isr_vector") =
{
	// Stack and program reset entry point
	&__StackTop,// The initial stack pointer
	boot_entry,// The reset handler
	NMI_Handler,
	HardFault_Handler,
	MemManage_Handler,
	BusFault_Handler,
	UsageFault_Handler,
	0,
	0,
	0,
	0,
	SVC_Handler,
	DebugMon_Handler,
	0,
	PendSV_Handler,
	SysTick_Handler,
	WWDG_IRQHandler,
	PVD_IRQHandler,
	TAMPER_STAMP_IRQHandler,
	RTC_WKUP_IRQHandler,
	FLASH_IRQHandler,
	RCC_IRQHandler,
	EXTI0_IRQHandler,
	EXTI1_IRQHandler,
	EXTI2_IRQHandler,
	EXTI3_IRQHandler,
	EXTI4_IRQHandler,
	DMA1_Channel1_IRQHandler,
	DMA1_Channel2_IRQHandler,
	DMA1_Channel3_IRQHandler,
	DMA1_Channel4_IRQHandler,
	DMA1_Channel5_IRQHandler,
	DMA1_Channel6_IRQHandler,
	DMA1_Channel7_IRQHandler,
	ADC1_IRQHandler,
	USB_HP_IRQHandler,
	USB_LP_IRQHandler,
	DAC_IRQHandler,
	COMP_IRQHandler,
	EXTI9_5_IRQHandler,
	LCD_IRQHandler,
	TIM9_IRQHandler,
	TIM10_IRQHandler,
	TIM11_IRQHandler,
	TIM2_IRQHandler,
	TIM3_IRQHandler,
	TIM4_IRQHandler,
	I2C1_EV_IRQHandler,
	I2C1_ER_IRQHandler,
	I2C2_EV_IRQHandler,
	I2C2_ER_IRQHandler,
	SPI1_IRQHandler,
	SPI2_IRQHandler,
	USART1_IRQHandler,
	USART2_IRQHandler,
	USART3_IRQHandler,
	EXTI15_10_IRQHandler,
	RTC_Alarm_IRQHandler,
	USB_FS_WKUP_IRQHandler,
	TIM6_IRQHandler,
	TIM7_IRQHandler,
	0,
	0,
	0,
	0,
	0,
	(void*)BootRAM /* This is for boot in RAM mode. */
};
#endif
/******************************************************************************
 * References
 *  [1] http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
 *  [2] http://gcc.gnu.org/onlinedocs/gcc/Variable-Attributes.html
 *  [3] http://www.arm.com/files/pdf/Cortex-M3_programming_for_ARM7_developers.pdf
 *  [4] http://fun-tech.se/stm32/OlimexBlinky/mini.php
 *****************************************************************************/

#endif
