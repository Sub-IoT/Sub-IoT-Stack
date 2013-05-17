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
#ifndef __MSP430WARE_TIMERD_H__
#define __MSP430WARE_TIMERD_H__

#define __MSP430_HAS_TxD7__
//*****************************************************************************
//
//The following are values that can be passed to the
//TimerD_startContinuousMode(), TimerD_startUpMode(), TimerD_startUpDownMode(),
//TimerD_generatePWM() APIs as the clockSource parameter.
//
//*****************************************************************************
#define TIMERD_CLOCKSOURCE_EXTERNAL_TDCLK            TDSSEL__TACLK
#define TIMERD_CLOCKSOURCE_ACLK                      TDSSEL__ACLK
#define TIMERD_CLOCKSOURCE_SMCLK                     TDSSEL__SMCLK
#define TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK   TDSSEL__INCLK

//*****************************************************************************
//
//The following are values that can be passed to the
//TimerD_startContinuousMode(), TimerD_startUpMode(), TimerD_startUpDownMode(),
//TimerD_generatePWM() APIs as the clockSource parameter.
//
//*****************************************************************************
#define TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK 		    	TDCLKM_0
#define TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK			TDCLKM_1
#define TIMERD_CLOCKINGMODE_AUXILIARY_CLK			TDCLKM_2

//*****************************************************************************
//
//The following are values that can be passed to the
//TimerD_configureHighResGeneratorInRegulatedMode()
// API as the highResMultiplyFactor parameter.
//
//*****************************************************************************
#define TIMERD_HIGHRES_CLK_MULTIPLY_FACTOR_8x	TDHM_0
#define TIMERD_HIGHRES_CLK_MULTIPLY_FACTOR_16x	TDHM_1

//*****************************************************************************
//
//The following are values that can be passed to the
//TimerD_startContinuousMode(), TimerD_startUpMode(), TimerD_startUpDownMode(),
//TimerD_generatePWM() APIs as the clockSourceDivider parameter.
//
//*****************************************************************************
#define TIMERD_CLOCKSOURCE_DIVIDER_1     0x01
#define TIMERD_CLOCKSOURCE_DIVIDER_2     0x02
#define TIMERD_CLOCKSOURCE_DIVIDER_4     0x04
#define TIMERD_CLOCKSOURCE_DIVIDER_8     0x08
#define TIMERD_CLOCKSOURCE_DIVIDER_3     0x03
#define TIMERD_CLOCKSOURCE_DIVIDER_5     0x05
#define TIMERD_CLOCKSOURCE_DIVIDER_6     0x06
#define TIMERD_CLOCKSOURCE_DIVIDER_7     0x07
#define TIMERD_CLOCKSOURCE_DIVIDER_10    0x0A
#define TIMERD_CLOCKSOURCE_DIVIDER_12    0x0C
#define TIMERD_CLOCKSOURCE_DIVIDER_14    0x0E
#define TIMERD_CLOCKSOURCE_DIVIDER_16    0x10
#define TIMERD_CLOCKSOURCE_DIVIDER_20    0x14
#define TIMERD_CLOCKSOURCE_DIVIDER_24    0x18
#define TIMERD_CLOCKSOURCE_DIVIDER_28    0x1C
#define TIMERD_CLOCKSOURCE_DIVIDER_32    0x20
#define TIMERD_CLOCKSOURCE_DIVIDER_40    0x28
#define TIMERD_CLOCKSOURCE_DIVIDER_48    0x30
#define TIMERD_CLOCKSOURCE_DIVIDER_56    0x38
#define TIMERD_CLOCKSOURCE_DIVIDER_64    0x40

//*****************************************************************************
//
//The following are values that can be passed to TimerD_startContinuousMode()
//TimerD_startUpMode(),  TimerD_startUpDownMode() as the timerClear parameter.
//
//*****************************************************************************
#define TIMERD_DO_CLEAR      TDCLR
#define TIMERD_SKIP_CLEAR    0x00

//*****************************************************************************
//
//The following are values that can be passed to the
//TimerD_getSynchronizedCaptureCompareInput() API as the synchronized
//parameter.
//
//*****************************************************************************
#define TIMERD_CAPTURECOMPARE_INPUT     CCI

//*****************************************************************************
//
//The following are values that is returned by the
//TimerD_getSynchronizedCaptureCompareInput() API
//
//*****************************************************************************
#define TIMERD_CAPTURECOMPARE_INPUT_HIGH    0x01
#define TIMERD_CAPTURECOMPARE_INPUT_LOW     0x00


