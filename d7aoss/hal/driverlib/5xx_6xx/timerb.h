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
#ifndef __MSP430WARE_TIMERB_H__
#define __MSP430WARE_TIMERB_H__

#define __MSP430_HAS_TxB7__
//*****************************************************************************
//
//The following are values that can be passed to the
//TimerB_startContinuousMode();, TimerB_startUpMode();, TimerB_startUpDownMode();,
//TimerB_generatePWM(); APIs as the clockSource parameter.
//
//*****************************************************************************
#define TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK            TBSSEL__TBCLK
#define TIMERB_CLOCKSOURCE_ACLK                      TBSSEL__ACLK
#define TIMERB_CLOCKSOURCE_SMCLK                     TBSSEL__SMCLK
#define TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK   TBSSEL__INCLK

//*****************************************************************************
//
//The following are values that can be passed to the
//TimerB_startContinuousMode();, TimerB_startUpMode();, TimerB_startUpDownMode();,
//TimerB_generatePWM(); APIs as the clockSourceDivider parameter.
//
//*****************************************************************************
#define TIMERB_CLOCKSOURCE_DIVIDER_1     0x01
#define TIMERB_CLOCKSOURCE_DIVIDER_2     0x02
#define TIMERB_CLOCKSOURCE_DIVIDER_4     0x04
#define TIMERB_CLOCKSOURCE_DIVIDER_8     0x08
#define TIMERB_CLOCKSOURCE_DIVIDER_3     0x03
#define TIMERB_CLOCKSOURCE_DIVIDER_5     0x05
#define TIMERB_CLOCKSOURCE_DIVIDER_6     0x06
#define TIMERB_CLOCKSOURCE_DIVIDER_7     0x07
#define TIMERB_CLOCKSOURCE_DIVIDER_10    0x0A
#define TIMERB_CLOCKSOURCE_DIVIDER_12    0x0C
#define TIMERB_CLOCKSOURCE_DIVIDER_14    0x0E
#define TIMERB_CLOCKSOURCE_DIVIDER_16    0x10
#define TIMERB_CLOCKSOURCE_DIVIDER_20    0x14
#define TIMERB_CLOCKSOURCE_DIVIDER_24    0x18
#define TIMERB_CLOCKSOURCE_DIVIDER_28    0x1C
#define TIMERB_CLOCKSOURCE_DIVIDER_32    0x20
#define TIMERB_CLOCKSOURCE_DIVIDER_40    0x28
#define TIMERB_CLOCKSOURCE_DIVIDER_48    0x30
#define TIMERB_CLOCKSOURCE_DIVIDER_56    0x38
#define TIMERB_CLOCKSOURCE_DIVIDER_64    0x40

//*****************************************************************************
//
//The following are values that can be passed to TimerB_startContinuousMode();
//TimerB_startUpMode();,  TimerB_startUpDownMode(); as the timerClear parameter.
//
//*****************************************************************************
#define TIMERB_DO_CLEAR      TBCLR
#define TIMERB_SKIP_CLEAR    0x00

//*****************************************************************************
//
//The following are values that can be passed to the
//TimerB_getSynchronizedCaptureCompareInput(); API as the synchronized
//parameter.
//
//*****************************************************************************
#define TIMERB_CAPTURECOMPARE_INPUT                  SCCI
#define TIMERB_SYNCHRONIZED_CAPTURECOMPARE_INPUT     CCI

//*****************************************************************************
//
//The following are values that is returned by the
//TimerB_getSynchronizedCaptureCompareInput(); API
//
//*****************************************************************************
#define TIMERB_CAPTURECOMPARE_INPUT_HIGH    0x01
#define TIMERB_CAPTURECOMPARE_INPUT_LOW     0x00


//*****************************************************************************
//
//The following are values that is returned by the
//TimerB_getOutputForOutputModeOutBitValue(); and passed to
//TimerB_setOutputForOutputModeOutBitValue(); as
//outputModeOutBitValue parameter
//
//*****************************************************************************
#define TIMERB_OUTPUTMODE_OUTBITVALUE_HIGH    OUT
#define TIMERB_OUTPUTMODE_OUTBITVALUE_LOW     0x00

//*****************************************************************************
//
//The following are values can be passed to the mask parameter of
//TimerB_captureCompareInterruptStatus(); API
//
//*****************************************************************************
#define TIMERB_CAPTURE_OVERFLOW                  COV
#define TIMERB_CAPTURECOMPARE_INTERRUPT_FLAG     CCIFG

