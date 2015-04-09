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
//! DEPRECATED FILE - Replaced by timera.h and timerb.h
#ifndef __MSP430WARE_TIMER_H__
#define __MSP430WARE_TIMER_H__

#define __MSP430_HAS_TxA7__
//*****************************************************************************
//
//The following are values that can be passed to the
//Timer_startContinuousMode(), Timer_startUpMode(), Timer_startUpDownMode(),
//Timer_generatePWM() APIs as the clockSource parameter.
//
//*****************************************************************************
#define TIMER_CLOCKSOURCE_EXTERNAL_TXCLK            TASSEL__TACLK
#define TIMER_CLOCKSOURCE_ACLK                      TASSEL__ACLK
#define TIMER_CLOCKSOURCE_SMCLK                     TASSEL__SMCLK
#define TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK   TASSEL__INCLK

//*****************************************************************************
//
//The following are values that can be passed to the
//Timer_startContinuousMode(), Timer_startUpMode(), Timer_startUpDownMode(),
//Timer_generatePWM() APIs as the clockSourceDivider parameter.
//
//*****************************************************************************
#define TIMER_CLOCKSOURCE_DIVIDER_1     0x01
#define TIMER_CLOCKSOURCE_DIVIDER_2     0x02
#define TIMER_CLOCKSOURCE_DIVIDER_4     0x04
#define TIMER_CLOCKSOURCE_DIVIDER_8     0x08
#define TIMER_CLOCKSOURCE_DIVIDER_3     0x03
#define TIMER_CLOCKSOURCE_DIVIDER_5     0x05
#define TIMER_CLOCKSOURCE_DIVIDER_6     0x06
#define TIMER_CLOCKSOURCE_DIVIDER_7     0x07
#define TIMER_CLOCKSOURCE_DIVIDER_10    0x0A
#define TIMER_CLOCKSOURCE_DIVIDER_12    0x0C
#define TIMER_CLOCKSOURCE_DIVIDER_14    0x0E
#define TIMER_CLOCKSOURCE_DIVIDER_16    0x10
#define TIMER_CLOCKSOURCE_DIVIDER_20    0x14
#define TIMER_CLOCKSOURCE_DIVIDER_24    0x18
#define TIMER_CLOCKSOURCE_DIVIDER_28    0x1C
#define TIMER_CLOCKSOURCE_DIVIDER_32    0x20
#define TIMER_CLOCKSOURCE_DIVIDER_40    0x28
#define TIMER_CLOCKSOURCE_DIVIDER_48    0x30
#define TIMER_CLOCKSOURCE_DIVIDER_56    0x38
#define TIMER_CLOCKSOURCE_DIVIDER_64    0x40

//*****************************************************************************
//
//The following are values that can be passed to Timer_startContinuousMode()
//Timer_startUpMode(),  Timer_startUpDownMode() as the timerClear parameter.
//
//*****************************************************************************
#define TIMER_DO_CLEAR      TACLR
#define TIMER_SKIP_CLEAR    0x00

//*****************************************************************************
//
//The following are values that can be passed to the
//Timer_getSynchronizedCaptureCompareInput() API as the synchronized
//parameter.
//
//*****************************************************************************
#define TIMER_CAPTURECOMPARE_INPUT                  SCCI
#define TIMER_SYNCHRONIZED_CAPTURECOMPARE_INPUT     CCI

//*****************************************************************************
//
//The following are values that is returned by the
//Timer_getSynchronizedCaptureCompareInput() API
//
//*****************************************************************************
#define TIMER_CAPTURECOMPARE_INPUT_HIGH    0x01
#define TIMER_CAPTURECOMPARE_INPUT_LOW     0x00


//*****************************************************************************
//
//The following are values that is returned by the
//Timer_getOutputForOutputModeOutBitValue() and passed to
//Timer_setOutputForOutputModeOutBitValue() as
//outputModeOutBitValue parameter
//
//*****************************************************************************
#define TIMER_OUTPUTMODE_OUTBITVALUE_HIGH    OUT
#define TIMER_OUTPUTMODE_OUTBITVALUE_LOW     0x00

//*****************************************************************************
//
//The following are values can be passed to the mask parameter of
//Timer_captureCompareInterruptStatus() API
//
//*****************************************************************************
#define TIMER_CAPTURE_OVERFLOW                  COV
#define TIMER_CAPTURECOMPARE_INTERRUPT_FLAG     CCIFG

//*****************************************************************************
//
//The following are values can be passed to the timerInterruptEnable_TAIE
//parameter of Timer_startContinuousMode(), Timer_startUpMode(),
//Timer_startUpDownMode()
//
//*****************************************************************************
#define TIMER_TAIE_INTERRUPT_ENABLE            TAIE
#define TIMER_TAIE_INTERRUPT_DISABLE           0x00

//*****************************************************************************
//
//The following are values can be passed to the
//captureCompareInterruptEnable_CCR0_CCIE parameter of Timer_startUpMode(),
//Timer_startUpDownMode API
//
//*****************************************************************************
#define TIMER_CCIE_CCR0_INTERRUPT_ENABLE   CCIE
#define TIMER_CCIE_CCR0_INTERRUPT_DISABLE  0x00

//*****************************************************************************
//
//The following are timer modes possible.
//
//*****************************************************************************
#define TIMER_STOP_MODE         MC_0
#define TIMER_UP_MODE           MC_1
#define TIMER_CONTINUOUS_MODE   MC_2
#define TIMER_UPDOWN_MODE       MC_3

