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
//*****************************************************************************
//
//timera.c - Driver for the TIMERA Module.
//
//*****************************************************************************
#include "../inc/hw_types.h"
#include "debug.h"
#include "timera.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "../deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif

void privateTimerAProcessClockSourceDivider (unsigned int baseAddress,
    unsigned int clockSourceDivider);

//*****************************************************************************
//
//! Starts TimerA counter
//!
//! \param baseAddress is the base address of the TimerA module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERA_CONTINUOUS_MODE [Default value]
//!         \b TIMERA_UPDOWN_MODE
//!         \b TIMERA_UP_MODE

//! Modified register is \b TAxCTL
//!
//!NOTE: This function assumes that the timer has been previously configured
//! using TimerA_configureContinuousMode,  TimerA_configureUpMode or
//!TimerA_configureUpDownMode.
//!
//! \return None
//
//*****************************************************************************
void TimerA_startCounter ( unsigned int baseAddress,
    unsigned int timerMode
    )
{
    ASSERT(
        (TIMERA_UPDOWN_MODE == timerMode) ||
        (TIMERA_CONTINUOUS_MODE == timerMode) ||
        (TIMERA_UP_MODE == timerMode) ||
         );


    HWREG(baseAddress + OFS_TAxCTL) |= timerMode;
}

//*****************************************************************************
//
//! Configures TimerA in continuous mode.
//!
//! \param baseAddress is the base address of the TimerA module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERA_CLOCKSOURCE_ACLK
//!         \b TIMERA_CLOCKSOURCE_SMCLK
//!         \b TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_64
//! \param timerInterruptEnable_TAIE is to enable or disable TimerA interrupt
//!        Valid values are
//!        \b TIMERA_TAIE_INTERRUPT_ENABLE
//!        \b TIMERA_TAIE_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if TimerA clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERA_DO_CLEAR
//!        \b TIMERA_SKIP_CLEAR [Default value]
//!
//! Modified reister is \b TAxCTL
//!
//!This API does not start the timer. Timer needs to be started when required
//!using the TimerA_startCounter API.
//!
//! \return None
//
//*****************************************************************************
void TimerA_configureContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERA_DO_CLEAR == timerClear) ||
        (TIMERA_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERA_TAIE_INTERRUPT_ENABLE == timerInterruptEnable_TAIE) ||
        (TIMERA_TAIE_INTERRUPT_DISABLE == timerInterruptEnable_TAIE)
        );

    ASSERT(
        (TIMERA_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );


    privateTimerAProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress +
        OFS_TAxCTL) &= ~(TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
        				 TIMERA_STOP_MODE +
                         TIMERA_DO_CLEAR +
                         TIMERA_TAIE_INTERRUPT_ENABLE
                         );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource + TIMERA_CONTINUOUS_MODE +
                                          timerClear +
                                          timerInterruptEnable_TAIE);
}
//*****************************************************************************
//
//! Configures TimerA in up mode.
//!
//! \param baseAddress is the base address of the TimerA module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERA_CLOCKSOURCE_ACLK
//!         \b TIMERA_CLOCKSOURCE_SMCLK
//!         \b TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod is the specified TimerA period. This is the value that gets
//!         written into the CCR0. Limited to 16 bits[unsigned int]
//! \param timerInterruptEnable_TAIE is to enable or disable TimerA interrupt
//!        Valid values are
//!        \b TIMERA_TAIE_INTERRUPT_ENABLE and
//!        \b TIMERA_TAIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         TimerA CCR0 captureComapre interrupt. Valid values are
//!        \b TIMERA_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMERA_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if TimerA clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERA_DO_CLEAR
//!        \b TIMERA_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TAxCTL, \b TAxCCR0, \b TAxCCTL0
//!
//!This API does not start the timer. Timer needs to be started when required
//!using the TimerA_startCounter API.
//!
//! \return None
//
//*****************************************************************************
void TimerA_configureUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERA_DO_CLEAR == timerClear) ||
        (TIMERA_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERA_DO_CLEAR == timerClear) ||
        (TIMERA_SKIP_CLEAR == timerClear)
        );

    privateTimerAProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TAxCTL) &=
        ~(TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMERA_STOP_MODE +
          TIMERA_DO_CLEAR +
          TIMERA_TAIE_INTERRUPT_ENABLE
          );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource +
                                          TIMERA_UP_MODE +
                                          timerClear +
                                          timerInterruptEnable_TAIE
                                          );

    if (TIMERA_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TAxCCTL0)  |= TIMERA_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TAxCCTL0)  &= ~TIMERA_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TAxCCR0)  = timerPeriod;
}

