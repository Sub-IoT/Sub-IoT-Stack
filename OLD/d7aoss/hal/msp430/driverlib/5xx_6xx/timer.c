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
//! DEPRECATED FILE - Replaced by timera.c and timerb.c

//*****************************************************************************
//
//timer.c - Driver for the TIMER Module.
//
//*****************************************************************************
#include "../inc/hw_types.h"
#include "debug.h"
#include "timer.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "../deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif

void privateTimerProcessClockSourceDivider (unsigned int baseAddress,
    unsigned int clockSourceDivider);

//*****************************************************************************
//
//! Starts timer in continuous mode.
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMER_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMER_CLOCKSOURCE_ACLK
//!         \b TIMER_CLOCKSOURCE_SMCLK
//!         \b TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMER_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMER_CLOCKSOURCE_DIVIDER_2
//!        \b TIMER_CLOCKSOURCE_DIVIDER_4
//!        \b TIMER_CLOCKSOURCE_DIVIDER_8
//!        \b TIMER_CLOCKSOURCE_DIVIDER_3
//!        \b TIMER_CLOCKSOURCE_DIVIDER_5
//!        \b TIMER_CLOCKSOURCE_DIVIDER_6
//!        \b TIMER_CLOCKSOURCE_DIVIDER_7
//!        \b TIMER_CLOCKSOURCE_DIVIDER_10
//!        \b TIMER_CLOCKSOURCE_DIVIDER_12
//!        \b TIMER_CLOCKSOURCE_DIVIDER_14
//!        \b TIMER_CLOCKSOURCE_DIVIDER_16
//!        \b TIMER_CLOCKSOURCE_DIVIDER_20
//!        \b TIMER_CLOCKSOURCE_DIVIDER_24
//!        \b TIMER_CLOCKSOURCE_DIVIDER_28
//!        \b TIMER_CLOCKSOURCE_DIVIDER_32
//!        \b TIMER_CLOCKSOURCE_DIVIDER_40
//!        \b TIMER_CLOCKSOURCE_DIVIDER_48
//!        \b TIMER_CLOCKSOURCE_DIVIDER_56
//!        \b TIMER_CLOCKSOURCE_DIVIDER_64
//! \param timerInterruptEnable_TAIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMER_TAIE_INTERRUPT_ENABLE
//!        \b TIMER_TAIE_INTERRUPT_DISABLE [Default value]
//! \param timerclear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMER_DO_CLEAR
//!        \b TIMER_SKIP_CLEAR [Default value]
//!
//! Modified reister is \b TAxCTL
//!
//! \return None
//
//*****************************************************************************
void Timer_startContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int timerclear
    )
{
    ASSERT(
        (TIMER_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMER_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMER_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMER_DO_CLEAR == timerclear) ||
        (TIMER_SKIP_CLEAR == timerclear)
        );

    ASSERT(
        (TIMER_TAIE_INTERRUPT_ENABLE == timerInterruptEnable_TAIE) ||
        (TIMER_TAIE_INTERRUPT_DISABLE == timerInterruptEnable_TAIE)
        );

    ASSERT(
        (TIMER_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMER_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );


    privateTimerProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress +
        OFS_TAxCTL) &= ~(TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
                         TIMER_UPDOWN_MODE +
                         TIMER_DO_CLEAR +
                         TIMER_TAIE_INTERRUPT_ENABLE
                         );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource + TIMER_CONTINUOUS_MODE +
                                          timerclear +
                                          timerInterruptEnable_TAIE);
}

