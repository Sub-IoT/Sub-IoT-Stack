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
//timerb.c - Driver for the TIMERB Module.
//
//*****************************************************************************
#include "../inc/hw_types.h"
#include "debug.h"
#include "timerb.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "../deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif

void privateTimerBProcessClockSourceDivider (unsigned int baseAddress,
    unsigned int clockSourceDivider);

//*****************************************************************************
//
//! Starts TimerB counter
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERB_CONTINUOUS_MODE [Default value]
//!         \b TIMERB_UPDOWN_MODE
//!         \b TIMERB_UP_MODE

//! Modified register is \b TAxCTL
//!
//!NOTE: This function assumes that the timer has been previously configured
//! using TimerB_configureContinuousMode,  TimerB_configureUpMode or
//!TimerB_configureUpDownMode.
//!
//! \return None
//
//*****************************************************************************
void TimerB_startCounter ( unsigned int baseAddress,
    unsigned int timerMode
    )
{
    ASSERT(
        (TIMERB_UPDOWN_MODE == timerMode) ||
        (TIMERB_CONTINUOUS_MODE == timerMode) ||
        (TIMERB_UP_MODE == timerMode) ||
         );


    HWREG(baseAddress + OFS_TBxCTL) |= timerMode;
}

//*****************************************************************************
//
//! Configures TimerB in continuous mode.
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERB_CLOCKSOURCE_ACLK
//!         \b TIMERB_CLOCKSOURCE_SMCLK
//!         \b TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_64
//! \param timerInterruptEnable_TBIE is to enable or disable TimerB interrupt
//!        Valid values are
//!        \b TIMERB_TBIE_INTERRUPT_ENABLE
//!        \b TIMERB_TBIE_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if TimerB clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERB_DO_CLEAR
//!        \b TIMERB_SKIP_CLEAR [Default value]
//!
//! Modified reister is \b TAxCTL
//!
//!This API does not start the timer. Timer needs to be started when required
//!using the TimerB_startCounter API.
//!
//! \return None
//
//*****************************************************************************
void TimerB_configureContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERB_DO_CLEAR == timerClear) ||
        (TIMERB_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERB_TBIE_INTERRUPT_ENABLE == timerInterruptEnable_TBIE) ||
        (TIMERB_TBIE_INTERRUPT_DISABLE == timerInterruptEnable_TBIE)
        );

    ASSERT(
        (TIMERB_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );


    privateTimerBProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress +
        OFS_TBxCTL) &= ~(TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
        				 TIMERB_STOP_MODE +
                         TIMERB_DO_CLEAR +
                         TIMERB_TBIE_INTERRUPT_ENABLE +
                         CNTL_3
                         );

    HWREG(baseAddress + OFS_TBxCTL)  |= ( clockSource + TIMERB_CONTINUOUS_MODE +
                                          timerClear +
                                          timerInterruptEnable_TBIE);
}
//*****************************************************************************
//
//! Configures TimerB in up mode.
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERB_CLOCKSOURCE_ACLK
//!         \b TIMERB_CLOCKSOURCE_SMCLK
//!         \b TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod is the specified TimerB period. This is the value that gets
//!         written into the CCR0. Limited to 16 bits[unsigned int]
//! \param timerInterruptEnable_TBIE is to enable or disable TimerB interrupt
//!        Valid values are
//!        \b TIMERB_TBIE_INTERRUPT_ENABLE and
//!        \b TIMERB_TBIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         TimerB CCR0 captureComapre interrupt. Valid values are
//!        \b TIMERB_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMERB_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if TimerB clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERB_DO_CLEAR
//!        \b TIMERB_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TAxCTL, \b TAxCCR0, \b TAxCCTL0
//!
//!This API does not start the timer. Timer needs to be started when required
//!using the TimerB_startCounter API.
//!
//! \return None
//
//*****************************************************************************
void TimerB_configureUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERB_DO_CLEAR == timerClear) ||
        (TIMERB_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERB_DO_CLEAR == timerClear) ||
        (TIMERB_SKIP_CLEAR == timerClear)
        );

    privateTimerBProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TBxCTL) &=
        ~(TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMERB_STOP_MODE +
          TIMERB_DO_CLEAR +
          TIMERB_TBIE_INTERRUPT_ENABLE +
          CNTL_3
          );

    HWREG(baseAddress + OFS_TBxCTL)  |= ( clockSource +
                                          TIMERB_UP_MODE +
                                          timerClear +
                                          timerInterruptEnable_TBIE
                                          );

    if (TIMERB_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TBxCCTL0)  |= TIMERB_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TBxCCTL0)  &= ~TIMERB_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TBxCCR0)  = timerPeriod;
}

