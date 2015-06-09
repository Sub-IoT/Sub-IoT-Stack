/* ---------------------------------------------------------------------------------------*/
/*  @file:    startup_MKL02Z4.s                                                           */
/*  @purpose: CMSIS Cortex-M0P Core Device Startup File                                   */
/*            MKL02Z4                                                                     */
/*  @version: 1.3                                                                         */
/*  @date:    2014-10-14                                                                  */
/*  @build:   b141023                                                                     */
/* ---------------------------------------------------------------------------------------*/
/*                                                                                        */
/* Copyright (c) 1997 - 2014 , Freescale Semiconductor, Inc.                              */
/* All rights reserved.                                                                   */
/*                                                                                        */
/* Redistribution and use in source and binary forms, with or without modification,       */
/* are permitted provided that the following conditions are met:                          */
/*                                                                                        */
/* o Redistributions of source code must retain the above copyright notice, this list     */
/*   of conditions and the following disclaimer.                                          */
/*                                                                                        */
/* o Redistributions in binary form must reproduce the above copyright notice, this       */
/*   list of conditions and the following disclaimer in the documentation and/or          */
/*   other materials provided with the distribution.                                      */
/*                                                                                        */
/* o Neither the name of Freescale Semiconductor, Inc. nor the names of its               */
/*   contributors may be used to endorse or promote products derived from this            */
/*   software without specific prior written permission.                                  */
/*                                                                                        */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND        */
/* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED          */
/* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE                 */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR       */
/* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES         */
/* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;           */
/* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON         */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT                */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                           */
/*****************************************************************************/
/* Version: GCC for ARM Embedded Processors                                  */
/*****************************************************************************/
    .syntax unified
    .arch armv6-m

    .section .isr_vector, "a"
    .align 2
    .globl __isr_vector
__isr_vector:
    .long   __StackTop                                      /* Top of Stack */
    .long   Reset_Handler                                   /* Reset Handler */
    .long   NMI_Handler                                     /* NMI Handler*/
    .long   HardFault_Handler                               /* Hard Fault Handler*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   SVC_Handler                                     /* SVCall Handler*/
    .long   0                                               /* Reserved*/
    .long   0                                               /* Reserved*/
    .long   PendSV_Handler                                  /* PendSV Handler*/
    .long   SysTick_Handler                                 /* SysTick Handler*/

                                                            /* External Interrupts*/
    .long   Reserved16_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved17_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved18_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved19_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved20_IRQHandler                           /* Reserved interrupt*/
    .long   FTFA_IRQHandler                                 /* FTFA command complete and read collision*/
    .long   LVD_LVW_IRQHandler                              /* Low-voltage detect, low-voltage warning*/
    .long   Reserved23_IRQHandler                           /* Reserved interrupt*/
    .long   I2C0_IRQHandler                                 /* I2C0 interrupt*/
    .long   I2C1_IRQHandler                                 /* I2C1 interrupt*/
    .long   SPI0_IRQHandler                                 /* SPI0 single interrupt vector for all sources*/
    .long   Reserved27_IRQHandler                           /* Reserved interrupt*/
    .long   UART0_IRQHandler                                /* UART0 status and error*/
    .long   Reserved29_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved30_IRQHandler                           /* Reserved interrupt*/
    .long   ADC0_IRQHandler                                 /* ADC0 interrupt*/
    .long   CMP0_IRQHandler                                 /* CMP0 interrupt*/
    .long   TPM0_IRQHandler                                 /* TPM0 single interrupt vector for all sources*/
    .long   TPM1_IRQHandler                                 /* TPM1 single interrupt vector for all sources*/
    .long   Reserved35_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved36_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved37_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved38_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved39_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved40_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved41_IRQHandler                           /* Reserved interrupt*/
    .long   Reserved42_IRQHandler                           /* Reserved interrupt*/
    .long   MCG_IRQHandler                                  /* MCG interrupt*/
    .long   LPTimer_IRQHandler                              /* LPTMR0 interrupt*/
    .long   Reserved45_IRQHandler                           /* Reserved interrupt*/
    .long   PORTA_IRQHandler                                /* PORTA pin detect*/
    .long   PORTB_IRQHandler                                /* PORTB pin detect*/

    .size    __isr_vector, . - __isr_vector

/* Flash Configuration */
    .section .FlashConfig, "a"
    .long 0xFFFFFFFF
    .long 0xFFFFFFFF
    .long 0xFFFFFFFF
    .long 0xFFFFFFFE

    .equ _NVIC_ICER,  0xE000E180
    .equ _NVIC_ICPR,  0xE000E280
    .text
    .thumb