//*****************************************************************************
//
//! Configures TimerA in up down mode.
//!
//! \param baseAddress is the base address of the TimerA module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERA_CLOCKSOURCE_ACLK
//!         \b TIMERA_CLOCKSOURCE_SMCLK
//!         \b TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod is the specified TimerA period
//! \param timerInterruptEnable_TAIE is to enable or disable TimerA interrupt
//!        Valid values are
//!        \b TIMERA_TAIE_INTERRUPT_ENABLE
//!        \b TIMERA_TAIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         TimerA CCR0 captureComapre interrupt. Valid values are
//!        \b TIMERA_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMERA_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if TimerA clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERA_DO_CLEAR
//!        \b TIMERA_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TAxCTL, \b TAxCCR0, \b TAxCCTL0
//!
//!This API does not start the timer. Timer needs to be started when required
//!using the TimerA_startCounter API.
//!
//! \return None
//
//*****************************************************************************
void TimerA_configureUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERA_DO_CLEAR == timerClear) ||
        (TIMERA_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERA_DO_CLEAR == timerClear) ||
        (TIMERA_SKIP_CLEAR == timerClear)
        );

    privateTimerAProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TAxCTL) &=
        ~(TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMERA_UPDOWN_MODE +
          TIMERA_DO_CLEAR +
          TIMERA_TAIE_INTERRUPT_ENABLE
          );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource +
    									  TIMERA_STOP_MODE +
                                          timerClear +
                                          timerInterruptEnable_TAIE
                                          );
    if (TIMERA_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TAxCCTL0)  |= TIMERA_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TAxCCTL0)  &= ~TIMERA_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TAxCCR0)  = timerPeriod;
}


//*****************************************************************************
//
//! Starts timer in continuous mode.
//!
//! DEPRPECATED - Replaced by TimerA_configureContinuousMode and TimerA_startCounter
//! API
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERA_CLOCKSOURCE_ACLK
//!         \b TIMERA_CLOCKSOURCE_SMCLK
//!         \b TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_64
//! \param timerInterruptEnable_TAIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMERA_TAIE_INTERRUPT_ENABLE
//!        \b TIMERA_TAIE_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERA_DO_CLEAR
//!        \b TIMERA_SKIP_CLEAR [Default value]
//!
//! Modified reister is \b TAxCTL
//!
//! \return None
//
//*****************************************************************************
void TimerA_startContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERA_DO_CLEAR == timerClear) ||
        (TIMERA_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERA_TAIE_INTERRUPT_ENABLE == timerInterruptEnable_TAIE) ||
        (TIMERA_TAIE_INTERRUPT_DISABLE == timerInterruptEnable_TAIE)
        );

    ASSERT(
        (TIMERA_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMERA_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );


    privateTimerAProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress +
        OFS_TAxCTL) &= ~(TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
                         TIMERA_UPDOWN_MODE +
                         TIMERA_DO_CLEAR +
                         TIMERA_TAIE_INTERRUPT_ENABLE
                         );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource + TIMERA_CONTINUOUS_MODE +
                                          timerClear +
                                          timerInterruptEnable_TAIE);
}