//*****************************************************************************
//
//! DEPRECATED- Spelling Error Fixed
//! Starts timer in continuous mode.
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMER_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMER_CLOCKSOURCE_ACLK
//!         \b TIMER_CLOCKSOURCE_SMCLK
//!         \b TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMER_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMER_CLOCKSOURCE_DIVIDER_2
//!        \b TIMER_CLOCKSOURCE_DIVIDER_4
//!        \b TIMER_CLOCKSOURCE_DIVIDER_8
//!        \b TIMER_CLOCKSOURCE_DIVIDER_3
//!        \b TIMER_CLOCKSOURCE_DIVIDER_5
//!        \b TIMER_CLOCKSOURCE_DIVIDER_6
//!        \b TIMER_CLOCKSOURCE_DIVIDER_7
//!        \b TIMER_CLOCKSOURCE_DIVIDER_10
//!        \b TIMER_CLOCKSOURCE_DIVIDER_12
//!        \b TIMER_CLOCKSOURCE_DIVIDER_14
//!        \b TIMER_CLOCKSOURCE_DIVIDER_16
//!        \b TIMER_CLOCKSOURCE_DIVIDER_20
//!        \b TIMER_CLOCKSOURCE_DIVIDER_24
//!        \b TIMER_CLOCKSOURCE_DIVIDER_28
//!        \b TIMER_CLOCKSOURCE_DIVIDER_32
//!        \b TIMER_CLOCKSOURCE_DIVIDER_40
//!        \b TIMER_CLOCKSOURCE_DIVIDER_48
//!        \b TIMER_CLOCKSOURCE_DIVIDER_56
//!        \b TIMER_CLOCKSOURCE_DIVIDER_64
//! \param timerInterruptEnable_TAIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMER_TAIE_INTERRUPT_ENABLE
//!        \b TIMER_TAIE_INTERRUPT_DISABLE [Default value]
//! \param timerclear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMER_DO_CLEAR
//!        \b TIMER_SKIP_CLEAR [Default value]
//!
//! Modified reister is \b TAxCTL
//!
//! \return None
//
//*****************************************************************************
void Timer_startContinousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int timerclear
    )
{
	Timer_startContinuousMode (baseAddress,
    clockSource,
    clockSourceDivider,
    timerInterruptEnable_TAIE,
    timerclear
    );
}
//*****************************************************************************
//
//! Starts timer in up mode.
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMER_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMER_CLOCKSOURCE_ACLK
//!         \b TIMER_CLOCKSOURCE_SMCLK
//!         \b TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMER_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMER_CLOCKSOURCE_DIVIDER_2
//!        \b TIMER_CLOCKSOURCE_DIVIDER_4
//!        \b TIMER_CLOCKSOURCE_DIVIDER_8
//!        \b TIMER_CLOCKSOURCE_DIVIDER_3
//!        \b TIMER_CLOCKSOURCE_DIVIDER_5
//!        \b TIMER_CLOCKSOURCE_DIVIDER_6
//!        \b TIMER_CLOCKSOURCE_DIVIDER_7
//!        \b TIMER_CLOCKSOURCE_DIVIDER_10
//!        \b TIMER_CLOCKSOURCE_DIVIDER_12
//!        \b TIMER_CLOCKSOURCE_DIVIDER_14
//!        \b TIMER_CLOCKSOURCE_DIVIDER_16
//!        \b TIMER_CLOCKSOURCE_DIVIDER_20
//!        \b TIMER_CLOCKSOURCE_DIVIDER_24
//!        \b TIMER_CLOCKSOURCE_DIVIDER_28
//!        \b TIMER_CLOCKSOURCE_DIVIDER_32
//!        \b TIMER_CLOCKSOURCE_DIVIDER_40
//!        \b TIMER_CLOCKSOURCE_DIVIDER_48
//!        \b TIMER_CLOCKSOURCE_DIVIDER_56
//!        \b TIMER_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod is the specified timer period. This is the value that gets
//!         written into the CCR0. Limited to 16 bits[unsigned int]
//! \param timerInterruptEnable_TAIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMER_TAIE_INTERRUPT_ENABLE and
//!        \b TIMER_TAIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         timer CCR0 captureComapre interrupt. Valid values are
//!        \b TIMER_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMER_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerclear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMER_DO_CLEAR
//!        \b TIMER_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TAxCTL, \b TAxCCR0, \b TAxCCTL0
//!
//! \return None
//
//*****************************************************************************
void Timer_startUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerclear
    )
{
    ASSERT(
        (TIMER_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMER_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMER_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMER_DO_CLEAR == timerclear) ||
        (TIMER_SKIP_CLEAR == timerclear)
        );

    ASSERT(
        (TIMER_DO_CLEAR == timerclear) ||
        (TIMER_SKIP_CLEAR == timerclear)
        );

    privateTimerProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TAxCTL) &=
        ~(TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMER_UPDOWN_MODE +
          TIMER_DO_CLEAR +
          TIMER_TAIE_INTERRUPT_ENABLE
          );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource +
                                          TIMER_UP_MODE +
                                          timerclear +
                                          timerInterruptEnable_TAIE
                                          );

    if (TIMER_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TAxCCTL0)  |= TIMER_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TAxCCTL0)  &= ~TIMER_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TAxCCR0)  = timerPeriod;
}

