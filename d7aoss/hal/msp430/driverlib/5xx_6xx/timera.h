/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
#ifndef __MSP430WARE_TIMERA_H__
#define __MSP430WARE_TIMERA_H__

#define __MSP430_HAS_TxA7__
//*****************************************************************************
//
//The following are values that can be passed to the
//TimerA_startContinuousMode();, TimerA_startUpMode();, TimerA_startUpDownMode();,
//TimerA_generatePWM(); APIs as the clockSource parameter.
//
//*****************************************************************************
#define TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK            TASSEL__TACLK
#define TIMERA_CLOCKSOURCE_ACLK                      TASSEL__ACLK
#define TIMERA_CLOCKSOURCE_SMCLK                     TASSEL__SMCLK
#define TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK   TASSEL__INCLK

//*****************************************************************************
//
//The following are values that can be passed to the
//TimerA_startContinuousMode();, TimerA_startUpMode();, TimerA_startUpDownMode();,
//TimerA_generatePWM(); APIs as the clockSourceDivider parameter.
//
//*****************************************************************************
#define TIMERA_CLOCKSOURCE_DIVIDER_1     0x01
#define TIMERA_CLOCKSOURCE_DIVIDER_2     0x02
#define TIMERA_CLOCKSOURCE_DIVIDER_4     0x04
#define TIMERA_CLOCKSOURCE_DIVIDER_8     0x08
#define TIMERA_CLOCKSOURCE_DIVIDER_3     0x03
#define TIMERA_CLOCKSOURCE_DIVIDER_5     0x05
#define TIMERA_CLOCKSOURCE_DIVIDER_6     0x06
#define TIMERA_CLOCKSOURCE_DIVIDER_7     0x07
#define TIMERA_CLOCKSOURCE_DIVIDER_10    0x0A
#define TIMERA_CLOCKSOURCE_DIVIDER_12    0x0C
#define TIMERA_CLOCKSOURCE_DIVIDER_14    0x0E
#define TIMERA_CLOCKSOURCE_DIVIDER_16    0x10
#define TIMERA_CLOCKSOURCE_DIVIDER_20    0x14
#define TIMERA_CLOCKSOURCE_DIVIDER_24    0x18
#define TIMERA_CLOCKSOURCE_DIVIDER_28    0x1C
#define TIMERA_CLOCKSOURCE_DIVIDER_32    0x20
#define TIMERA_CLOCKSOURCE_DIVIDER_40    0x28
#define TIMERA_CLOCKSOURCE_DIVIDER_48    0x30
#define TIMERA_CLOCKSOURCE_DIVIDER_56    0x38
#define TIMERA_CLOCKSOURCE_DIVIDER_64    0x40

//*****************************************************************************
//
//The following are values that can be passed to TimerA_startContinuousMode();
//TimerA_startUpMode();,  TimerA_startUpDownMode(); as the timerClear parameter.
//
//*****************************************************************************
#define TIMERA_DO_CLEAR      TACLR
#define TIMERA_SKIP_CLEAR    0x00

//*****************************************************************************
//
//The following are values that can be passed to the
//TimerA_getSynchronizedCaptureCompareInput(); API as the synchronized
//parameter.
//
//*****************************************************************************
#define TIMERA_CAPTURECOMPARE_INPUT                  SCCI
#define TIMERA_SYNCHRONIZED_CAPTURECOMPARE_INPUT     CCI

//*****************************************************************************
//
//The following are values that is returned by the
//TimerA_getSynchronizedCaptureCompareInput(); API
//
//*****************************************************************************
#define TIMERA_CAPTURECOMPARE_INPUT_HIGH    0x01
#define TIMERA_CAPTURECOMPARE_INPUT_LOW     0x00


//*****************************************************************************
//
//The following are values that is returned by the
//TimerA_getOutputForOutputModeOutBitValue(); and passed to
//TimerA_setOutputForOutputModeOutBitValue(); as
//outputModeOutBitValue parameter
//
//*****************************************************************************
#define TIMERA_OUTPUTMODE_OUTBITVALUE_HIGH    OUT
#define TIMERA_OUTPUTMODE_OUTBITVALUE_LOW     0x00