//*****************************************************************************
//
//The following are values that is returned by the
//TimerD_getOutputForOutputModeOutBitValue() and passed to
//TimerD_setOutputForOutputModeOutBitValue() as
//outputModeOutBitValue parameter
//
//*****************************************************************************
#define TIMERD_OUTPUTMODE_OUTBITVALUE_HIGH    OUT
#define TIMERD_OUTPUTMODE_OUTBITVALUE_LOW     0x00

//*****************************************************************************
//
//The following are values can be passed to the mask parameter of
//TimerD_captureCompareInterruptStatus() API
//
//*****************************************************************************
#define TIMERD_CAPTURE_OVERFLOW                  COV
#define TIMERD_CAPTURECOMPARE_INTERRUPT_FLAG     CCIFG

//*****************************************************************************
//
//The following are values can be passed to the mask parameter of
//TimerD_enableHighResInterrupt(), TimerD_getHighResInterruptStatus(),
// TimerD_disableHighResInterrupt() API
//
//*****************************************************************************
#define TIMERD_HIGH_RES_FREQUENCY_UNLOCK   TDHUNLKIE
#define TIMERD_HIGH_RES_FREQUENCY_LOCK     TDHLKIE
#define TIMERD_HIGH_RES_FAIL_HIGH          TDHFHIE
#define TIMERD_HIGH_RES_FAIL_LOW     	  TDHFLIE

//*****************************************************************************
//
//The following are values can be passed to the timerInterruptEnable_TDIE
//parameter of TimerD_startContinuousMode(), TimerD_startUpMode(),
//TimerD_startUpDownMode()
//
//*****************************************************************************
#define TIMERD_TDIE_INTERRUPT_ENABLE            TDIE
#define TIMERD_TDIE_INTERRUPT_DISABLE           0x00

//*****************************************************************************
//
//The following are values can be passed to the
//captureCompareInterruptEnable_CCR0_CCIE parameter of TimerD_startUpMode(),
//TimerD_startUpDownMode API
//
//*****************************************************************************
#define TIMERD_CCIE_CCR0_INTERRUPT_ENABLE   CCIE
#define TIMERD_CCIE_CCR0_INTERRUPT_DISABLE  0x00

//*****************************************************************************
//
//The following are timer modes possible.
//
//*****************************************************************************
#define TIMERD_STOP_MODE         MC_0
#define TIMERD_UP_MODE           MC_1
#define TIMERD_CONTINUOUS_MODE   MC_2
#define TIMERD_UPDOWN_MODE       MC_3

//*****************************************************************************
//
//The following are values can be passed to the
//compareRegister, captureCompareRegister or captureRegister parameter
//of TimerD_initCapture(), TimerD_enableCaptureCompareInterrupt(),
//TimerD_disableCaptureCompareInterrupt(),TimerD_captureCompareInterruptStatus(),
//TimerD_getSynchronizedCaptureCompareInput(),TimerD_initCompare()
//
//*****************************************************************************
#define TIMERD_CAPTURECOMPARE_REGISTER_0     0x08
#define TIMERD_CAPTURECOMPARE_REGISTER_1     0x0E
#define TIMERD_CAPTURECOMPARE_REGISTER_2     0x14
#define TIMERD_CAPTURECOMPARE_REGISTER_3     0x1A
#define TIMERD_CAPTURECOMPARE_REGISTER_4     0x20
#define TIMERD_CAPTURECOMPARE_REGISTER_5     0x28
#define TIMERD_CAPTURECOMPARE_REGISTER_6     0x2E

//*****************************************************************************
//
//The following are values can be passed to the
//captureMode parameter of TimerD_initCompare(), TimerD_initCapture(),
//TimerD_generatePWM(),
//
//*****************************************************************************
#define TIMERD_OUTPUTMODE_OUTBITVALUE        OUTMOD_0
#define TIMERD_OUTPUTMODE_SET                OUTMOD_1
#define TIMERD_OUTPUTMODE_TOGGLE_RESET       OUTMOD_2
#define TIMERD_OUTPUTMODE_SET_RESET          OUTMOD_3
#define TIMERD_OUTPUTMODE_TOGGLE             OUTMOD_4
#define TIMERD_OUTPUTMODE_RESET              OUTMOD_5
#define TIMERD_OUTPUTMODE_TOGGLE_SET         OUTMOD_6
#define TIMERD_OUTPUTMODE_RESET_SET          OUTMOD_7