//*****************************************************************************
//
//! Starts timer in up down mode.
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMER_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMER_CLOCKSOURCE_ACLK
//!         \b TIMER_CLOCKSOURCE_SMCLK
//!         \b TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMER_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMER_CLOCKSOURCE_DIVIDER_2
//!        \b TIMER_CLOCKSOURCE_DIVIDER_4
//!        \b TIMER_CLOCKSOURCE_DIVIDER_8
//!        \b TIMER_CLOCKSOURCE_DIVIDER_3
//!        \b TIMER_CLOCKSOURCE_DIVIDER_5
//!        \b TIMER_CLOCKSOURCE_DIVIDER_6
//!        \b TIMER_CLOCKSOURCE_DIVIDER_7
//!        \b TIMER_CLOCKSOURCE_DIVIDER_10
//!        \b TIMER_CLOCKSOURCE_DIVIDER_12
//!        \b TIMER_CLOCKSOURCE_DIVIDER_14
//!        \b TIMER_CLOCKSOURCE_DIVIDER_16
//!        \b TIMER_CLOCKSOURCE_DIVIDER_20
//!        \b TIMER_CLOCKSOURCE_DIVIDER_24
//!        \b TIMER_CLOCKSOURCE_DIVIDER_28
//!        \b TIMER_CLOCKSOURCE_DIVIDER_32
//!        \b TIMER_CLOCKSOURCE_DIVIDER_40
//!        \b TIMER_CLOCKSOURCE_DIVIDER_48
//!        \b TIMER_CLOCKSOURCE_DIVIDER_56
//!        \b TIMER_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod is the specified timer period
//! \param timerInterruptEnable_TAIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMER_TAIE_INTERRUPT_ENABLE
//!        \b TIMER_TAIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         timer CCR0 captureComapre interrupt. Valid values are
//!        \b TIMER_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMER_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerclear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMER_DO_CLEAR
//!        \b TIMER_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TAxCTL, \b TAxCCR0, \b TAxCCTL0
//!
//! \return None
//
//*****************************************************************************
void Timer_startUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerclear
    )
{
    ASSERT(
        (TIMER_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMER_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMER_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMER_DO_CLEAR == timerclear) ||
        (TIMER_SKIP_CLEAR == timerclear)
        );

    ASSERT(
        (TIMER_DO_CLEAR == timerclear) ||
        (TIMER_SKIP_CLEAR == timerclear)
        );

    privateTimerProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TAxCTL) &=
        ~(TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMER_UPDOWN_MODE +
          TIMER_DO_CLEAR +
          TIMER_TAIE_INTERRUPT_ENABLE
          );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource +
                                          TIMER_UPDOWN_MODE +
                                          timerclear +
                                          timerInterruptEnable_TAIE
                                          );
    if (TIMER_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TAxCCTL0)  |= TIMER_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TAxCCTL0)  &= ~TIMER_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TAxCCR0)  = timerPeriod;
}