//*****************************************************************************
//
//! DEPRECATED- Spelling Error Fixed
//! Starts timer in continuous mode.
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERA_CLOCKSOURCE_ACLK
//!         \b TIMERA_CLOCKSOURCE_SMCLK
//!         \b TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_64
//! \param timerInterruptEnable_TAIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMERA_TAIE_INTERRUPT_ENABLE
//!        \b TIMERA_TAIE_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERA_DO_CLEAR
//!        \b TIMERA_SKIP_CLEAR [Default value]
//!
//! Modified reister is \b TAxCTL
//!
//! \return None
//
//*****************************************************************************
void TimerA_startContinousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int timerClear
    )
{
	TimerA_startContinuousMode (baseAddress,
    clockSource,
    clockSourceDivider,
    timerInterruptEnable_TAIE,
    timerClear
    );
}
//*****************************************************************************
//
//! Starts timer in up mode.
//!
//! DEPRPECATED - Replaced by TimerA_configureUpMode and TimerA_startCounter
//! API
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERA_CLOCKSOURCE_ACLK
//!         \b TIMERA_CLOCKSOURCE_SMCLK
//!         \b TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod is the specified timer period. This is the value that gets
//!         written into the CCR0. Limited to 16 bits[unsigned int]
//! \param timerInterruptEnable_TAIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMERA_TAIE_INTERRUPT_ENABLE and
//!        \b TIMERA_TAIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         timer CCR0 captureComapre interrupt. Valid values are
//!        \b TIMERA_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMERA_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERA_DO_CLEAR
//!        \b TIMERA_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TAxCTL, \b TAxCCR0, \b TAxCCTL0
//!
//! \return None
//
//*****************************************************************************
void TimerA_startUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERA_DO_CLEAR == timerClear) ||
        (TIMERA_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERA_DO_CLEAR == timerClear) ||
        (TIMERA_SKIP_CLEAR == timerClear)
        );

    privateTimerAProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TAxCTL) &=
        ~(TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMERA_UPDOWN_MODE +
          TIMERA_DO_CLEAR +
          TIMERA_TAIE_INTERRUPT_ENABLE
          );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource +
                                          TIMERA_UP_MODE +
                                          timerClear +
                                          timerInterruptEnable_TAIE
                                          );

    if (TIMERA_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TAxCCTL0)  |= TIMERA_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TAxCCTL0)  &= ~TIMERA_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TAxCCR0)  = timerPeriod;
}