//*****************************************************************************
//
//The following are values can be passed to the
//compareRegister, captureCompareRegister or captureRegister parameter
//of Timer_initCapture(), Timer_enableCaptureCompareInterrupt(),
//Timer_disableCaptureCompareInterrupt(),Timer_captureCompareInterruptStatus(),
//Timer_getSynchronizedCaptureCompareInput(),Timer_initCompare()
//
//*****************************************************************************
#define TIMER_CAPTURECOMPARE_REGISTER_0     0x02
#define TIMER_CAPTURECOMPARE_REGISTER_1     0x04
#define TIMER_CAPTURECOMPARE_REGISTER_2     0x06
#define TIMER_CAPTURECOMPARE_REGISTER_3     0x08
#define TIMER_CAPTURECOMPARE_REGISTER_4     0x0A
#define TIMER_CAPTURECOMPARE_REGISTER_5     0x0C
#define TIMER_CAPTURECOMPARE_REGISTER_6     0x0E

//*****************************************************************************
//
//The following are values can be passed to the
//captureMode parameter of Timer_initCompare(), Timer_initCapture(),
//Timer_generatePWM(),
//
//*****************************************************************************
#define TIMER_OUTPUTMODE_OUTBITVALUE        OUTMOD_0
#define TIMER_OUTPUTMODE_SET                OUTMOD_1
#define TIMER_OUTPUTMODE_TOGGLE_RESET       OUTMOD_2
#define TIMER_OUTPUTMODE_SET_RESET          OUTMOD_3
#define TIMER_OUTPUTMODE_TOGGLE             OUTMOD_4
#define TIMER_OUTPUTMODE_RESET              OUTMOD_5
#define TIMER_OUTPUTMODE_TOGGLE_SET         OUTMOD_6
#define TIMER_OUTPUTMODE_RESET_SET          OUTMOD_7

//*****************************************************************************
//
//The following are values can be passed to the
//captureMode parameter of Timer_initCapture() API
//
//*****************************************************************************
#define TIMER_CAPTUREMODE_NO_CAPTURE                CM_0
#define TIMER_CAPTUREMODE_RISING_EDGE               CM_1
#define TIMER_CAPTUREMODE_FALLING_EDGE              CM_2
#define TIMER_CAPTUREMODE_RISING_AND_FALLING_EDGE   CM_3

//*****************************************************************************
//
//The following are values can be passed to the
//synchronizeCaptureSource parameter of Timer_initCapture() API
//
//*****************************************************************************
#define TIMER_CAPTURE_ASYNCHRONOUS                  0x00
#define TIMER_CAPTURE_SYNCHRONOUS                   SCS

//*****************************************************************************
//
//The following are values can be passed to the
//captureInterruptEnable, compareInterruptEnable parameter of
//Timer_initCapture() API
//
//*****************************************************************************
#define TIMER_CAPTURECOMPARE_INTERRUPT_ENABLE       CCIE
#define TIMER_CAPTURECOMPARE_INTERRUPT_DISABLE      0x00

//*****************************************************************************
//
//The following are values can be passed to the
//captureInputSelect parameter of Timer_initCapture() API
//
//*****************************************************************************
#define TIMER_CAPTURE_INPUTSELECT_CCIxA             CCIS_0
#define TIMER_CAPTURE_INPUTSELECT_CCIxB             CCIS_1
#define TIMER_CAPTURE_INPUTSELECT_GND               CCIS_2
#define TIMER_CAPTURE_INPUTSELECT_Vcc               CCIS_3

//*****************************************************************************
//
//The following are values that may be returned by
//Timer_getInterruptStatus() API
//
//*****************************************************************************
#define TIMER_INTERRUPT_NOT_PENDING     0x00
#define TIMER_INTERRUPT_PENDING         0x01

//*****************************************************************************
//
//The following are values can be passed to the
//synchronized parameter of Timer_getSynchronizedCaptureCompareInput() API
//
//*****************************************************************************
#define TIMER_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT SCCI
#define TIMER_READ_CAPTURE_COMPARE_INPUT            CCI

//*****************************************************************************
//
//Prototypes for the APIs.
//
//*****************************************************************************
extern
void Timer_startContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int timerclear
    );
extern
void Timer_startContinousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int timerclear
    );
extern
void Timer_startUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerclear
    );
extern
void Timer_startUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerclear
    );
extern
void Timer_initCapture (unsigned int baseAddress,
    unsigned int captureRegister,
    unsigned int captureMode,
    unsigned int captureInputSelect,
    unsigned short synchronizeCaptureSource,
    unsigned short captureInterruptEnable,
    unsigned int captureOutputMode
    );
extern
void Timer_initCompare (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned short compareInterruptEnable,
    unsigned int compareOutputMode,
    unsigned int compareValue
    );
extern
void Timer_enableInterrupt (unsigned int baseAddress);
extern
void Timer_disableInterrupt (unsigned int baseAddress);
extern
unsigned long Timer_getInterruptStatus (unsigned int baseAddress);
extern
void Timer_enableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern
void Timer_disableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern
unsigned long Timer_getCaptureCompareInterruptStatus
    (unsigned int baseAddress,
    unsigned int mask
    );
extern
void Timer_clear (unsigned int baseAddress);
extern
unsigned short Timer_getSynchronizedCaptureCompareInput
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned short synchronized
    );
extern
void Timer_setOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned char outputModeOutBitValue
    );
extern
unsigned char Timer_getOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );

extern
void Timer_generatePWM (  unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int compareRegister,
    unsigned int compareOutputMode,
    unsigned int dutyCycle
    );
extern
void Timer_stop ( unsigned int baseAddress );

extern
unsigned int Timer_getCaptureCompareCount
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern
void Timer_setCompareValue (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned int compareValue
    );
extern
void Timer_clearCaptureCompareInterruptFlag (unsigned int baseAddress,
    unsigned int compareRegister
    );
extern
void Timer_clearTimerInterruptFlag (unsigned int baseAddress);
#endif