//*****************************************************************************
//
//! Configures TimerB in up down mode.
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERB_CLOCKSOURCE_ACLK
//!         \b TIMERB_CLOCKSOURCE_SMCLK
//!         \b TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod is the specified TimerB period
//! \param timerInterruptEnable_TBIE is to enable or disable TimerB interrupt
//!        Valid values are
//!        \b TIMERB_TBIE_INTERRUPT_ENABLE
//!        \b TIMERB_TBIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         TimerB CCR0 captureComapre interrupt. Valid values are
//!        \b TIMERB_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMERB_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if TimerB clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERB_DO_CLEAR
//!        \b TIMERB_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TAxCTL, \b TAxCCR0, \b TAxCCTL0
//!
//!This API does not start the timer. Timer needs to be started when required
//!using the TimerB_startCounter API.
//!
//! \return None
//
//*****************************************************************************
void TimerB_configureUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERB_DO_CLEAR == timerClear) ||
        (TIMERB_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERB_DO_CLEAR == timerClear) ||
        (TIMERB_SKIP_CLEAR == timerClear)
        );

    privateTimerBProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TBxCTL) &=
        ~(TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMERB_UPDOWN_MODE +
          TIMERB_DO_CLEAR +
          TIMERB_TBIE_INTERRUPT_ENABLE +
          CNTL_3
          );

    HWREG(baseAddress + OFS_TBxCTL)  |= ( clockSource +
    									  TIMERB_STOP_MODE +
                                          timerClear +
                                          timerInterruptEnable_TBIE
                                          );
    if (TIMERB_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TBxCCTL0)  |= TIMERB_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TBxCCTL0)  &= ~TIMERB_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TBxCCR0)  = timerPeriod;
}

//*****************************************************************************
//
//! DEPRPECATED - Replaced by TimerB_configureContinuousMode and TimerB_startCounter
//! API
//! Starts TimerB in continuous mode.
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERB_CLOCKSOURCE_ACLK
//!         \b TIMERB_CLOCKSOURCE_SMCLK
//!         \b TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_64
//! \param timerInterruptEnable_TBIE is to enable or disable TimerB interrupt
//!        Valid values are
//!        \b TIMERB_TBIE_INTERRUPT_ENABLE
//!        \b TIMERB_TBIE_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if TimerB clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERB_DO_CLEAR
//!        \b TIMERB_SKIP_CLEAR [Default value]
//!
//! Modified reister is \b TAxCTL
//!
//! \return None
//
//*****************************************************************************
void TimerB_startContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERB_DO_CLEAR == timerClear) ||
        (TIMERB_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERB_TBIE_INTERRUPT_ENABLE == timerInterruptEnable_TBIE) ||
        (TIMERB_TBIE_INTERRUPT_DISABLE == timerInterruptEnable_TBIE)
        );

    ASSERT(
        (TIMERB_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMERB_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );


    privateTimerBProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress +
        OFS_TBxCTL) &= ~(TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
                         TIMERB_UPDOWN_MODE +
                         TIMERB_DO_CLEAR +
                         TIMERB_TBIE_INTERRUPT_ENABLE +
                         CNTL_3
                         );

    HWREG(baseAddress + OFS_TBxCTL)  |= ( clockSource + TIMERB_CONTINUOUS_MODE +
                                          timerClear +
                                          timerInterruptEnable_TBIE);
}