//*****************************************************************************
//
//! Initializes Capture Mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMER_CAPTURECOMPARE_REGISTER_0
//!     \b TIMER_CAPTURECOMPARE_REGISTER_1
//!     \b TIMER_CAPTURECOMPARE_REGISTER_2
//!     \b TIMER_CAPTURECOMPARE_REGISTER_3
//!     \b TIMER_CAPTURECOMPARE_REGISTER_4
//!     \b TIMER_CAPTURECOMPARE_REGISTER_5
//!     \b TIMER_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param captureMode is the capture mode selected. Valid values are
//!        \b TIMER_CAPTUREMODE_NO_CAPTURE [Default value]
//!        \b TIMER_CAPTUREMODE_RISING_EDGE
//!        \b TIMER_CAPTUREMODE_FALLING_EDGE
//!        \b TIMER_CAPTUREMODE_RISING_AND_FALLING_EDGE
//! \param captureInputSelect decides the Input Select
//!        \b TIMER_CAPTURE_INPUTSELECT_CCIxA [Default value]
//!        \b TIMER_CAPTURE_INPUTSELECT_CCIxB
//!        \b TIMER_CAPTURE_INPUTSELECT_GND
//!        \b TIMER_CAPTURE_INPUTSELECT_Vcc
//! \param synchronizeCaptureSource decides if capture source should be
//!         synchronized with timer clock
//!        Valid values are
//!        \b TIMER_CAPTURE_ASYNCHRONOUS [Default value]
//!        \b TIMER_CAPTURE_SYNCHRONOUS
//! \param captureInterruptEnable is to enable or disable
//!         timer captureComapre interrupt. Valid values are
//!        \b TIMER_CAPTURECOMPARE_INTERRUPT_DISABLE [Default value]
//!        \b TIMER_CAPTURECOMPARE_INTERRUPT_ENABLE
//! \param captureOutputMode specifies the ouput mode. Valid values are
//!        \b TIMER_OUTPUTMODE_OUTBITVALUE [Default value],
//!        \b TIMER_OUTPUTMODE_SET,
//!        \b TIMER_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMER_OUTPUTMODE_SET_RESET
//!        \b TIMER_OUTPUTMODE_TOGGLE,
//!        \b TIMER_OUTPUTMODE_RESET,
//!        \b TIMER_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMER_OUTPUTMODE_RESET_SET
//!
//! Modified register is \b TAxCCTLn
//! \return None
//
//*****************************************************************************
void Timer_initCapture (unsigned int baseAddress,
    unsigned int captureRegister,
    unsigned int captureMode,
    unsigned int captureInputSelect,
    unsigned short synchronizeCaptureSource,
    unsigned short captureInterruptEnable,
    unsigned int captureOutputMode
    )
{
    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == captureRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == captureRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == captureRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == captureRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == captureRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == captureRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == captureRegister) 
        );

    ASSERT((TIMER_CAPTUREMODE_NO_CAPTURE == captureMode) ||
        (TIMER_CAPTUREMODE_RISING_EDGE == captureMode) ||
        (TIMER_CAPTUREMODE_FALLING_EDGE == captureMode) ||
        (TIMER_CAPTUREMODE_RISING_AND_FALLING_EDGE == captureMode)
        );

    ASSERT((TIMER_CAPTURE_INPUTSELECT_CCIxA == captureInputSelect) ||
        (TIMER_CAPTURE_INPUTSELECT_CCIxB == captureInputSelect) ||
        (TIMER_CAPTURE_INPUTSELECT_GND == captureInputSelect) ||
        (TIMER_CAPTURE_INPUTSELECT_Vcc == captureInputSelect)
        );

    ASSERT((TIMER_CAPTURE_ASYNCHRONOUS == synchronizeCaptureSource) ||
        (TIMER_CAPTURE_SYNCHRONOUS == synchronizeCaptureSource)
        );

    ASSERT(
        (TIMER_CAPTURECOMPARE_INTERRUPT_DISABLE == captureInterruptEnable) ||
        (TIMER_CAPTURECOMPARE_INTERRUPT_ENABLE == captureInterruptEnable)
        );

    ASSERT((TIMER_OUTPUTMODE_OUTBITVALUE == captureOutputMode) ||
        (TIMER_OUTPUTMODE_SET == captureOutputMode) ||
        (TIMER_OUTPUTMODE_TOGGLE_RESET == captureOutputMode) ||
        (TIMER_OUTPUTMODE_SET_RESET == captureOutputMode) ||
        (TIMER_OUTPUTMODE_TOGGLE == captureOutputMode) ||
        (TIMER_OUTPUTMODE_RESET == captureOutputMode) ||
        (TIMER_OUTPUTMODE_TOGGLE_SET == captureOutputMode) ||
        (TIMER_OUTPUTMODE_RESET_SET == captureOutputMode)
        );

    if (TIMER_CAPTURECOMPARE_REGISTER_0 == captureRegister){
        //CaptureCompare register 0 only supports certain modes
        ASSERT((TIMER_OUTPUTMODE_OUTBITVALUE == captureOutputMode) ||
            (TIMER_OUTPUTMODE_SET == captureOutputMode) ||
            (TIMER_OUTPUTMODE_TOGGLE == captureOutputMode) ||
            (TIMER_OUTPUTMODE_RESET == captureOutputMode) 
            );
    }

    HWREG(baseAddress + captureRegister ) |=   CAP;

    HWREG(baseAddress + captureRegister) &=
        ~(TIMER_CAPTUREMODE_RISING_AND_FALLING_EDGE +
          TIMER_CAPTURE_INPUTSELECT_Vcc +
          TIMER_CAPTURE_SYNCHRONOUS +
          TIMER_DO_CLEAR +
          TIMER_TAIE_INTERRUPT_ENABLE +
          CM_3
          );

    HWREG(baseAddress + captureRegister)  |= (captureMode +
                                              captureInputSelect +
                                              synchronizeCaptureSource +
                                              captureInterruptEnable +
                                              captureOutputMode
                                              );
}