//*****************************************************************************
//
//! Starts timer in up down mode.
//!
//! DEPRPECATED - Replaced by TimerA_configureUpDownMode and TimerA_startCounter
//! API
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERA_CLOCKSOURCE_ACLK
//!         \b TIMERA_CLOCKSOURCE_SMCLK
//!         \b TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod is the specified timer period
//! \param timerInterruptEnable_TAIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMERA_TAIE_INTERRUPT_ENABLE
//!        \b TIMERA_TAIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         timer CCR0 captureComapre interrupt. Valid values are
//!        \b TIMERA_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMERA_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERA_DO_CLEAR
//!        \b TIMERA_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TAxCTL, \b TAxCCR0, \b TAxCCTL0
//!
//! \return None
//
//*****************************************************************************
void TimerA_startUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TAIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERA_DO_CLEAR == timerClear) ||
        (TIMERA_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERA_DO_CLEAR == timerClear) ||
        (TIMERA_SKIP_CLEAR == timerClear)
        );

    privateTimerAProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TAxCTL) &=
        ~(TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMERA_UPDOWN_MODE +
          TIMERA_DO_CLEAR +
          TIMERA_TAIE_INTERRUPT_ENABLE
          );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource +
                                          TIMERA_UPDOWN_MODE +
                                          timerClear +
                                          timerInterruptEnable_TAIE
                                          );
    if (TIMERA_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TAxCCTL0)  |= TIMERA_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TAxCCTL0)  &= ~TIMERA_CCIE_CCR0_INTERRUPT_ENABLE;
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
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param captureMode is the capture mode selected. Valid values are
//!        \b TIMERA_CAPTUREMODE_NO_CAPTURE [Default value]
//!        \b TIMERA_CAPTUREMODE_RISING_EDGE
//!        \b TIMERA_CAPTUREMODE_FALLING_EDGE
//!        \b TIMERA_CAPTUREMODE_RISING_AND_FALLING_EDGE
//! \param captureInputSelect decides the Input Select
//!        \b TIMERA_CAPTURE_INPUTSELECT_CCIxA [Default value]
//!        \b TIMERA_CAPTURE_INPUTSELECT_CCIxB
//!        \b TIMERA_CAPTURE_INPUTSELECT_GND
//!        \b TIMERA_CAPTURE_INPUTSELECT_Vcc
//! \param synchronizeCaptureSource decides if capture source should be
//!         synchronized with timer clock
//!        Valid values are
//!        \b TIMERA_CAPTURE_ASYNCHRONOUS [Default value]
//!        \b TIMERA_CAPTURE_SYNCHRONOUS
//! \param captureInterruptEnable is to enable or disable
//!         timer captureComapre interrupt. Valid values are
//!        \b TIMERA_CAPTURECOMPARE_INTERRUPT_DISABLE [Default value]
//!        \b TIMERA_CAPTURECOMPARE_INTERRUPT_ENABLE
//! \param captureOutputMode specifies the ouput mode. Valid values are
//!        \b TIMERA_OUTPUTMODE_OUTBITVALUE [Default value],
//!        \b TIMERA_OUTPUTMODE_SET,
//!        \b TIMERA_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMERA_OUTPUTMODE_SET_RESET
//!        \b TIMERA_OUTPUTMODE_TOGGLE,
//!        \b TIMERA_OUTPUTMODE_RESET,
//!        \b TIMERA_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMERA_OUTPUTMODE_RESET_SET
//!
//! Modified register is \b TAxCCTLn
//! \return None
//
//*****************************************************************************
void TimerA_initCapture (unsigned int baseAddress,
    unsigned int captureRegister,
    unsigned int captureMode,
    unsigned int captureInputSelect,
    unsigned short synchronizeCaptureSource,
    unsigned short captureInterruptEnable,
    unsigned int captureOutputMode
    )
{
    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == captureRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == captureRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == captureRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == captureRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == captureRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == captureRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == captureRegister)
        );

    ASSERT((TIMERA_CAPTUREMODE_NO_CAPTURE == captureMode) ||
        (TIMERA_CAPTUREMODE_RISING_EDGE == captureMode) ||
        (TIMERA_CAPTUREMODE_FALLING_EDGE == captureMode) ||
        (TIMERA_CAPTUREMODE_RISING_AND_FALLING_EDGE == captureMode)
        );

    ASSERT((TIMERA_CAPTURE_INPUTSELECT_CCIxA == captureInputSelect) ||
        (TIMERA_CAPTURE_INPUTSELECT_CCIxB == captureInputSelect) ||
        (TIMERA_CAPTURE_INPUTSELECT_GND == captureInputSelect) ||
        (TIMERA_CAPTURE_INPUTSELECT_Vcc == captureInputSelect)
        );

    ASSERT((TIMERA_CAPTURE_ASYNCHRONOUS == synchronizeCaptureSource) ||
        (TIMERA_CAPTURE_SYNCHRONOUS == synchronizeCaptureSource)
        );

    ASSERT(
        (TIMERA_CAPTURECOMPARE_INTERRUPT_DISABLE == captureInterruptEnable) ||
        (TIMERA_CAPTURECOMPARE_INTERRUPT_ENABLE == captureInterruptEnable)
        );

    ASSERT((TIMERA_OUTPUTMODE_OUTBITVALUE == captureOutputMode) ||
        (TIMERA_OUTPUTMODE_SET == captureOutputMode) ||
        (TIMERA_OUTPUTMODE_TOGGLE_RESET == captureOutputMode) ||
        (TIMERA_OUTPUTMODE_SET_RESET == captureOutputMode) ||
        (TIMERA_OUTPUTMODE_TOGGLE == captureOutputMode) ||
        (TIMERA_OUTPUTMODE_RESET == captureOutputMode) ||
        (TIMERA_OUTPUTMODE_TOGGLE_SET == captureOutputMode) ||
        (TIMERA_OUTPUTMODE_RESET_SET == captureOutputMode)
        );

    if (TIMERA_CAPTURECOMPARE_REGISTER_0 == captureRegister){
        //CaptureCompare register 0 only supports certain modes
        ASSERT((TIMERA_OUTPUTMODE_OUTBITVALUE == captureOutputMode) ||
            (TIMERA_OUTPUTMODE_SET == captureOutputMode) ||
            (TIMERA_OUTPUTMODE_TOGGLE == captureOutputMode) ||
            (TIMERA_OUTPUTMODE_RESET == captureOutputMode)
            );
    }

    HWREG(baseAddress + captureRegister ) |=   CAP;

    HWREG(baseAddress + captureRegister) &=
        ~(TIMERA_CAPTUREMODE_RISING_AND_FALLING_EDGE +
          TIMERA_CAPTURE_INPUTSELECT_Vcc +
          TIMERA_CAPTURE_SYNCHRONOUS +
          TIMERA_DO_CLEAR +
          TIMERA_TAIE_INTERRUPT_ENABLE +
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
//! \param compareRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareInterruptEnable is to enable or disable
//!         timer captureComapre interrupt. Valid values are
//!        \b TIMERA_CAPTURECOMPARE_INTERRUPT_ENABLE and
//!        \b TIMERA_CAPTURECOMPARE_INTERRUPT_DISABLE [Default value]
//! \param compareOutputMode specifies the ouput mode. Valid values are
//!        \b TIMERA_OUTPUTMODE_OUTBITVALUE [Default value],
//!        \b TIMERA_OUTPUTMODE_SET,
//!        \b TIMERA_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMERA_OUTPUTMODE_SET_RESET
//!        \b TIMERA_OUTPUTMODE_TOGGLE,
//!        \b TIMERA_OUTPUTMODE_RESET,
//!        \b TIMERA_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMERA_OUTPUTMODE_RESET_SET
//! \param compareValue is the count to be compared with in compare mode
//!
//! Modified register is \b TAxCCTLn and \b TAxCCRn
//! \return None
//
//*****************************************************************************
void TimerA_initCompare (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned short compareInterruptEnable,
    unsigned int compareOutputMode,
    unsigned int compareValue
    )
{
    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == compareRegister)
        );

   ASSERT((TIMERA_CAPTURECOMPARE_INTERRUPT_ENABLE == compareInterruptEnable) ||
        (TIMERA_CAPTURECOMPARE_INTERRUPT_DISABLE == compareInterruptEnable)
        );

    ASSERT((TIMERA_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_SET == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_TOGGLE_RESET == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_SET_RESET == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_TOGGLE == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_RESET == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_TOGGLE_SET == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_RESET_SET == compareOutputMode)
        );

    if (TIMERA_CAPTURECOMPARE_REGISTER_0 == compareRegister){
        //CaptureCompare register 0 only supports certain modes
        ASSERT((TIMERA_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
            (TIMERA_OUTPUTMODE_SET == compareOutputMode) ||
            (TIMERA_OUTPUTMODE_TOGGLE == compareOutputMode) ||
            (TIMERA_OUTPUTMODE_RESET == compareOutputMode)
            );
    }


    HWREG(baseAddress + compareRegister ) &=   ~CAP;

    HWREG(baseAddress + compareRegister) &=
        ~(TIMERA_CAPTURECOMPARE_INTERRUPT_ENABLE +
          TIMERA_OUTPUTMODE_RESET_SET
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
void TimerA_enableInterrupt (unsigned int baseAddress)
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
void TimerA_disableInterrupt (unsigned int baseAddress)
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
//!         \b TIMERA_INTERRUPT_PENDING
//!         \b TIMERA_INTERRUPT_NOT_PENDING
//
//*****************************************************************************
unsigned long TimerA_getInterruptStatus (unsigned int baseAddress)
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
void TimerA_enableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
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
void TimerA_disableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );
    HWREG(baseAddress + captureCompareRegister) &= ~CCIE;
}