//*****************************************************************************
//
//! DEPRECATED- Spelling Error Fixed
//! Starts TimerB in continuous mode.
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERB_CLOCKSOURCE_ACLK
//!         \b TIMERB_CLOCKSOURCE_SMCLK
//!         \b TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_64
//! \param timerInterruptEnable_TBIE is to enable or disable TimerB interrupt
//!        Valid values are
//!        \b TIMERB_TBIE_INTERRUPT_ENABLE
//!        \b TIMERB_TBIE_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if TimerB clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERB_DO_CLEAR
//!        \b TIMERB_SKIP_CLEAR [Default value]
//!
//! Modified register is \b TAxCTL
//!
//! \return None
//
//*****************************************************************************
void TimerB_startContinousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int timerClear
    )
{
	TimerB_startContinuousMode (baseAddress,
    clockSource,
    clockSourceDivider,
    timerInterruptEnable_TBIE,
    timerClear
    );
}
//*****************************************************************************
//
//! DEPRPECATED - Replaced by TimerB_configureUpMode and TimerB_startCounter  API
//!
//! Starts TimerB in up mode.
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERB_CLOCKSOURCE_ACLK
//!         \b TIMERB_CLOCKSOURCE_SMCLK
//!         \b TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod is the specified TimerB period. This is the value that gets
//!         written into the CCR0. Limited to 16 bits[unsigned int]
//! \param timerInterruptEnable_TBIE is to enable or disable TimerB interrupt
//!        Valid values are
//!        \b TIMERB_TBIE_INTERRUPT_ENABLE and
//!        \b TIMERB_TBIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         TimerB CCR0 captureComapre interrupt. Valid values are
//!        \b TIMERB_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMERB_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if TimerB clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERB_DO_CLEAR
//!        \b TIMERB_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TAxCTL, \b TAxCCR0, \b TAxCCTL0
//!
//! \return None
//
//*****************************************************************************
void TimerB_startUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERB_DO_CLEAR == timerClear) ||
        (TIMERB_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERB_DO_CLEAR == timerClear) ||
        (TIMERB_SKIP_CLEAR == timerClear)
        );

    privateTimerBProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TBxCTL) &=
        ~(TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMERB_UPDOWN_MODE +
          TIMERB_DO_CLEAR +
          TIMERB_TBIE_INTERRUPT_ENABLE +
          CNTL_3
          );

    HWREG(baseAddress + OFS_TBxCTL)  |= ( clockSource +
                                          TIMERB_UP_MODE +
                                          timerClear +
                                          timerInterruptEnable_TBIE
                                          );

    if (TIMERB_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TBxCCTL0)  |= TIMERB_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TBxCCTL0)  &= ~TIMERB_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TBxCCR0)  = timerPeriod;
}

//*****************************************************************************
//
//! DEPRPECATED - Replaced by TimerB_configureUpDownMode and TimerB_startCounter  API
//!
//! Starts TimerB in up down mode.
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK [Default value]
//!         \b TIMERB_CLOCKSOURCE_ACLK
//!         \b TIMERB_CLOCKSOURCE_SMCLK
//!         \b TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod is the specified TimerB period
//! \param timerInterruptEnable_TBIE is to enable or disable TimerB interrupt
//!        Valid values are
//!        \b TIMERB_TBIE_INTERRUPT_ENABLE
//!        \b TIMERB_TBIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         TimerB CCR0 captureComapre interrupt. Valid values are
//!        \b TIMERB_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMERB_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if TimerB clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERB_DO_CLEAR
//!        \b TIMERB_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TAxCTL, \b TAxCCR0, \b TAxCCTL0
//!
//! \return None
//
//*****************************************************************************
void TimerB_startUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TBIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    )
{
    ASSERT(
        (TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT(
        (TIMERB_DO_CLEAR == timerClear) ||
        (TIMERB_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERB_DO_CLEAR == timerClear) ||
        (TIMERB_SKIP_CLEAR == timerClear)
        );

    privateTimerBProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TBxCTL) &=
        ~(TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
          TIMERB_UPDOWN_MODE +
          TIMERB_DO_CLEAR +
          TIMERB_TBIE_INTERRUPT_ENABLE +
          CNTL_3
          );

    HWREG(baseAddress + OFS_TBxCTL)  |= ( clockSource +
                                          TIMERB_UPDOWN_MODE +
                                          timerClear +
                                          timerInterruptEnable_TBIE
                                          );
    if (TIMERB_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TBxCCTL0)  |= TIMERB_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TBxCCTL0)  &= ~TIMERB_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TBxCCR0)  = timerPeriod;
}