//*****************************************************************************
//
//The following are values can be passed to the timerInterruptEnable_TBIE
//parameter of TimerB_startContinuousMode();, TimerB_startUpMode();,
//TimerB_startUpDownMode();
//
//*****************************************************************************
#define TIMERB_TBIE_INTERRUPT_ENABLE            TBIE
#define TIMERB_TBIE_INTERRUPT_DISABLE           0x00

//*****************************************************************************
//
//The following are values can be passed to the
//captureCompareInterruptEnable_CCR0_CCIE parameter of TimerB_startUpMode();,
//TimerB_startUpDownMode API
//
//*****************************************************************************
#define TIMERB_CCIE_CCR0_INTERRUPT_ENABLE   CCIE
#define TIMERB_CCIE_CCR0_INTERRUPT_DISABLE  0x00

//*****************************************************************************
//
//The following are timer modes possible.
//
//*****************************************************************************
#define TIMERB_STOP_MODE         MC_0
#define TIMERB_UP_MODE           MC_1
#define TIMERB_CONTINUOUS_MODE   MC_2
#define TIMERB_UPDOWN_MODE       MC_3

//*****************************************************************************
//
//The following are values can be passed to the
//compareRegister, captureCompareRegister or captureRegister parameter
//of TimerB_initCapture();, TimerB_enableCaptureCompareInterrupt();,
//TimerB_disableCaptureCompareInterrupt();,TimerB_captureCompareInterruptStatus();,
//TimerB_getSynchronizedCaptureCompareInput();,TimerB_initCompare();
//
//*****************************************************************************
#define TIMERB_CAPTURECOMPARE_REGISTER_0     0x02
#define TIMERB_CAPTURECOMPARE_REGISTER_1     0x04
#define TIMERB_CAPTURECOMPARE_REGISTER_2     0x06
#define TIMERB_CAPTURECOMPARE_REGISTER_3     0x08
#define TIMERB_CAPTURECOMPARE_REGISTER_4     0x0A
#define TIMERB_CAPTURECOMPARE_REGISTER_5     0x0C
#define TIMERB_CAPTURECOMPARE_REGISTER_6     0x0E

//*****************************************************************************
//
//The following are values can be passed to the
//captureMode parameter of TimerB_initCompare();, TimerB_initCapture();,
//TimerB_generatePWM();,
//
//*****************************************************************************
#define TIMERB_OUTPUTMODE_OUTBITVALUE        OUTMOD_0
#define TIMERB_OUTPUTMODE_SET                OUTMOD_1
#define TIMERB_OUTPUTMODE_TOGGLE_RESET       OUTMOD_2
#define TIMERB_OUTPUTMODE_SET_RESET          OUTMOD_3
#define TIMERB_OUTPUTMODE_TOGGLE             OUTMOD_4
#define TIMERB_OUTPUTMODE_RESET              OUTMOD_5
#define TIMERB_OUTPUTMODE_TOGGLE_SET         OUTMOD_6
#define TIMERB_OUTPUTMODE_RESET_SET          OUTMOD_7

//*****************************************************************************
//
//The following are values can be passed to the
//captureMode parameter of TimerB_initCapture(); API
//
//*****************************************************************************
#define TIMERB_CAPTUREMODE_NO_CAPTURE                CM_0
#define TIMERB_CAPTUREMODE_RISING_EDGE               CM_1
#define TIMERB_CAPTUREMODE_FALLING_EDGE              CM_2
#define TIMERB_CAPTUREMODE_RISING_AND_FALLING_EDGE   CM_3

//*****************************************************************************
//
//The following are values can be passed to the
//synchronizeCaptureSource parameter of TimerB_initCapture(); API
//
//*****************************************************************************
#define TIMERB_CAPTURE_ASYNCHRONOUS                  0x00
#define TIMERB_CAPTURE_SYNCHRONOUS                   SCS

//*****************************************************************************
//
//The following are values can be passed to the
//captureInterruptEnable, compareInterruptEnable parameter of
//TimerB_initCapture(); API
//
//*****************************************************************************
#define TIMERB_CAPTURECOMPARE_INTERRUPT_ENABLE       CCIE
#define TIMERB_CAPTURECOMPARE_INTERRUPT_DISABLE      0x00

//*****************************************************************************
//
//The following are values can be passed to the
//captureInputSelect parameter of TimerB_initCapture(); API
//
//*****************************************************************************
#define TIMERB_CAPTURE_INPUTSELECT_CCIxA             CCIS_0
#define TIMERB_CAPTURE_INPUTSELECT_CCIxB             CCIS_1
#define TIMERB_CAPTURE_INPUTSELECT_GND               CCIS_2
#define TIMERB_CAPTURE_INPUTSELECT_Vcc               CCIS_3