//*****************************************************************************
//
//! Return capture compare interrupt status
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister is the selected capture compare register
//! \param mask is the mask for the interrupt status
//!         Valid values is and OR of
//!         \b TIMERA_CAPTURE_OVERFLOW,
//!         \b TIMERA_CAPTURECOMPARE_INTERRUPT_FLAG
//!
//!
//! \returns unsigned long. The mask of the set flags.
//
//*****************************************************************************
unsigned long TimerA_getCaptureCompareInterruptStatus (unsigned int baseAddress,
		 unsigned int captureCompareRegister,
		 unsigned int mask
		 )
{
    return ( HWREG(baseAddress + captureCompareRegister) & mask );
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
void TimerA_clear (unsigned int baseAddress)
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
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param synchronized is to select type of capture compare input.
//!         Valid values are
//!        \b TIMERA_READ_CAPTURE_COMPARE_INPUT
//!        \b TIMERA_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT
//!
//! \return \b TIMERA_CAPTURECOMPARE_INPUT_HIGH or
//!         \b TIMERA_CAPTURECOMPARE_INPUT_LOW
//
//*****************************************************************************
unsigned short TimerA_getSynchronizedCaptureCompareInput
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned short synchronized
    )
{
    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    ASSERT((TIMERA_READ_CAPTURE_COMPARE_INPUT == synchronized) ||
        (TIMERA_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT == synchronized)
        );

    if (HWREG(baseAddress + captureCompareRegister) & synchronized){
        return ( TIMERA_CAPTURECOMPARE_INPUT_HIGH) ;
    } else   {
        return ( TIMERA_CAPTURECOMPARE_INPUT_LOW) ;
    }
}