/* Reset Handler */

    .thumb_func
    .align 2
    .globl   Reset_Handler
    .weak    Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:
    cpsid   i               /* Mask interrupts */
    ldr r0, =_NVIC_ICER    /* Disable interrupts and clear pending flags */
    ldr r1, =_NVIC_ICPR
    ldr r2, =0xFFFFFFFF
    str r2, [r0]            /* NVIC_ICER - clear enable IRQ register */
    str r2, [r1]            /* NVIC_ICPR - clear pending IRQ register */
#ifndef __NO_SYSTEM_INIT
    bl SystemInit
#endif
    cpsie   i               /* Unmask interrupts */
/*     Loop to copy data from read only memory to RAM. The ranges
 *      of copy from/to are specified by following symbols evaluated in
 *      linker script.
 *      __etext: End of code section, i.e., begin of data sections to copy from.
 *      __data_start__/__data_end__: RAM address range that data should be
 *      copied to. Both must be aligned to 4 bytes boundary.  */

    ldr    r1, =__etext
    ldr    r2, =__data_start__
    ldr    r3, =__data_end__

    subs    r3, r2
    ble     .LC0

.LC1:
    subs    r3, 4
    ldr    r0, [r1,r3]
    str    r0, [r2,r3]
    bgt    .LC1
.LC0:

#ifdef __STARTUP_CLEAR_BSS
/*     This part of work usually is done in C library startup code. Otherwise,
 *     define this macro to enable it in this startup.
 *
 *     Loop to zero out BSS section, which uses following symbols
 *     in linker script:
 *      __bss_start__: start of BSS section. Must align to 4
 *      __bss_end__: end of BSS section. Must align to 4
 */
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__

    subs    r2, r1
    ble .LC3

    movs    r0, 0
.LC2:
    str r0, [r1, r2]
    subs    r2, 4
    bge .LC2
.LC3:
#endif
#ifndef __START
#define __START _start
#endif
    bl    _start
    .pool
    .size Reset_Handler, . - Reset_Handler

    .align	1
    .thumb_func
    .weak DefaultISR
    .type DefaultISR, %function
DefaultISR:
    ldr	r0, =DefaultISR
    bx r0
    .size DefaultISR, . - DefaultISR

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro def_irq_handler	handler_name
    .weak \handler_name
    .set  \handler_name, DefaultISR
    .endm

/* Exception Handlers */
    def_irq_handler    NMI_Handler
    def_irq_handler    HardFault_Handler
    def_irq_handler    SVC_Handler
    def_irq_handler    PendSV_Handler
    def_irq_handler    SysTick_Handler
    def_irq_handler    Reserved16_IRQHandler
    def_irq_handler    Reserved17_IRQHandler
    def_irq_handler    Reserved18_IRQHandler
    def_irq_handler    Reserved19_IRQHandler
    def_irq_handler    Reserved20_IRQHandler
    def_irq_handler    FTFA_IRQHandler
    def_irq_handler    LVD_LVW_IRQHandler
    def_irq_handler    Reserved23_IRQHandler
    def_irq_handler    I2C0_IRQHandler
    def_irq_handler    I2C1_IRQHandler
    def_irq_handler    SPI0_IRQHandler
    def_irq_handler    Reserved27_IRQHandler
    def_irq_handler    UART0_IRQHandler
    def_irq_handler    Reserved29_IRQHandler
    def_irq_handler    Reserved30_IRQHandler
    def_irq_handler    ADC0_IRQHandler
    def_irq_handler    CMP0_IRQHandler
    def_irq_handler    TPM0_IRQHandler
    def_irq_handler    TPM1_IRQHandler
    def_irq_handler    Reserved35_IRQHandler
    def_irq_handler    Reserved36_IRQHandler
    def_irq_handler    Reserved37_IRQHandler
    def_irq_handler    Reserved38_IRQHandler
    def_irq_handler    Reserved39_IRQHandler
    def_irq_handler    Reserved40_IRQHandler
    def_irq_handler    Reserved41_IRQHandler
    def_irq_handler    Reserved42_IRQHandler
    def_irq_handler    MCG_IRQHandler
    def_irq_handler    LPTimer_IRQHandler
    def_irq_handler    Reserved45_IRQHandler
    def_irq_handler    PORTA_IRQHandler
    def_irq_handler    PORTB_IRQHandler

    .end