//*****************************************************************************
//
//! Initializes Capture Mode
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param captureMode is the capture mode selected. Valid values are
//!        \b TIMERB_CAPTUREMODE_NO_CAPTURE [Default value]
//!        \b TIMERB_CAPTUREMODE_RISING_EDGE
//!        \b TIMERB_CAPTUREMODE_FALLING_EDGE
//!        \b TIMERB_CAPTUREMODE_RISING_AND_FALLING_EDGE
//! \param captureInputSelect decides the Input Select
//!        \b TIMERB_CAPTURE_INPUTSELECT_CCIxA [Default value]
//!        \b TIMERB_CAPTURE_INPUTSELECT_CCIxB
//!        \b TIMERB_CAPTURE_INPUTSELECT_GND
//!        \b TIMERB_CAPTURE_INPUTSELECT_Vcc
//! \param synchronizeCaptureSource decides if capture source should be
//!         synchronized with TimerB clock
//!        Valid values are
//!        \b TIMERB_CAPTURE_ASYNCHRONOUS [Default value]
//!        \b TIMERB_CAPTURE_SYNCHRONOUS
//! \param captureInterruptEnable is to enable or disable
//!         TimerB captureComapre interrupt. Valid values are
//!        \b TIMERB_CAPTURECOMPARE_INTERRUPT_DISABLE [Default value]
//!        \b TIMERB_CAPTURECOMPARE_INTERRUPT_ENABLE
//! \param captureOutputMode specifies the ouput mode. Valid values are
//!        \b TIMERB_OUTPUTMODE_OUTBITVALUE [Default value],
//!        \b TIMERB_OUTPUTMODE_SET,
//!        \b TIMERB_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMERB_OUTPUTMODE_SET_RESET
//!        \b TIMERB_OUTPUTMODE_TOGGLE,
//!        \b TIMERB_OUTPUTMODE_RESET,
//!        \b TIMERB_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMERB_OUTPUTMODE_RESET_SET
//!
//! Modified register is \b TAxCCTLn
//! \return None
//
//*****************************************************************************
void TimerB_initCapture (unsigned int baseAddress,
    unsigned int captureRegister,
    unsigned int captureMode,
    unsigned int captureInputSelect,
    unsigned short synchronizeCaptureSource,
    unsigned short captureInterruptEnable,
    unsigned int captureOutputMode
    )
{
    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == captureRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == captureRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == captureRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == captureRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == captureRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == captureRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == captureRegister)
        );

    ASSERT((TIMERB_CAPTUREMODE_NO_CAPTURE == captureMode) ||
        (TIMERB_CAPTUREMODE_RISING_EDGE == captureMode) ||
        (TIMERB_CAPTUREMODE_FALLING_EDGE == captureMode) ||
        (TIMERB_CAPTUREMODE_RISING_AND_FALLING_EDGE == captureMode)
        );

    ASSERT((TIMERB_CAPTURE_INPUTSELECT_CCIxA == captureInputSelect) ||
        (TIMERB_CAPTURE_INPUTSELECT_CCIxB == captureInputSelect) ||
        (TIMERB_CAPTURE_INPUTSELECT_GND == captureInputSelect) ||
        (TIMERB_CAPTURE_INPUTSELECT_Vcc == captureInputSelect)
        );

    ASSERT((TIMERB_CAPTURE_ASYNCHRONOUS == synchronizeCaptureSource) ||
        (TIMERB_CAPTURE_SYNCHRONOUS == synchronizeCaptureSource)
        );

    ASSERT(
        (TIMERB_CAPTURECOMPARE_INTERRUPT_DISABLE == captureInterruptEnable) ||
        (TIMERB_CAPTURECOMPARE_INTERRUPT_ENABLE == captureInterruptEnable)
        );

    ASSERT((TIMERB_OUTPUTMODE_OUTBITVALUE == captureOutputMode) ||
        (TIMERB_OUTPUTMODE_SET == captureOutputMode) ||
        (TIMERB_OUTPUTMODE_TOGGLE_RESET == captureOutputMode) ||
        (TIMERB_OUTPUTMODE_SET_RESET == captureOutputMode) ||
        (TIMERB_OUTPUTMODE_TOGGLE == captureOutputMode) ||
        (TIMERB_OUTPUTMODE_RESET == captureOutputMode) ||
        (TIMERB_OUTPUTMODE_TOGGLE_SET == captureOutputMode) ||
        (TIMERB_OUTPUTMODE_RESET_SET == captureOutputMode)
        );

    if (TIMERB_CAPTURECOMPARE_REGISTER_0 == captureRegister){
        //CaptureCompare register 0 only supports certain modes
        ASSERT((TIMERB_OUTPUTMODE_OUTBITVALUE == captureOutputMode) ||
            (TIMERB_OUTPUTMODE_SET == captureOutputMode) ||
            (TIMERB_OUTPUTMODE_TOGGLE == captureOutputMode) ||
            (TIMERB_OUTPUTMODE_RESET == captureOutputMode)
            );
    }

    HWREG(baseAddress + captureRegister ) |=   CAP;

    HWREG(baseAddress + captureRegister) &=
        ~(TIMERB_CAPTUREMODE_RISING_AND_FALLING_EDGE +
          TIMERB_CAPTURE_INPUTSELECT_Vcc +
          TIMERB_CAPTURE_SYNCHRONOUS +
          TIMERB_DO_CLEAR +
          TIMERB_TBIE_INTERRUPT_ENABLE +
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
//! \param baseAddress is the base address of the TimerB module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareInterruptEnable is to enable or disable
//!         TimerB captureComapre interrupt. Valid values are
//!        \b TIMERB_CAPTURECOMPARE_INTERRUPT_ENABLE and
//!        \b TIMERB_CAPTURECOMPARE_INTERRUPT_DISABLE [Default value]
//! \param compareOutputMode specifies the ouput mode. Valid values are
//!        \b TIMERB_OUTPUTMODE_OUTBITVALUE [Default value],
//!        \b TIMERB_OUTPUTMODE_SET,
//!        \b TIMERB_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMERB_OUTPUTMODE_SET_RESET
//!        \b TIMERB_OUTPUTMODE_TOGGLE,
//!        \b TIMERB_OUTPUTMODE_RESET,
//!        \b TIMERB_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMERB_OUTPUTMODE_RESET_SET
//! \param compareValue is the count to be compared with in compare mode
//!
//! Modified register is \b TAxCCTLn and \b TAxCCRn
//! \return None
//
//*****************************************************************************
void TimerB_initCompare (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned short compareInterruptEnable,
    unsigned int compareOutputMode,
    unsigned int compareValue
    )
{
    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == compareRegister)
        );

   ASSERT((TIMERB_CAPTURECOMPARE_INTERRUPT_ENABLE == compareInterruptEnable) ||
        (TIMERB_CAPTURECOMPARE_INTERRUPT_DISABLE == compareInterruptEnable)
        );

    ASSERT((TIMERB_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_SET == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_TOGGLE_RESET == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_SET_RESET == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_TOGGLE == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_RESET == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_TOGGLE_SET == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_RESET_SET == compareOutputMode)
        );

    if (TIMERB_CAPTURECOMPARE_REGISTER_0 == compareRegister){
        //CaptureCompare register 0 only supports certain modes
        ASSERT((TIMERB_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
            (TIMERB_OUTPUTMODE_SET == compareOutputMode) ||
            (TIMERB_OUTPUTMODE_TOGGLE == compareOutputMode) ||
            (TIMERB_OUTPUTMODE_RESET == compareOutputMode)
            );
    }


    HWREG(baseAddress + compareRegister ) &=   ~CAP;

    HWREG(baseAddress + compareRegister) &=
        ~(TIMERB_CAPTURECOMPARE_INTERRUPT_ENABLE +
          TIMERB_OUTPUTMODE_RESET_SET
          );

    HWREG(baseAddress + compareRegister)  |= ( compareInterruptEnable +
                                               compareOutputMode
                                               );

    HWREG(baseAddress + compareRegister + OFS_TBxR) = compareValue;
}