//*****************************************************************************
//
//! Get ouput bit for output mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! \return \b TIMERA_OUTPUTMODE_OUTBITVALUE_HIGH or
//!         \b TIMERA_OUTPUTMODE_OUTBITVALUE_LOW
//
//*****************************************************************************
unsigned char TimerA_getOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    if (HWREG(baseAddress + captureCompareRegister) & OUT){
        return ( TIMERA_OUTPUTMODE_OUTBITVALUE_HIGH) ;
    } else   {
        return ( TIMERA_OUTPUTMODE_OUTBITVALUE_LOW) ;
    }
}

//*****************************************************************************
//
//! Get current capturecompare count
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! \return current count as unsigned int
//
//*****************************************************************************
unsigned int TimerA_getCaptureCompareCount
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    return  (HWREG(baseAddress + OFS_TAxR + captureCompareRegister));
}

//*****************************************************************************
//
//! Set ouput bit for output mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister selects the Capture register being used.
//!     are
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param outputModeOutBitValueis the value to be set for out bit
//!     Valid values are \b TIMERA_OUTPUTMODE_OUTBITVALUE_HIGH
//!                      \b TIMERA_OUTPUTMODE_OUTBITVALUE_LOW
//!
//! Modified register is \b TAxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerA_setOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned char outputModeOutBitValue
    )
{
    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    ASSERT((TIMERA_OUTPUTMODE_OUTBITVALUE_HIGH == outputModeOutBitValue) ||
        (TIMERA_OUTPUTMODE_OUTBITVALUE_LOW == outputModeOutBitValue)
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
//!         \b TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK
//!         \b TIMERA_CLOCKSOURCE_ACLK
//!         \b TIMERA_CLOCKSOURCE_SMCLK
//!         \b TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_1
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERA_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod selects the desired timer period
//! \param compareRegister selects the compare register being used. Valid values
//!     are
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareOutputMode specifies the ouput mode. Valid values are
//!        \b TIMERA_OUTPUTMODE_OUTBITVALUE,
//!        \b TIMERA_OUTPUTMODE_SET,
//!        \b TIMERA_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMERA_OUTPUTMODE_SET_RESET
//!        \b TIMERA_OUTPUTMODE_TOGGLE,
//!        \b TIMERA_OUTPUTMODE_RESET,
//!        \b TIMERA_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMERA_OUTPUTMODE_RESET_SET
//! \param dutyCycle specifies the dutycycle for the generated waveform
//!
//! Modified registers are \b TAxCTL, \b TAxCCR0, \b TAxCCTL0,\b TAxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerA_generatePWM (  unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int compareRegister,
    unsigned int compareOutputMode,
    unsigned int dutyCycle
    )
{
    ASSERT(
        (TIMERA_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == compareRegister)
        );


    ASSERT((TIMERA_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_SET == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_TOGGLE_RESET == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_SET_RESET == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_TOGGLE == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_RESET == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_TOGGLE_SET == compareOutputMode) ||
        (TIMERA_OUTPUTMODE_RESET_SET == compareOutputMode)
        );

    privateTimerAProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TAxCTL)  &=
        ~( TIMERA_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
           TIMERA_UPDOWN_MODE + TIMERA_DO_CLEAR +
           TIMERA_TAIE_INTERRUPT_ENABLE
           );

    HWREG(baseAddress + OFS_TAxCTL)  |= ( clockSource +
                                          TIMERA_UP_MODE +
                                          TIMERA_DO_CLEAR
                                          );

    HWREG(baseAddress + OFS_TAxCCR0)  = timerPeriod;

    HWREG(baseAddress + OFS_TAxCCTL0)  &=
        ~(TIMERA_CAPTURECOMPARE_INTERRUPT_ENABLE +
          TIMERA_OUTPUTMODE_RESET_SET
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
void TimerA_stop ( unsigned int baseAddress )
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
void privateTimerAProcessClockSourceDivider (unsigned int baseAddress,
    unsigned int clockSourceDivider)
{
    HWREG(baseAddress + OFS_TAxCTL) &= ~ID__8;
    HWREG(baseAddress + OFS_TAxEX0) &= ~TAIDEX_7;
    switch (clockSourceDivider){
        case TIMERA_CLOCKSOURCE_DIVIDER_1:
        case TIMERA_CLOCKSOURCE_DIVIDER_2:
        case TIMERA_CLOCKSOURCE_DIVIDER_4:
        case TIMERA_CLOCKSOURCE_DIVIDER_8:
            HWREG(baseAddress + OFS_TAxCTL) |= ((clockSourceDivider - 1) << 6);
            HWREG(baseAddress + OFS_TAxEX0) = TAIDEX_0;
            break;

        case TIMERA_CLOCKSOURCE_DIVIDER_3:
        case TIMERA_CLOCKSOURCE_DIVIDER_5:
        case TIMERA_CLOCKSOURCE_DIVIDER_6:
        case TIMERA_CLOCKSOURCE_DIVIDER_7:
            HWREG(baseAddress + OFS_TAxCTL) |= ID__1;
            HWREG(baseAddress + OFS_TAxEX0) = (clockSourceDivider - 1);
            break;

        case TIMERA_CLOCKSOURCE_DIVIDER_10:
        case TIMERA_CLOCKSOURCE_DIVIDER_12:
        case TIMERA_CLOCKSOURCE_DIVIDER_14:
        case TIMERA_CLOCKSOURCE_DIVIDER_16:
            HWREG(baseAddress + OFS_TAxCTL) |= ID__2;
            HWREG(baseAddress + OFS_TAxEX0) = (clockSourceDivider / 2 - 1 );
            break;

        case TIMERA_CLOCKSOURCE_DIVIDER_20:
        case TIMERA_CLOCKSOURCE_DIVIDER_24:
        case TIMERA_CLOCKSOURCE_DIVIDER_28:
        case TIMERA_CLOCKSOURCE_DIVIDER_32:
            HWREG(baseAddress + OFS_TAxCTL) |= ID__4;
            HWREG(baseAddress + OFS_TAxEX0) = (clockSourceDivider / 4 - 1);
            break;
        case TIMERA_CLOCKSOURCE_DIVIDER_40:
        case TIMERA_CLOCKSOURCE_DIVIDER_48:
        case TIMERA_CLOCKSOURCE_DIVIDER_56:
        case TIMERA_CLOCKSOURCE_DIVIDER_64:
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
//! \param compareRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareValue is the count to be compared with in compare mode
//!
//! Modified register is \b TAxCCRn
//!
//! \return None
//
//*****************************************************************************
void TimerA_setCompareValue (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned int compareValue
    )
{
    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == compareRegister)
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
void TimerA_clearTimerInterruptFlag (unsigned int baseAddress)
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
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERA_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! Modified bits are CCIFG of \b TAxCCTLn register
//!
//! \return None
//
//*****************************************************************************
void TimerA_clearCaptureCompareInterruptFlag (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERA_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERA_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    HWREG(baseAddress + captureCompareRegister)  &= ~CCIFG;
}

//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************