//*****************************************************************************
//
//The following are values can be passed to the
//captureMode parameter of TimerD_initCapture() API
//
//*****************************************************************************
#define TIMERD_CAPTUREMODE_NO_CAPTURE                CM_0
#define TIMERD_CAPTUREMODE_RISING_EDGE               CM_1
#define TIMERD_CAPTUREMODE_FALLING_EDGE              CM_2
#define TIMERD_CAPTUREMODE_RISING_AND_FALLING_EDGE   CM_3

//*****************************************************************************
//
//The following are values can be passed to the
//synchronizeCaptureSource parameter of TimerD_initCapture() API
//
//*****************************************************************************
#define TIMERD_CAPTURE_ASYNCHRONOUS                  0x00
#define TIMERD_CAPTURE_SYNCHRONOUS                   SCS

//*****************************************************************************
//
//The following are values can be passed to the
//channelCaptureMode parameter of TimerD_initCapture() API
//
//*****************************************************************************
#define TIMERD_SINGLE_CAPTURE_MODE                 0x00
#define TIMERD_DUAL_CAPTURE_MODE                   0x01
//*****************************************************************************
//
//The following are values can be passed to the
//captureInterruptEnable, compareInterruptEnable parameter of
//TimerD_initCapture() API
//
//*****************************************************************************
#define TIMERD_CAPTURECOMPARE_INTERRUPT_ENABLE       CCIE
#define TIMERD_CAPTURECOMPARE_INTERRUPT_DISABLE      0x00

//*****************************************************************************
//
//The following are values can be passed to the
//captureInputSelect parameter of TimerD_initCapture() API
//
//*****************************************************************************
#define TIMERD_CAPTURE_INPUTSELECT_CCIxA             CCIS_0
#define TIMERD_CAPTURE_INPUTSELECT_CCIxB             CCIS_1
#define TIMERD_CAPTURE_INPUTSELECT_GND               CCIS_2
#define TIMERD_CAPTURE_INPUTSELECT_Vcc               CCIS_3

//*****************************************************************************
//
//The following are values that may be returned by
//TimerD_getInterruptStatus() API
//
//*****************************************************************************
#define TIMERD_INTERRUPT_NOT_PENDING     0x00
#define TIMERD_INTERRUPT_PENDING         0x01

//*****************************************************************************
//
//The following are values can be passed to the
//synchronized parameter of TimerD_getSynchronizedCaptureCompareInput() API
//
//*****************************************************************************
#define TIMERD_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT SCCI
#define TIMERD_READ_CAPTURE_COMPARE_INPUT            CCI

//*****************************************************************************
//
//The following are values can be passed to the
//desiredHighResFrequency parameter of 
//TimerD_configureHighResGeneratorInFreeRunningMode() API
//
//*****************************************************************************
#define TIMERD_HIGHRES_64MHZ	0x00
#define TIMERD_HIGHRES_128MHZ	0x01
#define TIMERD_HIGHRES_200MHZ	0x02
#define TIMERD_HIGHRES_256MHZ	0x03   

//*****************************************************************************
//
//The following are values can be passed to the
//TDOutput parameter of 
//TimerD_combineTDCCRToGeneratePWM() API
//
//*****************************************************************************
#define TIMERD_COMBINE_CCR1_CCR2	2
#define TIMERD_COMBINE_CCR3_CCR4	4
#define TIMERD_COMBINE_CCR5_CCR6	6

//*****************************************************************************
//
//The following are values can be passed to the highResCoarseClockRange
//parameter of  TimerD_selectHighResClockRange() API
//
//*****************************************************************************
#define TIMERD_CLOCK_RANGE0 		0x0000
#define TIMERD_CLOCK_RANGE1			0x2000
#define TIMERD_CLOCK_RANGE2			0x4000

//*****************************************************************************
//
//The following are values can be passed to the
//TDOutput parameter of
//TimerD_selectHighResCoarseClockRange() API
//
//*****************************************************************************
#define TIMERD_HIGHRES_BELOW_15MHz  0x00
#define TIMERD_HIGHRES_ABOVE_15MHz  TDHCLKCR




//*****************************************************************************
//
//Prototypes for the APIs.
//
//*****************************************************************************
extern void TimerD_startCounter ( unsigned int baseAddress,
    unsigned int timerMode
    );
extern void TimerD_configureContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int clockingMode,
    unsigned int timerInterruptEnable_TDIE,
    unsigned int timerClear
    );