//*****************************************************************************
//
//! Enable TimerB interrupt
//!
//! \param baseAddress is the base address of the TimerB module.
//!
//! Modified register is TAxCTL
//!
//! \return None
//
//*****************************************************************************
void TimerB_enableInterrupt (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_TBxCTL) &=  ~TBIFG;
    HWREG(baseAddress + OFS_TBxCTL) |= TBIE;
}

//*****************************************************************************
//
//! Disable TimerB interrupt
//!
//! \param baseAddress is the base address of the TimerB module.
//!
//! Modified register is \b TAxCTL
//!
//! \return None
//
//*****************************************************************************
void TimerB_disableInterrupt (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TBxCTL) &= ~TBIE;
}

//*****************************************************************************
//
//! Get TimerB interrupt status
//!
//! \param baseAddress is the base address of the TimerB module.
//!
//! \return unsigned long. Return interrupt status. Valid values are
//!         \b TIMERB_INTERRUPT_PENDING
//!         \b TIMERB_INTERRUPT_NOT_PENDING
//
//*****************************************************************************
unsigned long TimerB_getInterruptStatus (unsigned int baseAddress)
{
    return ( HWREG(baseAddress + OFS_TBxCTL) & TBIFG );
}

//*****************************************************************************
//
//! Enable capture compare interrupt
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param captureCompareRegister is the selected capture compare regsiter
//!
//! Modified register is \b TAxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerB_enableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    HWREGB(baseAddress + captureCompareRegister) &= ~CCIFG;
    HWREG(baseAddress + captureCompareRegister) |= CCIE;
}

//*****************************************************************************
//
//! Disable capture compare interrupt
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param captureCompareRegister is the selected capture compare regsiter
//!
//! Modified register is \b TAxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerB_disableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );
    HWREG(baseAddress + captureCompareRegister) &= ~CCIE;
}

//*****************************************************************************
//
//! Return capture compare interrupt status
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param mask is the mask for the interrupt status
//!         Valid values is and OR of
//!         \b TIMERB_CAPTURE_OVERFLOW,
//!         \b TIMERB_CAPTURECOMPARE_INTERRUPT_FLAG
//!
//!
//! \returns unsigned long. The mask of the set flags.
//
//*****************************************************************************
unsigned long TimerB_getCaptureCompareInterruptStatus (unsigned int baseAddress,
		 unsigned int captureCompareRegister,
		 unsigned int mask
		 )
{
    return ( HWREG(baseAddress + captureCompareRegister) & mask );
}

//*****************************************************************************
//
//! Reset/Clear the TimerB clock divider, count direction, count
//!
//! \param baseAddress is the base address of the TimerB module.
//!
//! Modified register is \b TAxCTL
//!
//! \returns None
//
//*****************************************************************************
void TimerB_clear (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TBxCTL) |= TBCLR;
}