//*****************************************************************************
//
//! Initializes Compare Mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMER_CAPTURECOMPARE_REGISTER_0
//!     \b TIMER_CAPTURECOMPARE_REGISTER_1
//!     \b TIMER_CAPTURECOMPARE_REGISTER_2
//!     \b TIMER_CAPTURECOMPARE_REGISTER_3
//!     \b TIMER_CAPTURECOMPARE_REGISTER_4
//!     \b TIMER_CAPTURECOMPARE_REGISTER_5
//!     \b TIMER_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareInterruptEnable is to enable or disable
//!         timer captureComapre interrupt. Valid values are
//!        \b TIMER_CAPTURECOMPARE_INTERRUPT_ENABLE and
//!        \b TIMER_CAPTURECOMPARE_INTERRUPT_DISABLE [Default value]
//! \param compareOutputMode specifies the ouput mode. Valid values are
//!        \b TIMER_OUTPUTMODE_OUTBITVALUE [Default value],
//!        \b TIMER_OUTPUTMODE_SET,
//!        \b TIMER_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMER_OUTPUTMODE_SET_RESET
//!        \b TIMER_OUTPUTMODE_TOGGLE,
//!        \b TIMER_OUTPUTMODE_RESET,
//!        \b TIMER_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMER_OUTPUTMODE_RESET_SET
//! \param compareValue is the count to be compared with in compare mode
//!
//! Modified register is \b TAxCCTLn and \b TAxCCRn
//! \return None
//
//*****************************************************************************
void Timer_initCompare (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned short compareInterruptEnable,
    unsigned int compareOutputMode,
    unsigned int compareValue
    )
{
    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == compareRegister) 
        );

   ASSERT((TIMER_CAPTURECOMPARE_INTERRUPT_ENABLE == compareInterruptEnable) ||
        (TIMER_CAPTURECOMPARE_INTERRUPT_DISABLE == compareInterruptEnable)
        );

    ASSERT((TIMER_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
        (TIMER_OUTPUTMODE_SET == compareOutputMode) ||
        (TIMER_OUTPUTMODE_TOGGLE_RESET == compareOutputMode) ||
        (TIMER_OUTPUTMODE_SET_RESET == compareOutputMode) ||
        (TIMER_OUTPUTMODE_TOGGLE == compareOutputMode) ||
        (TIMER_OUTPUTMODE_RESET == compareOutputMode) ||
        (TIMER_OUTPUTMODE_TOGGLE_SET == compareOutputMode) ||
        (TIMER_OUTPUTMODE_RESET_SET == compareOutputMode)
        );

    if (TIMER_CAPTURECOMPARE_REGISTER_0 == compareRegister){
        //CaptureCompare register 0 only supports certain modes
        ASSERT((TIMER_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
            (TIMER_OUTPUTMODE_SET == compareOutputMode) ||
            (TIMER_OUTPUTMODE_TOGGLE == compareOutputMode) ||
            (TIMER_OUTPUTMODE_RESET == compareOutputMode) 
            );
    }


    HWREG(baseAddress + compareRegister ) &=   ~CAP;

    HWREG(baseAddress + compareRegister) &=
        ~(TIMER_CAPTURECOMPARE_INTERRUPT_ENABLE +
          TIMER_OUTPUTMODE_RESET_SET
          );

    HWREG(baseAddress + compareRegister)  |= ( compareInterruptEnable +
                                               compareOutputMode
                                               );

    HWREG(baseAddress + compareRegister + OFS_TAxR) = compareValue;
}

//*****************************************************************************
//
//! Enable timer interrupt
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified register is TAxCTL
//!
//! \return None
//
//*****************************************************************************
void Timer_enableInterrupt (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_TAxCTL) &=  ~TAIFG;
    HWREG(baseAddress + OFS_TAxCTL) |= TAIE;
}

//*****************************************************************************
//
//! Disable timer interrupt
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified register is \b TAxCTL
//!
//! \return None
//
//*****************************************************************************
void Timer_disableInterrupt (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TAxCTL) &= ~TAIE;
}

//*****************************************************************************
//
//! Get timer interrupt status
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! \return unsigned long. Return interrupt status. Valid values are
//!         \b TIMER_INTERRUPT_PENDING
//!         \b TIMER_INTERRUPT_NOT_PENDING
//
//*****************************************************************************
unsigned long Timer_getInterruptStatus (unsigned int baseAddress)
{
    return ( HWREG(baseAddress + OFS_TAxCTL) & TAIFG );
}

//*****************************************************************************
//
//! Enable capture compare interrupt
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister is the selected capture compare regsiter
//!
//! Modified register is \b TAxCCTLn
//!
//! \return None
//
//*****************************************************************************
void Timer_enableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    HWREGB(baseAddress + captureCompareRegister) &= ~CCIFG;
    HWREG(baseAddress + captureCompareRegister) |= CCIE;
}

//*****************************************************************************
//
//! Disable capture compare interrupt
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister is the selected capture compare regsiter
//!
//! Modified register is \b TAxCCTLn
//!
//! \return None
//
//*****************************************************************************
void Timer_disableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );
    HWREG(baseAddress + captureCompareRegister) &= ~CCIE;
}

//*****************************************************************************
//
//! Return capture compare interrupt status
//!
//! \param baseAddress is the base address of the Timer module.
//! \param mask is the mask for the interrupt status
//!         Valid values is and OR of
//!         \b TIMER_CAPTURE_OVERFLOW,
//!         \b TIMER_CAPTURECOMPARE_INTERRUPT_FLAG
//!
//! Modified register is \b TAxCTL
//!
//! \returns unsigned long. The mask of the set flags.
//
//*****************************************************************************
unsigned long Timer_getCaptureCompareInterruptStatus (unsigned int baseAddress,
    unsigned int mask)
{
    return ( HWREG(baseAddress + OFS_TAxCTL) & mask );
}

//*****************************************************************************
//
//! Reset/Clear the timer clock divider, count direction, count
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified register is \b TAxCTL
//!
//! \returns None
//
//*****************************************************************************
void Timer_clear (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TAxCTL) |= TACLR;
}

