/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file ezr32wg_mcu.c
 *
 *  \author glenn.ergeerts@uantwerpen.be
 *  \author daniel.vandenakker@uantwerpen.be
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include "em_cmu.h"
#include "em_chip.h"
#include "platform.h"

#define INT_PRIO_HIGH 1
#define INT_PRIO_LOW 2

void __ezr32lg_mcu_init()
{
    /* Chip errata */
    CHIP_Init();

    //Enable Profiler Trace
    //BSP_TraceProfilerSetup();

#ifdef HW_USE_HFXO
    // init clock with HFXO (external)
    CMU_ClockDivSet(cmuClock_HF, cmuClkDiv_1);		// 48 MHZ
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);   // Enable XTAL Osc and wait to stabilize
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO); // Select HF XTAL osc as system clock source. 48MHz XTAL, but we divided the system clock by 1, therefore our HF clock will be 48MHz
    //CMU_ClockDivSet(cmuClock_HFPER, cmuClkDiv_4); // TODO set HFPER clock divider (used for SPI) + disable gate clock when not used?
#else
    // init clock with HFRCO (internal)
    CMU_HFRCOBandSet(cmuHFRCOBand_21MHz);
    CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
#endif

    uint32_t hf = CMU_ClockFreqGet(cmuClock_HF);

    // set interrupt priorities, mainly to ensure USART IO has highest prio (after exceptions), to prevent RF overflow.
    // This is system wide for now.
    NVIC_SetPriorityGrouping(0); // only use preempt priorities, no subpriorities
    NVIC_SetPriority(SysTick_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(DMA_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(GPIO_EVEN_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(TIMER0_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(USARTRF0_RX_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(USARTRF0_TX_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(USB_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(ACMP0_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(ADC0_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(DAC0_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(I2C0_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(I2C1_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(GPIO_ODD_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(TIMER1_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(TIMER2_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(TIMER3_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(USART1_RX_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(USART1_TX_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(LESENSE_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(USART2_RX_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(USART2_TX_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(UART0_RX_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(UART0_TX_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(UART1_RX_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(UART1_TX_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(LEUART0_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(LEUART1_IRQn, INT_PRIO_HIGH);
    NVIC_SetPriority(LETIMER0_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(PCNT0_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(PCNT1_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(PCNT2_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(RTC_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(BURTC_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(CMU_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(VCMP_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(MSC_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(AES_IRQn, INT_PRIO_LOW);
    NVIC_SetPriority(EMU_IRQn, INT_PRIO_LOW);
}