extern void TimerD_configureUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int clockingMode,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TDIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    );
extern void TimerD_configureUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int clockingMode,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TDIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    );
extern void TimerD_initCapture (unsigned int baseAddress,
    unsigned int captureRegister,
    unsigned int captureMode,
    unsigned int captureInputSelect,
    unsigned short synchronizeCaptureSource,
    unsigned short captureInterruptEnable,
    unsigned int captureOutputMode,
    unsigned char channelCaptureMode
    );
extern void TimerD_initCompare (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned short compareInterruptEnable,
    unsigned int compareOutputMode,
    unsigned int compareValue
    );
extern void TimerD_enableTimerInterrupt (unsigned int baseAddress);
extern void TimerD_enableHighResInterrupt (unsigned int baseAddress,
				unsigned int mask);
extern void TimerD_disableTimerInterrupt (unsigned int baseAddress);

extern void TimerD_disableHighResInterrupt (unsigned int baseAddress,
						unsigned int mask);
extern unsigned long TimerD_getTimerInterruptStatus (unsigned int baseAddress);
extern void TimerD_enableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern void TimerD_disableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern unsigned long TimerD_getCaptureCompareInterruptStatus (unsigned int baseAddress,
	    unsigned int captureCompareRegister,
	    unsigned int mask
	    );
extern unsigned int TimerD_getHighResInterruptStatus (unsigned int baseAddress,
    unsigned int mask);

extern void TimerD_clear (unsigned int baseAddress);
extern void TimerD_clearHighResInterruptStatus (unsigned int baseAddress,
    unsigned int mask);
extern unsigned short TimerD_getSynchronizedCaptureCompareInput
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned short synchronized
    );
extern unsigned char TimerD_getOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern unsigned int TimerD_getCaptureCompareCount
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern unsigned int TimerD_getCaptureCompareLatchCount
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern unsigned char TimerD_getCaptureCompareInputSignal
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern void TimerD_setOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned char outputModeOutBitValue
    );
extern void TimerD_generatePWM (  unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int clockingMode,
    unsigned int timerPeriod,
    unsigned int compareRegister,
    unsigned int compareOutputMode,
    unsigned int dutyCycle
    );
extern void TimerD_stop ( unsigned int baseAddress );
extern void TimerD_setCompareValue (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned int compareValue
    );
extern void TimerD_clearTimerInterruptFlag (unsigned int baseAddress);
extern void TimerD_clearCaptureCompareInterruptFlag (unsigned int baseAddress,
    unsigned int captureCompareRegister
    );
extern unsigned char TimerD_configureHighResGeneratorInFreeRunningMode
	(unsigned int baseAddress,
    unsigned char desiredHighResFrequency
    );
extern void TimerD_configureHighResGeneratorInRegulatedMode (unsigned int baseAddress,
     unsigned int clockSource, 
     unsigned int clockSourceDivider,
     unsigned int clockingMode, 
     unsigned char highResClockMultiplyFactor,
     unsigned char highResClockDivider
    );
extern void TimerD_combineTDCCRToGeneratePWM (  unsigned int baseAddress,
	    unsigned int clockSource,
	    unsigned int clockSourceDivider,
            unsigned int clockingMode,
	    unsigned int timerPeriod,
	    unsigned int combineCCRRegistersCombination,
	    unsigned int compareOutputMode,
	    unsigned int dutyCycle1,
	    unsigned int dutyCycle2
	    );
extern void TimerD_selectLatchingGroup(unsigned int  baseAddress,
		unsigned int  groupLatch);
extern void TimerD_selectCounterLength (unsigned int  baseAddress,
		unsigned int counterLength
		);
extern void TimerD_initCompareLatchLoadEvent(unsigned int  baseAddress,
		unsigned int  compareRegister,
		unsigned int  compareLatchLoadEvent
		);
extern void TimerD_disableHighResFastWakeup (unsigned int baseAddress);
extern void TimerD_enableHighResFastWakeup (unsigned int baseAddress);
extern void TimerD_disableHighResClockEnhancedAccuracy (unsigned int baseAddress);
extern void TimerD_enableHighResClockEnhancedAccuracy (unsigned int baseAddress);
extern void TimerD_DisableHighResGeneratorForceON (unsigned int baseAddress);
extern void TimerD_EnableHighResGeneratorForceON (unsigned int baseAddress);

extern void TimerD_selectHighResCoarseClockRange (unsigned int baseAddress,
		unsigned int highResCoarseClockRange
		);
extern void TimerD_selectHighResClockRange (unsigned int baseAddress,
		unsigned int highResClockRange
		);

//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************

#endif