//*****************************************************************************
//
//! Get synchrnozied capturecompare input
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMER_CAPTURECOMPARE_REGISTER_0
//!     \b TIMER_CAPTURECOMPARE_REGISTER_1
//!     \b TIMER_CAPTURECOMPARE_REGISTER_2
//!     \b TIMER_CAPTURECOMPARE_REGISTER_3
//!     \b TIMER_CAPTURECOMPARE_REGISTER_4
//!     \b TIMER_CAPTURECOMPARE_REGISTER_5
//!     \b TIMER_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param synchronized is to select type of capture compare input.
//!         Valid values are
//!        \b TIMER_READ_CAPTURE_COMPARE_INPUT
//!        \b TIMER_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT
//!
//! \return \b TIMER_CAPTURECOMPARE_INPUT_HIGH or
//!         \b TIMER_CAPTURECOMPARE_INPUT_LOW
//
//*****************************************************************************
unsigned short Timer_getSynchronizedCaptureCompareInput
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned short synchronized
    )
{
    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister) 
        );

    ASSERT((TIMER_READ_CAPTURE_COMPARE_INPUT == synchronized) ||
        (TIMER_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT == synchronized)
        );

    if (HWREG(baseAddress + captureCompareRegister) & synchronized){
        return ( TIMER_CAPTURECOMPARE_INPUT_HIGH) ;
    } else   {
        return ( TIMER_CAPTURECOMPARE_INPUT_LOW) ;
    }
}

//*****************************************************************************
//
//! Get ouput bit for output mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMER_CAPTURECOMPARE_REGISTER_0
//!     \b TIMER_CAPTURECOMPARE_REGISTER_1
//!     \b TIMER_CAPTURECOMPARE_REGISTER_2
//!     \b TIMER_CAPTURECOMPARE_REGISTER_3
//!     \b TIMER_CAPTURECOMPARE_REGISTER_4
//!     \b TIMER_CAPTURECOMPARE_REGISTER_5
//!     \b TIMER_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! \return \b TIMER_OUTPUTMODE_OUTBITVALUE_HIGH or
//!         \b TIMER_OUTPUTMODE_OUTBITVALUE_LOW
//
//*****************************************************************************
unsigned char Timer_getOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister) 
        );

    if (HWREG(baseAddress + captureCompareRegister) & OUT){
        return ( TIMER_OUTPUTMODE_OUTBITVALUE_HIGH) ;
    } else   {
        return ( TIMER_OUTPUTMODE_OUTBITVALUE_LOW) ;
    }
}

//*****************************************************************************
//
//! Get current capturecompare count
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMER_CAPTURECOMPARE_REGISTER_0
//!     \b TIMER_CAPTURECOMPARE_REGISTER_1
//!     \b TIMER_CAPTURECOMPARE_REGISTER_2
//!     \b TIMER_CAPTURECOMPARE_REGISTER_3
//!     \b TIMER_CAPTURECOMPARE_REGISTER_4
//!     \b TIMER_CAPTURECOMPARE_REGISTER_5
//!     \b TIMER_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! \return current count as unsigned int
//
//*****************************************************************************
unsigned int Timer_getCaptureCompareCount
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    return  (HWREG(baseAddress + OFS_TAxR + captureCompareRegister));
}

//*****************************************************************************
//
//! Set ouput bit for output mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMER_CAPTURECOMPARE_REGISTER_0
//!     \b TIMER_CAPTURECOMPARE_REGISTER_1
//!     \b TIMER_CAPTURECOMPARE_REGISTER_2
//!     \b TIMER_CAPTURECOMPARE_REGISTER_3
//!     \b TIMER_CAPTURECOMPARE_REGISTER_4
//!     \b TIMER_CAPTURECOMPARE_REGISTER_5
//!     \b TIMER_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param outputModeOutBitValueis the value to be set for out bit
//!     Valid values are \b TIMER_OUTPUTMODE_OUTBITVALUE_HIGH
//!                      \b TIMER_OUTPUTMODE_OUTBITVALUE_LOW
//!
//! Modified register is \b TAxCCTLn
//!
//! \return None
//
//*****************************************************************************
void Timer_setOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned char outputModeOutBitValue
    )
{
    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister) 
        );

    ASSERT((TIMER_OUTPUTMODE_OUTBITVALUE_HIGH == outputModeOutBitValue) ||
        (TIMER_OUTPUTMODE_OUTBITVALUE_LOW == outputModeOutBitValue)
        );

    HWREG(baseAddress + captureCompareRegister) &= ~OUT;
    HWREG(baseAddress + captureCompareRegister) |= outputModeOutBitValue;
}