//*****************************************************************************
//
//! Get synchrnozied capturecompare input
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param synchronized is to select type of capture compare input.
//!         Valid values are
//!        \b TIMERB_READ_CAPTURE_COMPARE_INPUT
//!        \b TIMERB_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT
//!
//! \return \b TIMERB_CAPTURECOMPARE_INPUT_HIGH or
//!         \b TIMERB_CAPTURECOMPARE_INPUT_LOW
//
//*****************************************************************************
unsigned short TimerB_getSynchronizedCaptureCompareInput
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned short synchronized
    )
{
    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    ASSERT((TIMERB_READ_CAPTURE_COMPARE_INPUT == synchronized) ||
        (TIMERB_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT == synchronized)
        );

    if (HWREG(baseAddress + captureCompareRegister) & synchronized){
        return ( TIMERB_CAPTURECOMPARE_INPUT_HIGH) ;
    } else   {
        return ( TIMERB_CAPTURECOMPARE_INPUT_LOW) ;
    }
}

//*****************************************************************************
//
//! Get ouput bit for output mode
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! \return \b TIMERB_OUTPUTMODE_OUTBITVALUE_HIGH or
//!         \b TIMERB_OUTPUTMODE_OUTBITVALUE_LOW
//
//*****************************************************************************
unsigned char TimerB_getOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    if (HWREG(baseAddress + captureCompareRegister) & OUT){
        return ( TIMERB_OUTPUTMODE_OUTBITVALUE_HIGH) ;
    } else   {
        return ( TIMERB_OUTPUTMODE_OUTBITVALUE_LOW) ;
    }
}

//*****************************************************************************
//
//! Get current capturecompare count
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! \return current count as unsigned int
//
//*****************************************************************************
unsigned int TimerB_getCaptureCompareCount
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    return  (HWREG(baseAddress + OFS_TBxR + captureCompareRegister));
}

//*****************************************************************************
//
//! Set ouput bit for output mode
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param captureCompareRegister selects the Capture register being used.
//!     are
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param outputModeOutBitValueis the value to be set for out bit
//!     Valid values are \b TIMERB_OUTPUTMODE_OUTBITVALUE_HIGH
//!                      \b TIMERB_OUTPUTMODE_OUTBITVALUE_LOW
//!
//! Modified register is \b TAxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerB_setOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned char outputModeOutBitValue
    )
{
    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    ASSERT((TIMERB_OUTPUTMODE_OUTBITVALUE_HIGH == outputModeOutBitValue) ||
        (TIMERB_OUTPUTMODE_OUTBITVALUE_LOW == outputModeOutBitValue)
        );

    HWREG(baseAddress + captureCompareRegister) &= ~OUT;
    HWREG(baseAddress + captureCompareRegister) |= outputModeOutBitValue;
}

//*****************************************************************************
//
//! Generate a PWM with TimerB running in up down mode
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK
//!         \b TIMERB_CLOCKSOURCE_ACLK
//!         \b TIMERB_CLOCKSOURCE_SMCLK
//!         \b TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK
//! \param clockSourceDivider is the divider for Clock source. Valid values are
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_1
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERB_CLOCKSOURCE_DIVIDER_64
//! \param timerPeriod selects the desired TimerB period
//! \param compareRegister selects the compare register being used. Valid values
//!     are
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareOutputMode specifies the ouput mode. Valid values are
//!        \b TIMERB_OUTPUTMODE_OUTBITVALUE,
//!        \b TIMERB_OUTPUTMODE_SET,
//!        \b TIMERB_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMERB_OUTPUTMODE_SET_RESET
//!        \b TIMERB_OUTPUTMODE_TOGGLE,
//!        \b TIMERB_OUTPUTMODE_RESET,
//!        \b TIMERB_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMERB_OUTPUTMODE_RESET_SET
//! \param dutyCycle specifies the dutycycle for the generated waveform
//!
//! Modified registers are \b TAxCTL, \b TAxCCR0, \b TAxCCTL0,\b TAxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerB_generatePWM (  unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int timerPeriod,
    unsigned int compareRegister,
    unsigned int compareOutputMode,
    unsigned int dutyCycle
    )
{
    ASSERT(
        (TIMERB_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );

    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == compareRegister)
        );


    ASSERT((TIMERB_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_SET == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_TOGGLE_RESET == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_SET_RESET == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_TOGGLE == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_RESET == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_TOGGLE_SET == compareOutputMode) ||
        (TIMERB_OUTPUTMODE_RESET_SET == compareOutputMode)
        );

    privateTimerBProcessClockSourceDivider(baseAddress,
        clockSourceDivider
        );

    HWREG(baseAddress + OFS_TBxCTL)  &=
        ~( TIMERB_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK +
           TIMERB_UPDOWN_MODE + TIMERB_DO_CLEAR +
           TIMERB_TBIE_INTERRUPT_ENABLE
           );

    HWREG(baseAddress + OFS_TBxCTL)  |= ( clockSource +
                                          TIMERB_UP_MODE +
                                          TIMERB_DO_CLEAR
                                          );

    HWREG(baseAddress + OFS_TBxCCR0)  = timerPeriod;

    HWREG(baseAddress + OFS_TBxCCTL0)  &=
        ~(TIMERB_CAPTURECOMPARE_INTERRUPT_ENABLE +
          TIMERB_OUTPUTMODE_RESET_SET
          );
    HWREG(baseAddress + compareRegister)  |= compareOutputMode;

    HWREG(baseAddress + compareRegister + OFS_TBxR) = dutyCycle;
}