//*****************************************************************************
//
//The following are values can be passed to the mask parameter of
//TimerA_captureCompareInterruptStatus(); API
//
//*****************************************************************************
#define TIMERA_CAPTURE_OVERFLOW                  COV
#define TIMERA_CAPTURECOMPARE_INTERRUPT_FLAG     CCIFG

//*****************************************************************************
//
//The following are values can be passed to the timerInterruptEnable_TAIE
//parameter of TimerA_startContinuousMode();, TimerA_startUpMode();,
//TimerA_startUpDownMode();
//
//*****************************************************************************
#define TIMERA_TAIE_INTERRUPT_ENABLE            TAIE
#define TIMERA_TAIE_INTERRUPT_DISABLE           0x00

//*****************************************************************************
//
//The following are values can be passed to the
//captureCompareInterruptEnable_CCR0_CCIE parameter of TimerA_startUpMode();,
//TimerA_startUpDownMode API
//
//*****************************************************************************
#define TIMERA_CCIE_CCR0_INTERRUPT_ENABLE   CCIE
#define TIMERA_CCIE_CCR0_INTERRUPT_DISABLE  0x00

//*****************************************************************************
//
//The following are timer modes possible.
//
//*****************************************************************************
#define TIMERA_STOP_MODE         MC_0
#define TIMERA_UP_MODE           MC_1
#define TIMERA_CONTINUOUS_MODE   MC_2
#define TIMERA_UPDOWN_MODE       MC_3

//*****************************************************************************
//
//The following are values can be passed to the
//compareRegister, captureCompareRegister or captureRegister parameter
//of TimerA_initCapture();, TimerA_enableCaptureCompareInterrupt();,
//TimerA_disableCaptureCompareInterrupt();,TimerA_captureCompareInterruptStatus();,
//TimerA_getSynchronizedCaptureCompareInput();,TimerA_initCompare();
//
//*****************************************************************************
#define TIMERA_CAPTURECOMPARE_REGISTER_0     0x02
#define TIMERA_CAPTURECOMPARE_REGISTER_1     0x04
#define TIMERA_CAPTURECOMPARE_REGISTER_2     0x06
#define TIMERA_CAPTURECOMPARE_REGISTER_3     0x08
#define TIMERA_CAPTURECOMPARE_REGISTER_4     0x0A
#define TIMERA_CAPTURECOMPARE_REGISTER_5     0x0C
#define TIMERA_CAPTURECOMPARE_REGISTER_6     0x0E

//*****************************************************************************
//
//The following are values can be passed to the
//captureMode parameter of TimerA_initCompare();, TimerA_initCapture();,
//TimerA_generatePWM();,
//
//*****************************************************************************
#define TIMERA_OUTPUTMODE_OUTBITVALUE        OUTMOD_0
#define TIMERA_OUTPUTMODE_SET                OUTMOD_1
#define TIMERA_OUTPUTMODE_TOGGLE_RESET       OUTMOD_2
#define TIMERA_OUTPUTMODE_SET_RESET          OUTMOD_3
#define TIMERA_OUTPUTMODE_TOGGLE             OUTMOD_4
#define TIMERA_OUTPUTMODE_RESET              OUTMOD_5
#define TIMERA_OUTPUTMODE_TOGGLE_SET         OUTMOD_6
#define TIMERA_OUTPUTMODE_RESET_SET          OUTMOD_7

//*****************************************************************************
//
//The following are values can be passed to the
//captureMode parameter of TimerA_initCapture(); API
//
//*****************************************************************************
#define TIMERA_CAPTUREMODE_NO_CAPTURE                CM_0
#define TIMERA_CAPTUREMODE_RISING_EDGE               CM_1
#define TIMERA_CAPTUREMODE_FALLING_EDGE              CM_2
#define TIMERA_CAPTUREMODE_RISING_AND_FALLING_EDGE   CM_3