//*****************************************************************************
//
//! Generate a PWM with timer running in up down mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMER_CLOCKSOURCE_EXTERNAL_TXCLK
//!         \b TIMER_CLOCKSOURCE_ACLK
//!         \b TIMER_CLOCKSOURCE_SMCLK
//!         \b TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMER_CLOCKSOURCE_DIVIDER_1
//!        \b TIMER_CLOCKSOURCE_DIVIDER_2
//!        \b TIMER_CLOCKSOURCE_DIVIDER_4
//!        \b TIMER_CLOCKSOURCE_DIVIDER_8
//!        \b TIMER_CLOCKSOURCE_DIVIDER_3
//!        \b TIMER_CLOCKSOURCE_DIVIDER_5
//!        \b TIMER_CLOCKSOURCE_DIVIDER_6
//!        \b TIMER_CLOCKSOURCE_DIVIDER_7
//!        \b TIMER_CLOCKSOURCE_DIVIDER_10
//!        \b TIMER_CLOCKSOURCE_DIVIDER_12
//!        \b TIMER_CLOCKSOURCE_DIVIDER_14
//!        \b TIMER_CLOCKSOURCE_DIVIDER_16
//!        \b TIMER_CLOCKSOURCE_DIVIDER_20
//!        \b TIMER_CLOCKSOURCE_DIVIDER_24
//!        \b TIMER_CLOCKSOURCE_DIVIDER_28
//!        \b TIMER_CLOCKSOURCE_DIVIDER_32
//!        \b TIMER_CLOCKSOURCE_DIVIDER_40
//!        \b TIMER_CLOCKSOURCE_DIVIDER_48
//!        \b TIMER_CLOCKSOURCE_DIVIDER_56
//!        \b TIMER_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod selects the desired timer period
//! \param compareRegister selects the compare register being used. Valid values
//!     are
//!     \b TIMER_CAPTURECOMPARE_REGISTER_0
//!     \b TIMER_CAPTURECOMPARE_REGISTER_1
//!     \b TIMER_CAPTURECOMPARE_REGISTER_2
//!     \b TIMER_CAPTURECOMPARE_REGISTER_3
//!     \b TIMER_CAPTURECOMPARE_REGISTER_4
//!     \b TIMER_CAPTURECOMPARE_REGISTER_5
//!     \b TIMER_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareOutputMode specifies the ouput mode. Valid values are
//!        \b TIMER_OUTPUTMODE_OUTBITVALUE,
//!        \b TIMER_OUTPUTMODE_SET,
//!        \b TIMER_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMER_OUTPUTMODE_SET_RESET
//!        \b TIMER_OUTPUTMODE_TOGGLE,
//!        \b TIMER_OUTPUTMODE_RESET,
//!        \b TIMER_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMER_OUTPUTMODE_RESET_SET
//! \param dutyCycle specifies the dutycycle for the generated waveform
//!
//! Modified registers are \b TAxCTL, \b TAxCCR0, \b TAxCCTL0,\b TAxCCTLn
//!
//! \return None
//
//*****************************************************************************
void Timer_generatePWM (  unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int compareRegister,
    unsigned int compareOutputMode,
    unsigned int dutyCycle
    )
{
    ASSERT(
        (TIMER_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMER_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMER_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == compareRegister) 
        );


    ASSERT((TIMER_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
        (TIMER_OUTPUTMODE_SET == compareOutputMode) ||
        (TIMER_OUTPUTMODE_TOGGLE_RESET == compareOutputMode) ||
        (TIMER_OUTPUTMODE_SET_RESET == compareOutputMode) ||
        (TIMER_OUTPUTMODE_TOGGLE == compareOutputMode) ||
        (TIMER_OUTPUTMODE_RESET == compareOutputMode) ||
        (TIMER_OUTPUTMODE_TOGGLE_SET == compareOutputMode) ||
        (TIMER_OUTPUTMODE_RESET_SET == compareOutputMode)
        );

    privateTimerProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TAxCTL)  &=
        ~( TIMER_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
           TIMER_UPDOWN_MODE + TIMER_DO_CLEAR +
           TIMER_TAIE_INTERRUPT_ENABLE
           );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource +
                                          TIMER_UP_MODE +
                                          TIMER_DO_CLEAR
                                          );

    HWREG(baseAddress + OFS_TAxCCR0)  = timerPeriod;

    HWREG(baseAddress + OFS_TAxCCTL0)  &=
        ~(TIMER_CAPTURECOMPARE_INTERRUPT_ENABLE +
          TIMER_OUTPUTMODE_RESET_SET
          );
    HWREG(baseAddress + compareRegister)  |= compareOutputMode;

    HWREG(baseAddress + compareRegister + OFS_TAxR) = dutyCycle;
}

//*****************************************************************************
//
//! Stops the timer
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified registers are \b TAxCTL
//!
//! \returns None
//
//*****************************************************************************
void Timer_stop ( unsigned int baseAddress )
{
    HWREG(baseAddress + OFS_TAxCTL)  &= ~MC_3;
    HWREG(baseAddress + OFS_TAxCTL)  |= MC_0;
}