//*****************************************************************************
//
//! Stops the TimerB
//!
//! \param baseAddress is the base address of the TimerB module.
//!
//! Modified registers are \b TAxCTL
//!
//! \returns None
//
//*****************************************************************************
void TimerB_stop ( unsigned int baseAddress )
{
    HWREG(baseAddress + OFS_TBxCTL)  &= ~MC_3;
    HWREG(baseAddress + OFS_TBxCTL)  |= MC_0;
}

//*****************************************************************************
//
//! Private clock source divider helper function
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param clockSourceDivider is the desired divider for the clock source
//!
//! Modified registers are TAxEX0, TAxCTL
//!
//! \returns None
//
//*****************************************************************************
void privateTimerBProcessClockSourceDivider (unsigned int baseAddress,
    unsigned int clockSourceDivider)
{
    HWREG(baseAddress + OFS_TBxCTL) &= ~ID__8;
    HWREG(baseAddress + OFS_TBxEX0) &= ~TBIDEX_7;
    switch (clockSourceDivider){
        case TIMERB_CLOCKSOURCE_DIVIDER_1:
        case TIMERB_CLOCKSOURCE_DIVIDER_2:
        case TIMERB_CLOCKSOURCE_DIVIDER_4:
        case TIMERB_CLOCKSOURCE_DIVIDER_8:
            HWREG(baseAddress + OFS_TBxCTL) |= ((clockSourceDivider - 1) << 6);
            HWREG(baseAddress + OFS_TBxEX0) = TBIDEX_0;
            break;

        case TIMERB_CLOCKSOURCE_DIVIDER_3:
        case TIMERB_CLOCKSOURCE_DIVIDER_5:
        case TIMERB_CLOCKSOURCE_DIVIDER_6:
        case TIMERB_CLOCKSOURCE_DIVIDER_7:
            HWREG(baseAddress + OFS_TBxCTL) |= ID__1;
            HWREG(baseAddress + OFS_TBxEX0) = (clockSourceDivider - 1);
            break;

        case TIMERB_CLOCKSOURCE_DIVIDER_10:
        case TIMERB_CLOCKSOURCE_DIVIDER_12:
        case TIMERB_CLOCKSOURCE_DIVIDER_14:
        case TIMERB_CLOCKSOURCE_DIVIDER_16:
            HWREG(baseAddress + OFS_TBxCTL) |= ID__2;
            HWREG(baseAddress + OFS_TBxEX0) = (clockSourceDivider / 2 - 1 );
            break;

        case TIMERB_CLOCKSOURCE_DIVIDER_20:
        case TIMERB_CLOCKSOURCE_DIVIDER_24:
        case TIMERB_CLOCKSOURCE_DIVIDER_28:
        case TIMERB_CLOCKSOURCE_DIVIDER_32:
            HWREG(baseAddress + OFS_TBxCTL) |= ID__4;
            HWREG(baseAddress + OFS_TBxEX0) = (clockSourceDivider / 4 - 1);
            break;
        case TIMERB_CLOCKSOURCE_DIVIDER_40:
        case TIMERB_CLOCKSOURCE_DIVIDER_48:
        case TIMERB_CLOCKSOURCE_DIVIDER_56:
        case TIMERB_CLOCKSOURCE_DIVIDER_64:
            HWREG(baseAddress + OFS_TBxCTL) |= ID__8;
            HWREG(baseAddress + OFS_TBxEX0) = (clockSourceDivider / 8 - 1);
            break;
    }
}

//*****************************************************************************
//
//! Sets the value of the capture-compare register
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param compareRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareValue is the count to be compared with in compare mode
//!
//! Modified register is \b TAxCCRn
//!
//! \return None
//
//*****************************************************************************
void TimerB_setCompareValue (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned int compareValue
    )
{
    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == compareRegister)
        );

    HWREG(baseAddress + compareRegister + OFS_TBxR) = compareValue;
}