//*****************************************************************************
//
//The following are values can be passed to the
//synchronizeCaptureSource parameter of TimerA_initCapture(); API
//
//*****************************************************************************
#define TIMERA_CAPTURE_ASYNCHRONOUS                  0x00
#define TIMERA_CAPTURE_SYNCHRONOUS                   SCS

//*****************************************************************************
//
//The following are values can be passed to the
//captureInterruptEnable, compareInterruptEnable parameter of
//TimerA_initCapture(); API
//
//*****************************************************************************
#define TIMERA_CAPTURECOMPARE_INTERRUPT_ENABLE       CCIE
#define TIMERA_CAPTURECOMPARE_INTERRUPT_DISABLE      0x00

//*****************************************************************************
//
//The following are values can be passed to the
//captureInputSelect parameter of TimerA_initCapture(); API
//
//*****************************************************************************
#define TIMERA_CAPTURE_INPUTSELECT_CCIxA             CCIS_0
#define TIMERA_CAPTURE_INPUTSELECT_CCIxB             CCIS_1
#define TIMERA_CAPTURE_INPUTSELECT_GND               CCIS_2
#define TIMERA_CAPTURE_INPUTSELECT_Vcc               CCIS_3

//*****************************************************************************
//
//The following are values that may be returned by
//TimerA_getInterruptStatus(); API
//
//*****************************************************************************
#define TIMERA_INTERRUPT_NOT_PENDING     0x00
#define TIMERA_INTERRUPT_PENDING         0x01

//*****************************************************************************
//
//The following are values can be passed to the
//synchronized parameter of TimerA_getSynchronizedCaptureCompareInput(); API
//
//*****************************************************************************
#define TIMERA_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT SCCI
#define TIMERA_READ_CAPTURE_COMPARE_INPUT            CCI

//*****************************************************************************
//
//Prototypes for the APIs.
//
//*****************************************************************************
extern void TimerA_startCounter ( unsigned int baseAddress,
    unsigned int timerMode
    );
extern void TimerA_configureContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int timerClear
    );
extern void TimerA_configureUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    );
extern void TimerA_configureUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    );
extern void TimerA_startContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int timerClear
    );
extern void TimerA_startContinousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int timerClear
    );
extern void TimerA_startUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    );
extern void TimerA_startUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    );
extern void TimerA_initCapture (unsigned int baseAddress,
    unsigned int captureRegister,
    unsigned int captureMode,
    unsigned int captureInputSelect,
    unsigned short synchronizeCaptureSource,
    unsigned short captureInterruptEnable,
    unsigned int captureOutputMode
    );
extern void TimerA_initCompare (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned short compareInterruptEnable,
    unsigned int compareOutputMode,
    unsigned int compareValue
    );
extern void TimerA_enableInterrupt (unsigned int baseAddress);
extern void TimerA_disableInterrupt (unsigned int baseAddress);
extern unsigned long TimerA_getInterruptStatus (unsigned int baseAddress);
extern void TimerA_enableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern void TimerA_disableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern unsigned long TimerA_getCaptureCompareInterruptStatus (unsigned int baseAddress,
		 unsigned int captureCompareRegister,
		 unsigned int mask
		 );
extern void TimerA_clear (unsigned int baseAddress);
extern unsigned short TimerA_getSynchronizedCaptureCompareInput
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned short synchronized
    );
extern unsigned char TimerA_getOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern unsigned int TimerA_getCaptureCompareCount
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern void TimerA_setOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned char outputModeOutBitValue
    );
extern void TimerA_generatePWM (  unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int compareRegister,
    unsigned int compareOutputMode,
    unsigned int dutyCycle
    );
extern void TimerA_stop ( unsigned int baseAddress );
extern void TimerA_setCompareValue (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned int compareValue
    );
extern void TimerA_clearTimerInterruptFlag (unsigned int baseAddress);
extern void TimerA_clearCaptureCompareInterruptFlag (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
#endif