//*****************************************************************************
//
//The following are values can be passed to the
//counterLength parameter of TimerB_selectCounterLength(); API
//
//*****************************************************************************
#define TIMERB_COUNTER_8BIT CNTL_0
#define TIMERB_COUNTER_10BIT CNTL_1
#define TIMERB_COUNTER_12BIT CNTL_2
#define TIMERB_COUNTER_16BIT CNTL_3

//*****************************************************************************
//
//The following are values can be passed to the
//groupLatch parameter of TimerB_selectLatchingGroup(); API
//
//*****************************************************************************
#define TIMERB_GROUP_NONE				TBCLGRP_0
#define TIMERB_GROUP_CL12_CL23_CL56	TBCLGRP_1
#define TIMERB_GROUP_CL123_CL456		TBCLGRP_2
#define TIMERB_GROUP_ALL				TBCLGRP_3


//*****************************************************************************
//
//The following are values can be passed to the
//compareLatchLoadEvent parameter of TimerB_initCompareLatchLoadEvent(); API
//
//*****************************************************************************
#define TIMERB_LATCH_ON_WRITE_TO_TBxCCRn_COMPARE_REGISTER    		CLLD_0
#define TIMERB_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UP_OR_CONT_MODE	CLLD_1
#define TIMERB_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UPDOWN_MODE 		CLLD_2
#define TIMERB_LATCH_WHEN_COUNTER_COUNTS_TO_CURRENT_COMPARE_LATCH_VALUE  CLLD_3


//*****************************************************************************
//
//The following are values that may be returned by
//TimerB_getInterruptStatus(); API
//
//*****************************************************************************
#define TIMERB_INTERRUPT_NOT_PENDING     0x00
#define TIMERB_INTERRUPT_PENDING         0x01

//*****************************************************************************
//
//The following are values can be passed to the
//synchronized parameter of TimerB_getSynchronizedCaptureCompareInput(); API
//
//*****************************************************************************
#define TIMERB_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT SCCI
#define TIMERB_READ_CAPTURE_COMPARE_INPUT            CCI

//*****************************************************************************
//
//Prototypes for the APIs.
//
//*****************************************************************************
extern void TimerB_startCounter ( unsigned int baseAddress,
    unsigned int timerMode
    );
extern void TimerB_configureContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int timerClear
    );
extern void TimerB_configureUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    );
extern void TimerB_configureUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    );
extern void TimerB_startContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int timerClear
    );
extern void TimerB_startContinousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int timerClear
    );
extern void TimerB_startUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    );
extern void TimerB_startUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    );
extern void TimerB_initCapture (unsigned int baseAddress,
    unsigned int captureRegister,
    unsigned int captureMode,
    unsigned int captureInputSelect,
    unsigned short synchronizeCaptureSource,
    unsigned short captureInterruptEnable,
    unsigned int captureOutputMode
    );
extern void TimerB_initCompare (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned short compareInterruptEnable,
    unsigned int compareOutputMode,
    unsigned int compareValue
    );
extern void TimerB_enableInterrupt (unsigned int baseAddress);
extern void TimerB_disableInterrupt (unsigned int baseAddress);
extern unsigned long TimerB_getInterruptStatus (unsigned int baseAddress);
extern void TimerB_enableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern void TimerB_disableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern unsigned long TimerB_getCaptureCompareInterruptStatus (unsigned int baseAddress,
		 unsigned int captureCompareRegister,
		 unsigned int mask
		 );
extern void TimerB_clear (unsigned int baseAddress);
unsigned short TimerB_getSynchronizedCaptureCompareInput
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned short synchronized
    );
extern unsigned char TimerB_getOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern unsigned int TimerB_getCaptureCompareCount
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern void TimerB_setOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned char outputModeOutBitValue
    );
extern void TimerB_generatePWM (  unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int compareRegister,
    unsigned int compareOutputMode,
    unsigned int dutyCycle
    );
extern void TimerB_stop ( unsigned int baseAddress );
extern void privateTimerBProcessClockSourceDivider (unsigned int baseAddress,
    unsigned int clockSourceDivider);
extern void TimerB_setCompareValue (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned int compareValue
    );
extern void TimerB_clearTimerInterruptFlag (unsigned int baseAddress);
extern void TimerB_clearCaptureCompareInterruptFlag (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern void TimerB_selectCounterLength (unsigned int  baseAddress,
		unsigned int counterLength
		);
extern void TimerB_selectLatchingGroup(unsigned int  baseAddress,
		unsigned int  groupLatch);
extern void TimerB_initCompareLatchLoadEvent(unsigned int  baseAddress,
		unsigned int  compareRegister,
		unsigned int  compareLatchLoadEvent
		);


#endif