//*****************************************************************************
//
//! Clears the TimerB TBIFG interrupt flag
//!
//! \param baseAddress is the base address of the TimerB module.
//!
//! Modified bits are TBIFG og TAxCTL register
//!
//! \return None
//
//*****************************************************************************
void TimerB_clearTimerInterruptFlag (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_TBxCTL) &= ~TBIFG;
}

//*****************************************************************************
//
//! Clears the capture-compare interrupt flag
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param captureCompareRegister selects the Capture-compare register being
//! used. Valid values are
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! Modified bits are CCIFG of \b TAxCCTLn register
//!
//! \return None
//
//*****************************************************************************
void TimerB_clearCaptureCompareInterruptFlag (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERB_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERB_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    HWREG(baseAddress + captureCompareRegister)  &= ~CCIFG;
}

//*****************************************************************************
//
//! Selects TimerB counter length
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param counterLength selects the value of counter length.
//! Valid values are
//!     \b TIMERB_COUNTER_16BIT [Default value]
//!     \b TIMERB_COUNTER_12BIT
//!     \b TIMERB_COUNTER_10BIT
//!     \b TIMERB_COUNTER_8BIT
//!
//! Modified bits are CNTL of \b TBxCTL register
//!
//! \return None
//
//*****************************************************************************
void TimerB_selectCounterLength (unsigned int  baseAddress,
		unsigned int counterLength
		)
{
	ASSERT((TIMERB_COUNTER_8BIT == counterLength) ||
	        (TIMERB_COUNTER_10BIT == counterLength) ||
	        (TIMERB_COUNTER_12BIT == counterLength) ||
	        (TIMERB_COUNTER_16BIT == counterLength)
	        );


	HWREG(baseAddress + OFS_TBxCTL) &= ~CNTL_3;
	HWREG(baseAddress + OFS_TBxCTL) |= counterLength;
}
//*****************************************************************************
//
//! Selects TimerB Latching Group
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param groupLatch selects the value of counter length.
//! Valid values are
//!     \b TIMERB_GROUP_NONE [Default value]
//!     \b TIMERB_GROUP_CL12_CL23_CL56
//!     \b TIMERB_GROUP_CL123_CL456
//!     \b TIMERB_GROUP_ALL
//!
//! Modified bits are TBCLGRP of \b TBxCTL register
//!
//! \return None
//
//*****************************************************************************
void TimerB_selectLatchingGroup(unsigned int  baseAddress,
		unsigned int  groupLatch)
{
	ASSERT((TIMERB_GROUP_NONE  == groupLatch) ||
		   (TIMERB_GROUP_CL12_CL23_CL56 == groupLatch) ||
		   (TIMERB_GROUP_CL123_CL456 == groupLatch) ||
		   (TIMERB_GROUP_ALL == groupLatch)
		   );


	HWREG(baseAddress + OFS_TBxCTL) &= ~TBCLGRP_3;
	HWREG(baseAddress + OFS_TBxCTL) |= groupLatch;
}
//*****************************************************************************
//
//! Selects Compare Latch Load Event
//!
//! \param baseAddress is the base address of the TimerB module.
//! \param compareRegister selects the Capture-compare register being
//! used. Valid values are
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERB_CAPTURECOMPARE_REGISTER_6
//! \param compareLatchLoadEvent selects the latch load event
//! Valid values are
//!     \b TIMERB_LATCH_ON_WRITE_TO_TBxCCRn_COMPARE_REGISTER [Default value]
//!     \b TIMERB_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UP_OR_CONT_MODE
//!     \b TIMERB_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UPDOWN_MODE
//!     \b TIMERB_LATCH_WHEN_COUNTER_COUNTS_TO_CURRENT_COMPARE_LATCH_VALUE
//!
//! Modified bits are CLLD of \b TBxCCTLn register
//!
//! \return None
//
//*****************************************************************************
void TimerB_initCompareLatchLoadEvent(unsigned int  baseAddress,
		unsigned int  compareRegister,
		unsigned int  compareLatchLoadEvent
		)
{
	ASSERT((TIMERB_LATCH_ON_WRITE_TO_TBxCCRn_COMPARE_REGISTER  == groupLatch) ||
		(TIMERB_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UP_OR_CONT_MODE == groupLatch) ||
		(TIMERB_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UPDOWN_MODE == groupLatch) ||
		(TIMERB_LATCH_WHEN_COUNTER_COUNTS_TO_CURRENT_COMPARE_LATCH_VALUE == groupLatch)
		);

	HWREG(baseAddress + compareRegister)  &= ~CLLD_3;
	HWREG(baseAddress + compareRegister)  |= compareLatchLoadEvent;
}

//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************