//*****************************************************************************
//
//! Private clock source divider helper function
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSourceDivider is the desired divider for the clock source
//!
//! Modified registers are TAxEX0, TAxCTL
//!
//! \returns None
//
//*****************************************************************************
void privateTimerProcessClockSourceDivider (unsigned int baseAddress,
    unsigned int clockSourceDivider)
{
    HWREG(baseAddress + OFS_TAxCTL) &= ~ID__8;
    HWREG(baseAddress + OFS_TAxEX0) &= ~TAIDEX_7;
    switch (clockSourceDivider){
        case TIMER_CLOCKSOURCE_DIVIDER_1:
        case TIMER_CLOCKSOURCE_DIVIDER_2:
        case TIMER_CLOCKSOURCE_DIVIDER_4:
        case TIMER_CLOCKSOURCE_DIVIDER_8:
            HWREG(baseAddress + OFS_TAxCTL) |= ((clockSourceDivider - 1) << 6);
            HWREG(baseAddress + OFS_TAxEX0) = TAIDEX_0;
            break;

        case TIMER_CLOCKSOURCE_DIVIDER_3:
        case TIMER_CLOCKSOURCE_DIVIDER_5:
        case TIMER_CLOCKSOURCE_DIVIDER_6:
        case TIMER_CLOCKSOURCE_DIVIDER_7:
            HWREG(baseAddress + OFS_TAxCTL) |= ID__1;
            HWREG(baseAddress + OFS_TAxEX0) = (clockSourceDivider - 1);
            break;

        case TIMER_CLOCKSOURCE_DIVIDER_10:
        case TIMER_CLOCKSOURCE_DIVIDER_12:
        case TIMER_CLOCKSOURCE_DIVIDER_14:
        case TIMER_CLOCKSOURCE_DIVIDER_16:
            HWREG(baseAddress + OFS_TAxCTL) |= ID__2;
            HWREG(baseAddress + OFS_TAxEX0) = (clockSourceDivider / 2 - 1 );
            break;

        case TIMER_CLOCKSOURCE_DIVIDER_20:
        case TIMER_CLOCKSOURCE_DIVIDER_24:
        case TIMER_CLOCKSOURCE_DIVIDER_28:
        case TIMER_CLOCKSOURCE_DIVIDER_32:
            HWREG(baseAddress + OFS_TAxCTL) |= ID__4;
            HWREG(baseAddress + OFS_TAxEX0) = (clockSourceDivider / 4 - 1);
            break;
        case TIMER_CLOCKSOURCE_DIVIDER_40:
        case TIMER_CLOCKSOURCE_DIVIDER_48:
        case TIMER_CLOCKSOURCE_DIVIDER_56:
        case TIMER_CLOCKSOURCE_DIVIDER_64:
            HWREG(baseAddress + OFS_TAxCTL) |= ID__8;
            HWREG(baseAddress + OFS_TAxEX0) = (clockSourceDivider / 8 - 1);
            break;
    }
}

//*****************************************************************************
//
//! Sets the value of the capture-compare register
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMER_CAPTURECOMPARE_REGISTER_0
//!     \b TIMER_CAPTURECOMPARE_REGISTER_1
//!     \b TIMER_CAPTURECOMPARE_REGISTER_2
//!     \b TIMER_CAPTURECOMPARE_REGISTER_3
//!     \b TIMER_CAPTURECOMPARE_REGISTER_4
//!     \b TIMER_CAPTURECOMPARE_REGISTER_5
//!     \b TIMER_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareValue is the count to be compared with in compare mode
//!
//! Modified register is \b TAxCCRn
//!
//! \return None
//
//*****************************************************************************
void Timer_setCompareValue (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned int compareValue
    )
{
    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == compareRegister) 
        );

    HWREG(baseAddress + compareRegister + OFS_TAxR) = compareValue;
}

//*****************************************************************************
//
//! Clears the Timer TAIFG interrupt flag
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified bits are TAIFG og TAxCTL register
//!
//! \return None
//
//*****************************************************************************
void Timer_clearTimerInterruptFlag (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_TAxCTL) &= ~TAIFG;
}

//*****************************************************************************
//
//! Clears the capture-compare interrupt flag
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister selects the Capture-compare register being
//! used. Valid values are
//!     \b TIMER_CAPTURECOMPARE_REGISTER_0
//!     \b TIMER_CAPTURECOMPARE_REGISTER_1
//!     \b TIMER_CAPTURECOMPARE_REGISTER_2
//!     \b TIMER_CAPTURECOMPARE_REGISTER_3
//!     \b TIMER_CAPTURECOMPARE_REGISTER_4
//!     \b TIMER_CAPTURECOMPARE_REGISTER_5
//!     \b TIMER_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! Modified bits are CCIFG of \b TAxCCTLn register
//!
//! \return None
//
//*****************************************************************************
void Timer_clearCaptureCompareInterruptFlag (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMER_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMER_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister) 
        );

    HWREG(baseAddress + captureCompareRegister)  &= ~CCIFG;
}

//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************
