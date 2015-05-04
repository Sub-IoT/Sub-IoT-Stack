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
//timer.c - Driver for the TIMER Module.
//
//*****************************************************************************
#include "../inc/hw_types.h"
#include "debug.h"
#include "timerd.h"
#include "tlv.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "../deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif

void privateTimerDProcessClockSourceDivider (unsigned int baseAddress,
    unsigned int clockSourceDivider);

//*****************************************************************************
//
//! Starts TimerD counter
//!
//! \param baseAddress is the base address of the TimerA module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERD_CONTINUOUS_MODE [Default value]
//!         \b TIMERD_UPDOWN_MODE
//!         \b TIMERD_UP_MODE
//! Modified register is \b TDxCTL0
//!
//!NOTE: This function assumes that the timer has been previously configured
//! using TimerD_configureContinuousMode,  TimerD_configureUpMode or
//!TimerD_configureUpDownMode.
//!
//! \return None
//
//*****************************************************************************
void TimerD_startCounter ( unsigned int baseAddress,
    unsigned int timerMode
    )
{
    ASSERT(
        (TIMERA_UPDOWN_MODE == timerMode) ||
        (TIMERA_CONTINUOUS_MODE == timerMode) ||
        (TIMERA_UP_MODE == timerMode) ||
         );


    HWREG(baseAddress + OFS_TDxCTL0) |= timerMode;
}
//*****************************************************************************
//
//! Configures timer in continuous mode.
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERD_CLOCKSOURCE_EXTERNAL_TDCLK [Default value]
//!         \b TIMERD_CLOCKSOURCE_ACLK
//!         \b TIMERD_CLOCKSOURCE_SMCLK
//!         \b TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK
//! \param clockSourceDivider is the divider for Clock source.
//! 	Valid values are
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_64
//! \param clockingMode is the selected clock mode register values.
//! Valid values are
//!     \b TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK [Default value]
//!     \b TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK
//!	\b TIMERD_CLOCKINGMODE_AUXILIARY_CLK
//! \param timerInterruptEnable_TDIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMERD_TDIE_INTERRUPT_ENABLE
//!        \b TIMERD_TDIE_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERD_DO_CLEAR
//!        \b TIMERD_SKIP_CLEAR [Default value]
//!
//! Modified registers are \b TDxCTL0 and \b TDxCTL1
//!
//!This API does not start the timer. Timer needs to be started when required
//!using the TimerD_start API.
//!
//! \return None
//
//*****************************************************************************
void TimerD_configureContinuousMode ( unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int clockingMode,
    unsigned int timerInterruptEnable_TDIE,
    unsigned int timerClear
    )
{
    
    ASSERT(
        (TIMERD_DO_CLEAR == timerClear) ||
        (TIMERD_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERD_TDIE_INTERRUPT_ENABLE == timerInterruptEnable_TDIE) ||
        (TIMERD_TDIE_INTERRUPT_DISABLE == timerInterruptEnable_TDIE)
        );
    
    ASSERT(
        (TIMERD_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );


    ASSERT(
        (TIMERD_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );

    ASSERT(
        (TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_AUXILIARY_CLK == clockingMode)
        );

    HWREG(baseAddress +
        OFS_TDxCTL0) &= ~(TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK +
        				 TIMERD_STOP_MODE +
                         TIMERD_DO_CLEAR +
                         TIMERD_TDIE_INTERRUPT_ENABLE
                         );

    HWREG(baseAddress + OFS_TDxCTL1)  &= ~(TDCLKM0 + TDCLKM1);
    
    privateTimerDProcessClockSourceDivider(baseAddress,
    		        clockSourceDivider
    		        );
    HWREG(baseAddress + OFS_TDxCTL0)  |=  clockSource;
    HWREG(baseAddress + OFS_TDxCTL1) |= clockingMode;
  
    HWREG(baseAddress + OFS_TDxCTL0)  |= ( TIMERD_CONTINUOUS_MODE +
        	                                          timerClear +
        	                                          timerInterruptEnable_TDIE);

}
//*****************************************************************************
//
//! Configures timer in up mode.
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERD_CLOCKSOURCE_EXTERNAL_TDCLK [Default value]
//!         \b TIMERD_CLOCKSOURCE_ACLK
//!         \b TIMERD_CLOCKSOURCE_SMCLK
//!         \b TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK
//! \param clockSourceDivider is the divider for Clock source.
//! 	Valid values are
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_64
//! \param clockingMode is the selected clock mode register values.
//! Valid values are
//!     \b TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK [Default value]
//!     \b TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK
//!	\b TIMERD_CLOCKINGMODE_AUXILIARY_CLK
//! \param timerPeriod is the specified timer period. This is the value that gets
//!         written into the CCR0. Limited to 16 bits[unsigned int]
//! \param timerInterruptEnable_TDIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMERD_TDIE_INTERRUPT_ENABLE and
//!        \b TIMERD_TDIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         timer CCR0 captureComapre interrupt. Valid values are
//!        \b TIMERD_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMERD_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERD_DO_CLEAR
//!        \b TIMERD_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TDxCTL0, \b TDxCTL1,\b TDxCCR0, \b TDxCCTL0
//!
//!This API does not start the timer. Timer needs to be started when required
//!using the TimerD_start API.
//!
//! \return None
//
//*****************************************************************************
void TimerD_configureUpMode (   unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int clockingMode,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TDIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    )
{

    ASSERT(
        (TIMERD_DO_CLEAR == timerClear) ||
        (TIMERD_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERD_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );


    ASSERT(
        (TIMERD_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );

    ASSERT(
        (TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_AUXILIARY_CLK == clockingMode)
        );

    HWREG(baseAddress + OFS_TDxCTL0) &=
        ~(TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK +
          TIMERD_UPDOWN_MODE +
          TIMERD_DO_CLEAR +
          TIMERD_TDIE_INTERRUPT_ENABLE
          );

    HWREG(baseAddress + OFS_TDxCTL1)  &= ~(TDCLKM0 + TDCLKM1);

    privateTimerDProcessClockSourceDivider(baseAddress,
    		        clockSourceDivider
    		        );
    HWREG(baseAddress + OFS_TDxCTL0)  |=  clockSource;
    HWREG(baseAddress + OFS_TDxCTL1) |= clockingMode;

    HWREG(baseAddress + OFS_TDxCTL0)  |= ( TIMERD_STOP_MODE +
                                          timerClear +
                                          timerInterruptEnable_TDIE
                                          );

    if (TIMERD_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TDxCCTL0)  |= TIMERD_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TDxCCTL0)  &= ~TIMERD_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TDxCCR0)  = timerPeriod;
}

//*****************************************************************************
//
//! Configures timer in up down mode.
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERD_CLOCKSOURCE_EXTERNAL_TDCLK [Default value]
//!         \b TIMERD_CLOCKSOURCE_ACLK
//!         \b TIMERD_CLOCKSOURCE_SMCLK
//!         \b TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK
//! \param clockSourceDivider is the divider for Clock source.
//! 	Valid values are
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_64
//! \param clockingMode is the selected clock mode register values.
//! Valid values are
//!     \b TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK [Default value]
//!     \b TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK
//!	\b TIMERD_CLOCKINGMODE_AUXILIARY_CLK
//! \param timerPeriod is the specified timer period
//! \param timerInterruptEnable_TDIE is to enable or disable timer interrupt
//!        Valid values are
//!        \b TIMERD_TDIE_INTERRUPT_ENABLE
//!        \b TIMERD_TDIE_INTERRUPT_DISABLE [Default value]
//! \param captureCompareInterruptEnable_CCR0_CCIE is to enable or disable
//!         timer CCR0 captureComapre interrupt. Valid values are
//!        \b TIMERD_CCIE_CCR0_INTERRUPT_ENABLE and
//!        \b TIMERD_CCIE_CCR0_INTERRUPT_DISABLE [Default value]
//! \param timerClear decides if timer clock divider, count direction, count
//!        need to be reset. Valid values are
//!        \b TIMERD_DO_CLEAR
//!        \b TIMERD_SKIP_CLEAR [Default value]
//!
//! Modified registers are  \b TDxCTL0, \b TDxCTL1, \b TDxCCR0, \b TDxCCTL0
//!
//!This API does not start the timer. Timer needs to be started when required
//!using the TimerD_start API.
//!
//! \return None
//
//*****************************************************************************
void TimerD_configureUpDownMode (
    unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int clockingMode,
    unsigned int timerPeriod,
    unsigned int timerInterruptEnable_TDIE,
    unsigned int captureCompareInterruptEnable_CCR0_CCIE,
    unsigned int timerClear
    )
{
	
    ASSERT(
        (TIMERD_DO_CLEAR == timerClear) ||
        (TIMERD_SKIP_CLEAR == timerClear)
        );

    ASSERT(
        (TIMERD_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );


    ASSERT(
        (TIMERD_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );

    ASSERT(
        (TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_AUXILIARY_CLK == clockingMode)
        );

    HWREG(baseAddress + OFS_TDxCTL0) &=
        ~(TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK +
          TIMERD_UPDOWN_MODE +
          TIMERD_DO_CLEAR +
          TIMERD_TDIE_INTERRUPT_ENABLE
          );

    HWREG(baseAddress + OFS_TDxCTL1)  &= ~(TDCLKM0 + TDCLKM1);

    privateTimerDProcessClockSourceDivider(baseAddress,
    		        clockSourceDivider
    		        );
    HWREG(baseAddress + OFS_TDxCTL0)  |=  clockSource;
    HWREG(baseAddress + OFS_TDxCTL1) |= clockingMode;

    HWREG(baseAddress + OFS_TDxCTL0)  |= ( TIMERD_STOP_MODE +
                                          timerClear +
                                          timerInterruptEnable_TDIE
                                          );
    if (TIMERD_CCIE_CCR0_INTERRUPT_ENABLE ==
        captureCompareInterruptEnable_CCR0_CCIE){
        HWREG(baseAddress + OFS_TDxCCTL0)  |= TIMERD_CCIE_CCR0_INTERRUPT_ENABLE;
    } else   {
        HWREG(baseAddress + OFS_TDxCCTL0)  &= ~TIMERD_CCIE_CCR0_INTERRUPT_ENABLE;
    }

    HWREG(baseAddress + OFS_TDxCCR0)  = timerPeriod;
}


//*****************************************************************************
//
//! Initializes Capture Mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param captureMode is the capture mode selected. Valid values are
//!        \b TIMERD_CAPTUREMODE_NO_CAPTURE [Default value]
//!        \b TIMERD_CAPTUREMODE_RISING_EDGE
//!        \b TIMERD_CAPTUREMODE_FALLING_EDGE
//!        \b TIMERD_CAPTUREMODE_RISING_AND_FALLING_EDGE
//! \param captureInputSelect decides the Input Select
//!        \b TIMERD_CAPTURE_INPUTSELECT_CCIxA [Default value]
//!        \b TIMERD_CAPTURE_INPUTSELECT_CCIxB
//!        \b TIMERD_CAPTURE_INPUTSELECT_GND
//!        \b TIMERD_CAPTURE_INPUTSELECT_Vcc
//! \param synchronizeCaptureSource decides if capture source should be
//!         synchronized with timer clock
//!        Valid values are
//!        \b TIMERD_CAPTURE_ASYNCHRONOUS [Default value]
//!        \b TIMERD_CAPTURE_SYNCHRONOUS
//! \param captureInterruptEnable is to enable or disable
//!         timer captureComapre interrupt. Valid values are
//!        \b TIMERD_CAPTURECOMPARE_INTERRUPT_DISABLE [Default value]
//!        \b TIMERD_CAPTURECOMPARE_INTERRUPT_ENABLE
//! \param captureOutputMode specifies the output mode. Valid values are
//!        \b TIMERD_OUTPUTMODE_OUTBITVALUE [Default value],
//!        \b TIMERD_OUTPUTMODE_SET,
//!        \b TIMERD_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMERD_OUTPUTMODE_SET_RESET
//!        \b TIMERD_OUTPUTMODE_TOGGLE,
//!        \b TIMERD_OUTPUTMODE_RESET,
//!        \b TIMERD_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMERD_OUTPUTMODE_RESET_SET
//! \param channelCaptureMode specifies single/dual capture mode.
//!		Valid values are
//!        \b TIMERD_SINGLE_CAPTURE_MODE [Default value],
//!        \b TIMERD_DUAL_CAPTURE_MODE
//!
//! Modified registers are \b TDxCTL2, \b TDxCCTLn
//! \return None
//
//*****************************************************************************
void TimerD_initCapture (unsigned int baseAddress,
    unsigned int captureRegister,
    unsigned int captureMode,
    unsigned int captureInputSelect,
    unsigned short synchronizeCaptureSource,
    unsigned short captureInterruptEnable,
    unsigned int captureOutputMode,
    unsigned char channelCaptureMode
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == captureRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == captureRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == captureRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == captureRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == captureRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == captureRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == captureRegister) 
        );

    ASSERT((TIMERD_CAPTUREMODE_NO_CAPTURE == captureMode) ||
        (TIMERD_CAPTUREMODE_RISING_EDGE == captureMode) ||
        (TIMERD_CAPTUREMODE_FALLING_EDGE == captureMode) ||
        (TIMERD_CAPTUREMODE_RISING_AND_FALLING_EDGE == captureMode)
        );

    ASSERT((TIMERD_CAPTURE_INPUTSELECT_CCIxA == captureInputSelect) ||
        (TIMERD_CAPTURE_INPUTSELECT_CCIxB == captureInputSelect) ||
        (TIMERD_CAPTURE_INPUTSELECT_GND == captureInputSelect) ||
        (TIMERD_CAPTURE_INPUTSELECT_Vcc == captureInputSelect)
        );

    ASSERT((TIMERD_CAPTURE_ASYNCHRONOUS == synchronizeCaptureSource) ||
        (TIMERD_CAPTURE_SYNCHRONOUS == synchronizeCaptureSource)
        );

    ASSERT(
        (TIMERD_CAPTURECOMPARE_INTERRUPT_DISABLE == captureInterruptEnable) ||
        (TIMERD_CAPTURECOMPARE_INTERRUPT_ENABLE == captureInterruptEnable)
        );

    ASSERT((TIMERD_OUTPUTMODE_OUTBITVALUE == captureOutputMode) ||
        (TIMERD_OUTPUTMODE_SET == captureOutputMode) ||
        (TIMERD_OUTPUTMODE_TOGGLE_RESET == captureOutputMode) ||
        (TIMERD_OUTPUTMODE_SET_RESET == captureOutputMode) ||
        (TIMERD_OUTPUTMODE_TOGGLE == captureOutputMode) ||
        (TIMERD_OUTPUTMODE_RESET == captureOutputMode) ||
        (TIMERD_OUTPUTMODE_TOGGLE_SET == captureOutputMode) ||
        (TIMERD_OUTPUTMODE_RESET_SET == captureOutputMode)
        );

    ASSERT((TIMERD_SINGLE_CAPTURE_MODE == channelCaptureMode) ||
            (TIMERD_DUAL_CAPTURE_MODE == channelCaptureMode)
            );

    if (TIMERD_CAPTURECOMPARE_REGISTER_0 == captureRegister){
        //CaptureCompare register 0 only supports certain modes
        ASSERT((TIMERD_OUTPUTMODE_OUTBITVALUE == captureOutputMode) ||
            (TIMERD_OUTPUTMODE_SET == captureOutputMode) ||
            (TIMERD_OUTPUTMODE_TOGGLE == captureOutputMode) ||
            (TIMERD_OUTPUTMODE_RESET == captureOutputMode) 
            );
    }

    HWREG(baseAddress + captureRegister ) |=   CAP;

    HWREGB(baseAddress + OFS_TDxCTL2) |=
    		(channelCaptureMode << ((captureRegister - TIMERD_CAPTURECOMPARE_REGISTER_0)/6));

    HWREG(baseAddress + captureRegister) &=
        ~(TIMERD_CAPTUREMODE_RISING_AND_FALLING_EDGE +
          TIMERD_CAPTURE_INPUTSELECT_Vcc +
          TIMERD_CAPTURE_SYNCHRONOUS +
          TIMERD_DO_CLEAR +
          TIMERD_TDIE_INTERRUPT_ENABLE +
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
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareInterruptEnable is to enable or disable
//!         timer captureComapre interrupt. Valid values are
//!        \b TIMERD_CAPTURECOMPARE_INTERRUPT_ENABLE and
//!        \b TIMERD_CAPTURECOMPARE_INTERRUPT_DISABLE [Default value]
//! \param compareOutputMode specifies the ouput mode. Valid values are
//!        \b TIMERD_OUTPUTMODE_OUTBITVALUE [Default value],
//!        \b TIMERD_OUTPUTMODE_SET,
//!        \b TIMERD_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMERD_OUTPUTMODE_SET_RESET
//!        \b TIMERD_OUTPUTMODE_TOGGLE,
//!        \b TIMERD_OUTPUTMODE_RESET,
//!        \b TIMERD_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMERD_OUTPUTMODE_RESET_SET
//! \param compareValue is the count to be compared with in compare mode
//!
//! Modified register is \b TDxCCTLn and \b TDxCCRn
//! \return None
//
//*****************************************************************************
void TimerD_initCompare (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned short compareInterruptEnable,
    unsigned int compareOutputMode,
    unsigned int compareValue
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == compareRegister) 
        );

   ASSERT((TIMERD_CAPTURECOMPARE_INTERRUPT_ENABLE == compareInterruptEnable) ||
        (TIMERD_CAPTURECOMPARE_INTERRUPT_DISABLE == compareInterruptEnable)
        );

    ASSERT((TIMERD_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_SET == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_TOGGLE_RESET == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_SET_RESET == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_TOGGLE == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_RESET == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_TOGGLE_SET == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_RESET_SET == compareOutputMode)
        );

    if (TIMERD_CAPTURECOMPARE_REGISTER_0 == compareRegister){
        //CaptureCompare register 0 only supports certain modes
        ASSERT((TIMERD_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
            (TIMERD_OUTPUTMODE_SET == compareOutputMode) ||
            (TIMERD_OUTPUTMODE_TOGGLE == compareOutputMode) ||
            (TIMERD_OUTPUTMODE_RESET == compareOutputMode) 
            );
    }


    HWREG(baseAddress + compareRegister ) &=   ~CAP;

    HWREG(baseAddress + compareRegister) &=
        ~(TIMERD_CAPTURECOMPARE_INTERRUPT_ENABLE +
          TIMERD_OUTPUTMODE_RESET_SET
          );

    HWREG(baseAddress + compareRegister)  |= ( compareInterruptEnable +
                                               compareOutputMode
                                               );

    HWREG(baseAddress + compareRegister + 2) = compareValue;
}

//*****************************************************************************
//
//! Enable timer interrupt
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified register is TDxCTL0
//!
//! \return None
//
//*****************************************************************************
void TimerD_enableTimerInterrupt (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_TDxCTL0) &=  ~TDIFG;
    HWREGB(baseAddress + OFS_TDxCTL0) |= TDIE;
}

//*****************************************************************************
//
//! Enable High Resolution interrupt
//!
//! \param baseAddress is the base address of the Timer module.
//! \param mask is the mask for the interrupt status
//!         Valid values is an OR of
//!         \b TIMERD_HIGH_RES_FREQUENCY_UNLOCK,
//!         \b TIMERD_HIGH_RES_FREQUENCY_LOCK
//!         \b TIMERD_HIGH_RES_FAIL_HIGH,
//!         \b TIMERD_HIGH_RES_FAIL_LOW
//!
//! Modified register is TDxHINT
//!
//! \returns NONE
//
//*****************************************************************************
void TimerD_enableHighResInterrupt (unsigned int baseAddress,
				unsigned int mask)
{
    HWREG(baseAddress + OFS_TDxHINT) &=  ~(mask >> 8);
    HWREG(baseAddress + OFS_TDxHINT) |= mask;
}

//*****************************************************************************
//
//! Disable timer interrupt
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified register is \b TDxCTL0
//!
//! \return None
//
//*****************************************************************************
void TimerD_disableTimerInterrupt (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_TDxCTL0) &= ~TDIE;
}

//*****************************************************************************
//
//! Disable High Resolution interrupt
//!
//! \param baseAddress is the base address of the Timer module.
//! \param mask is the mask for the interrupt status
//!         Valid values is an OR of
//!         \b TIMERD_HIGH_RES_FREQUENCY_UNLOCK,
//!         \b TIMERD_HIGH_RES_FREQUENCY_LOCK
//!         \b TIMERD_HIGH_RES_FAIL_HIGH,
//!         \b TIMERD_HIGH_RES_FAIL_LOW
//!
//! Modified register is TDxHINT
//!
//! \returns NONE
//
//*****************************************************************************
void TimerD_disableHighResInterrupt (unsigned int baseAddress,
						unsigned int mask)
{
    HWREG(baseAddress + OFS_TDxHINT) &= ~mask;
}
//*****************************************************************************
//
//! Get timer interrupt status
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! \return unsigned char. Return interrupt status. Valid values are
//!         \b TIMERD_INTERRUPT_PENDING
//!         \b TIMERD_INTERRUPT_NOT_PENDING
//
//*****************************************************************************
unsigned long TimerD_getTimerInterruptStatus (unsigned int baseAddress)
{
    return ( HWREGB(baseAddress + OFS_TDxCTL0) & TDIFG );
}

//*****************************************************************************
//
//! Enable capture compare interrupt
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister is the selected capture compare regsiter
//!
//! Modified register is \b TDxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerD_enableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
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
//! Modified register is \b TDxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerD_disableCaptureCompareInterrupt (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
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
//!         Valid values is an OR of
//!         \b TIMERD_CAPTURE_OVERFLOW,
//!         \b TIMERD_CAPTURECOMPARE_INTERRUPT_FLAG
//!
//! \returns unsigned long. The mask of the set flags.
//! Modifed Registers None
//
//*****************************************************************************
unsigned long TimerD_getCaptureCompareInterruptStatus (unsigned int baseAddress,
	    unsigned int captureCompareRegister,
	    unsigned int mask
	    )
{
	return ( HWREG(baseAddress + captureCompareRegister) & mask );
}

//*****************************************************************************
//
//! Returns High Resolution interrupt status
//!
//! \param baseAddress is the base address of the Timer module.
//! \param mask is the mask for the interrupt status
//!         Valid values is an OR of
//!         \b TIMERD_HIGH_RES_FREQUENCY_UNLOCK,
//!         \b TIMERD_HIGH_RES_FREQUENCY_LOCK
//!         \b TIMERD_HIGH_RES_FAIL_HIGH,
//!         \b TIMERD_HIGH_RES_FAIL_LOW
//!
//! Modified register is \b TDxHINT
//!
//! \returns unsigned long. The mask of the set flags.
//
//*****************************************************************************
unsigned int TimerD_getHighResInterruptStatus (unsigned int baseAddress,
    unsigned int mask)
{
	mask = (mask >> 8);
    return ( (HWREG(baseAddress + OFS_TDxHINT) & mask) << 8 );
}
//*****************************************************************************
//
//! Reset/Clear the timer clock divider, count direction, count
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified register is \b TDxCTL0
//!
//! \returns None
//
//*****************************************************************************
void TimerD_clear (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TDxCTL0) |= TDCLR;
}
//*****************************************************************************
//
//! Clears High Resolution interrupt status
//!
//! \param baseAddress is the base address of the Timer module.
//! \param mask is the mask for the interrupt status
//!         Valid values is an OR of
//!         \b TIMERD_HIGH_RES_FREQUENCY_UNLOCK,
//!         \b TIMERD_HIGH_RES_FREQUENCY_LOCK
//!         \b TIMERD_HIGH_RES_FAIL_HIGH,
//!         \b TIMERD_HIGH_RES_FAIL_LOW
//!
//! Modified register is \b TDxHINT
//!
//! \returns none
//
//*****************************************************************************
void TimerD_clearHighResInterruptStatus (unsigned int baseAddress,
    unsigned int mask)
{
	mask = (mask >> 8);
	HWREG(baseAddress + OFS_TDxHINT) &= ~mask;
}

//*****************************************************************************
//
//! Get synchrnozied capturecompare input
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param synchronized is to select type of capture compare input.
//!         Valid values are
//!        \b TIMERD_READ_CAPTURE_COMPARE_INPUT
//!        \b TIMERD_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT
//!
//! \return \b TIMERD_CAPTURECOMPARE_INPUT_HIGH or
//!         \b TIMERD_CAPTURECOMPARE_INPUT_LOW
//! Modifed Registers None
//
//*****************************************************************************
unsigned short TimerD_getSynchronizedCaptureCompareInput
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned short synchronized
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister) 
        );

    ASSERT((TIMERD_READ_CAPTURE_COMPARE_INPUT == synchronized) ||
        (TIMERD_READ_SYNCHRONIZED_CAPTURECOMPAREINPUT == synchronized)
        );

    if (HWREG(baseAddress + captureCompareRegister) & synchronized){
        return ( TIMERD_CAPTURECOMPARE_INPUT_HIGH) ;
    } else   {
        return ( TIMERD_CAPTURECOMPARE_INPUT_LOW) ;
    }
}

//*****************************************************************************
//
//! Get ouput bit for output mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureRegister selects the Capture register being used. Valid values
//!     are
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! \return \b TIMERD_OUTPUTMODE_OUTBITVALUE_HIGH or
//!         \b TIMERD_OUTPUTMODE_OUTBITVALUE_LOW
//
//*****************************************************************************
unsigned char TimerD_getOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister) 
        );

    if (HWREG(baseAddress + captureCompareRegister) & OUT){
        return ( TIMERD_OUTPUTMODE_OUTBITVALUE_HIGH) ;
    } else   {
        return ( TIMERD_OUTPUTMODE_OUTBITVALUE_LOW) ;
    }
}

//*****************************************************************************
//
//! Get current capturecompare count
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister selects the Capture register being used.
//!	Valid values are
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! \return current count as unsigned int
//! Modifed Registers None
//
//*****************************************************************************
unsigned int TimerD_getCaptureCompareCount
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    return  (HWREG(baseAddress + captureCompareRegister + 2));
}

//*****************************************************************************
//
//! Get current capture compare latch register count
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister selects the Capture register being used.
//!	Valid values
//!     are
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! \return current count as unsigned int
//! Modifed Registers None
//
//*****************************************************************************
unsigned int TimerD_getCaptureCompareLatchCount
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    return  (HWREG(baseAddress + captureCompareRegister + 4));
}
//*****************************************************************************
//
//! Get current capturecompare input signal
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister selects the Capture register being used.
//! Valid values are
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! \return current input signal  as TIMERD_CAPTURECOMPARE_INPUT
//!		or 0x00
//! Modifed Registers None
//
//*****************************************************************************
unsigned char TimerD_getCaptureCompareInputSignal
    (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister)
        );

    return  ((HWREGB(baseAddress + captureCompareRegister) & CCI));
}
//*****************************************************************************
//
//! Set ouput bit for output mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister selects the Capture register being used.
//!	Valid values are
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param outputModeOutBitValueis the value to be set for out bit
//!     Valid values are \b TIMERD_OUTPUTMODE_OUTBITVALUE_HIGH
//!                      \b TIMERD_OUTPUTMODE_OUTBITVALUE_LOW
//!
//! Modified register is \b TDxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerD_setOutputForOutputModeOutBitValue
    (unsigned int baseAddress,
    unsigned int captureCompareRegister,
    unsigned char outputModeOutBitValue
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister) 
        );

    ASSERT((TIMERD_OUTPUTMODE_OUTBITVALUE_HIGH == outputModeOutBitValue) ||
        (TIMERD_OUTPUTMODE_OUTBITVALUE_LOW == outputModeOutBitValue)
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
//!         \b TIMERD_CLOCKSOURCE_EXTERNAL_TDCLK [Default value]
//!         \b TIMERD_CLOCKSOURCE_ACLK
//!         \b TIMERD_CLOCKSOURCE_SMCLK
//!         \b TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK
//! \param clockSourceDivider is the divider for Clock source.
//! 	Valid values are
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_64
//! \param clockingMode is the selected clock mode register values.
//! Valid values are
//!     \b TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK [Default value]
//!     \b TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK
//!	\b TIMERD_CLOCKINGMODE_AUXILIARY_CLK
//! \param timerPeriod selects the desired timer period
//! \param compareRegister selects the compare register being used. Valid values
//!     are
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareOutputMode specifies the ouput mode. Valid values are
//!        \b TIMERD_OUTPUTMODE_OUTBITVALUE,
//!        \b TIMERD_OUTPUTMODE_SET,
//!        \b TIMERD_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMERD_OUTPUTMODE_SET_RESET
//!        \b TIMERD_OUTPUTMODE_TOGGLE,
//!        \b TIMERD_OUTPUTMODE_RESET,
//!        \b TIMERD_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMERD_OUTPUTMODE_RESET_SET
//! \param dutyCycle specifies the dutycycle for the generated waveform
//!
//! Modified registers are \b TDxCTL0, \b TDxCTL1, \b TDxCCR0,
//!						   \b TDxCCTL0,\b TDxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerD_generatePWM (  unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int clockingMode,
    unsigned int timerPeriod,
    unsigned int compareRegister,
    unsigned int compareOutputMode,
    unsigned int dutyCycle
    )
{
    
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == compareRegister) 
        );

    ASSERT(
        (TIMERD_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );


    ASSERT(
        (TIMERD_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );

    ASSERT(
        (TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_AUXILIARY_CLK == clockingMode)
        );
    
    ASSERT((TIMERD_OUTPUTMODE_OUTBITVALUE == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_SET == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_TOGGLE_RESET == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_SET_RESET == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_TOGGLE == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_RESET == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_TOGGLE_SET == compareOutputMode) ||
        (TIMERD_OUTPUTMODE_RESET_SET == compareOutputMode)
        );

    HWREG(baseAddress + OFS_TDxCTL1)  &= ~(TDCLKM0 + TDCLKM1);

    HWREG(baseAddress + OFS_TDxCTL0)  &=
            ~( TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK +
               TIMERD_UPDOWN_MODE + TIMERD_DO_CLEAR +
               TIMERD_TDIE_INTERRUPT_ENABLE
               );


    privateTimerDProcessClockSourceDivider(baseAddress,
    		        clockSourceDivider
    		        );
    HWREG(baseAddress + OFS_TDxCTL0)  |=  clockSource;
    HWREG(baseAddress + OFS_TDxCTL1) |= clockingMode;

    HWREG(baseAddress + OFS_TDxCTL0)  |= ( TIMERD_UP_MODE +
                                          TIMERD_DO_CLEAR
                                          );

    HWREG(baseAddress + OFS_TDxCCR0)  = timerPeriod;

    HWREG(baseAddress + OFS_TDxCCTL0)  &=
        ~(TIMERD_CAPTURECOMPARE_INTERRUPT_ENABLE +
          TIMERD_OUTPUTMODE_RESET_SET
          );
    HWREG(baseAddress + compareRegister)  |= compareOutputMode;

    HWREG(baseAddress + compareRegister + 2) = dutyCycle;
}

//*****************************************************************************
//
//! Stops the timer
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified registers are \b TDxCTL0
//!
//! \returns None
//
//*****************************************************************************
void TimerD_stop ( unsigned int baseAddress )
{
    HWREG(baseAddress + OFS_TDxCTL0)  &= ~MC_3;
    HWREG(baseAddress + OFS_TDxCTL0)  |= MC_0;
}

//*****************************************************************************
//
//! Private clock source divider helper function
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSourceDivider is the desired divider for the clock source
//!
//! Modified registers are TDxCTL1, TDxCTL0
//!
//! \returns None
//
//*****************************************************************************
void privateTimerDProcessClockSourceDivider (unsigned int baseAddress,
    unsigned int clockSourceDivider)
{
    HWREG(baseAddress + OFS_TDxCTL0) &= ~ID__8;
    HWREG(baseAddress + OFS_TDxCTL1) &= ~TDIDEX_7;
    switch (clockSourceDivider){
        case TIMERD_CLOCKSOURCE_DIVIDER_1:
        case TIMERD_CLOCKSOURCE_DIVIDER_2:
        case TIMERD_CLOCKSOURCE_DIVIDER_4:
        case TIMERD_CLOCKSOURCE_DIVIDER_8:
            HWREG(baseAddress + OFS_TDxCTL0) |= ((clockSourceDivider - 1) << 6);
            HWREG(baseAddress + OFS_TDxCTL1) = TDIDEX_0;
            break;

        case TIMERD_CLOCKSOURCE_DIVIDER_3:
        case TIMERD_CLOCKSOURCE_DIVIDER_5:
        case TIMERD_CLOCKSOURCE_DIVIDER_6:
        case TIMERD_CLOCKSOURCE_DIVIDER_7:
            HWREG(baseAddress + OFS_TDxCTL0) |= ID__1;
            HWREG(baseAddress + OFS_TDxCTL1) = (clockSourceDivider - 1);
            break;

        case TIMERD_CLOCKSOURCE_DIVIDER_10:
        case TIMERD_CLOCKSOURCE_DIVIDER_12:
        case TIMERD_CLOCKSOURCE_DIVIDER_14:
        case TIMERD_CLOCKSOURCE_DIVIDER_16:
            HWREG(baseAddress + OFS_TDxCTL0) |= ID__2;
            HWREG(baseAddress + OFS_TDxCTL1) = (clockSourceDivider / 2 - 1 );
            break;

        case TIMERD_CLOCKSOURCE_DIVIDER_20:
        case TIMERD_CLOCKSOURCE_DIVIDER_24:
        case TIMERD_CLOCKSOURCE_DIVIDER_28:
        case TIMERD_CLOCKSOURCE_DIVIDER_32:
            HWREG(baseAddress + OFS_TDxCTL0) |= ID__4;
            HWREG(baseAddress + OFS_TDxCTL1) = (clockSourceDivider / 4 - 1);
            break;
        case TIMERD_CLOCKSOURCE_DIVIDER_40:
        case TIMERD_CLOCKSOURCE_DIVIDER_48:
        case TIMERD_CLOCKSOURCE_DIVIDER_56:
        case TIMERD_CLOCKSOURCE_DIVIDER_64:
            HWREG(baseAddress + OFS_TDxCTL0) |= ID__8;
            HWREG(baseAddress + OFS_TDxCTL1) = (clockSourceDivider / 8 - 1);
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
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//! \param compareValue is the count to be compared with in compare mode
//!
//! Modified register is \b TDxCCRn
//!
//! \return None
//
//*****************************************************************************
void TimerD_setCompareValue (  unsigned int baseAddress,
    unsigned int compareRegister,
    unsigned int compareValue
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == compareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == compareRegister) 
        );

    HWREG(baseAddress + compareRegister + 0x02) = compareValue;
}

//*****************************************************************************
//
//! Clears the Timer TAIFG interrupt flag
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified bits are TAIFG og TDxCTL0 register
//!
//! \return None
//
//*****************************************************************************
void TimerD_clearTimerInterruptFlag (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_TDxCTL0) &= ~TDIFG;
}

//*****************************************************************************
//
//! Clears the capture-compare interrupt flag
//!
//! \param baseAddress is the base address of the Timer module.
//! \param captureCompareRegister selects the Capture-compare register being
//! used. Valid values are
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//!    Refer datasheet to ensure the device has the capture compare register
//!    being used
//!
//! Modified bits are CCIFG of \b TDxCCTLn register
//!
//! \return None
//
//*****************************************************************************
void TimerD_clearCaptureCompareInterruptFlag (unsigned int baseAddress,
    unsigned int captureCompareRegister
    )
{
    ASSERT((TIMERD_CAPTURECOMPARE_REGISTER_0 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_1 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_2 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_3 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_4 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_5 == captureCompareRegister) ||
        (TIMERD_CAPTURECOMPARE_REGISTER_6 == captureCompareRegister) 
        );

    HWREG(baseAddress + captureCompareRegister)  &= ~CCIFG;
}

//*****************************************************************************
//
//! Configures Timer_D in free running mode
//!
//! \param baseAddress is the base address of the Timer module.
//! \param desiredHighResFrequency selects the desired High 
//! Resolution frequency
//! used. Valid values are
//!     \b TIMERD_HIGHRES_64MHZ
//!     \b TIMERD_HIGHRES_128MHZ
//!     \b TIMERD_HIGHRES_200MHZ
//!     \b TIMERD_HIGHRES_256MHZ
//!
//! Modified registers are \b TDxHCTL1, \b TDxCTL1 and  TDxHCTL0 register
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//*****************************************************************************
unsigned char TimerD_configureHighResGeneratorInFreeRunningMode 
	(unsigned int baseAddress,
    unsigned char desiredHighResFrequency
    )
{
	struct s_TLV_Timer_D_Cal_Data * pTD0CAL; 
  	unsigned char TD0CAL_bytes;   
  	
  	ASSERT((TIMERD_HIGHRES_64MHZ == desiredHighResFrequency) ||
        (TIMERD_HIGHRES_128MHZ == desiredHighResFrequency) ||
        (TIMERD_HIGHRES_200MHZ == desiredHighResFrequency) ||
        (TIMERD_HIGHRES_256MHZ == desiredHighResFrequency)        
        );
  	
  	// Read the TimerD TLV Data
	TLV_getInfo(TLV_TIMER_D_CAL, 
              0, 
              &TD0CAL_bytes, 
              (unsigned int **)&pTD0CAL
              );
              
	if(0x00 == TD0CAL_bytes)
  	{
    	// No TimerD free running cal data found
      	return STATUS_FAIL;
  	}  
  	
	HWREG(baseAddress + OFS_TDxHCTL1) = TDHCLKTRIM6;
  	HWREG(baseAddress + OFS_TDxCTL1) = 0x00;
  	HWREG(baseAddress + OFS_TDxHCTL0) = 0x00;

  	switch( desiredHighResFrequency )
  	{
  		case TIMERD_HIGHRES_64MHZ:
  			HWREG(baseAddress + OFS_TDxHCTL1) = pTD0CAL->TDH0CTL1_64; 
  			break;
  			
  		case TIMERD_HIGHRES_128MHZ:
  			HWREG(baseAddress + OFS_TDxHCTL1) = pTD0CAL->TDH0CTL1_128; 
  			break;
  			
  		case TIMERD_HIGHRES_200MHZ:
  			HWREG(baseAddress + OFS_TDxHCTL1) = pTD0CAL->TDH0CTL1_200;
  			break;
  			 
  		case TIMERD_HIGHRES_256MHZ:
  			HWREG(baseAddress + OFS_TDxHCTL1) = pTD0CAL->TDH0CTL1_256;
  			break; 
  	}
  	


	// Select Hi-res local clock
  	HWREG(baseAddress + OFS_TDxCTL1) |= TDCLKM_1;
  	        
    // CALEN=0 => free running mode; enable Hi-res mode
 	if(TIMERD_HIGHRES_256MHZ == desiredHighResFrequency)
  		HWREG(baseAddress + OFS_TDxHCTL0) |= TDHM_1;
  	
  	HWREG(baseAddress + OFS_TDxHCTL0) |= TDHEN; 
  							
  	   
  	return STATUS_SUCCESS;
    
}

//*****************************************************************************
//
//! Configures Timer_D in Regulated mode
//!
//! \param baseAddress is the base address of the Timer module.
///! \param clockSource selects Clock source. Valid values are
//!         \b TIMERD_CLOCKSOURCE_EXTERNAL_TDCLK [Default value]
//!         \b TIMERD_CLOCKSOURCE_ACLK
//!         \b TIMERD_CLOCKSOURCE_SMCLK
//!         \b TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK
//! \param clockSourceDivider is the divider for Clock source.
//! 	Valid values are
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_64
//! \param clockingMode is the selected clock mode register values.
//! Valid values are
//!     \b TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK [Default value]
//!     \b TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK
//!	\b TIMERD_CLOCKINGMODE_AUXILIARY_CLK
//! \param highResClockMultiplyFactor selects the high resolution
//! multiply factor.
//!     \b TIMERD_HIGHRES_CLK_MULTIPLY_FACTOR_8x
//!     \b TIMERD_HIGHRES_CLK_MULTIPLY_FACTOR_16x
//! \param highResClockDivider selects the high resolution
//! divider.
//!     \b TIMERD_HIGHRES_CLK_DIVIDER_1 [Default value]
//!     \b TIMERD_HIGHRES_CLK_DIVIDER_2
//!     \b TIMERD_HIGHRES_CLK_DIVIDER_4
//!     \b TIMERD_HIGHRES_CLK_DIVIDER_8
//!
//! Modified registers are \b OFS_TDxCTL0, \b TDxCTL1 and  TDxHCTL0 register
//!
//! \return NONE
//
//*****************************************************************************
void TimerD_configureHighResGeneratorInRegulatedMode (unsigned int baseAddress,
     unsigned int clockSource, 
     unsigned int clockSourceDivider,
     unsigned int clockingMode, 
     unsigned char highResClockMultiplyFactor,
     unsigned char highResClockDivider
    )
{	
	ASSERT(
        (TIMERD_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );


    ASSERT(
        (TIMERD_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );

    ASSERT(
        (TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_AUXILIARY_CLK == clockingMode)
        );
    
	ASSERT((TIMERD_8x == highResMultiplyFactor) ||
        (TIMERD_16x == highResMultiplyFactor)
        );

	ASSERT((TIMERD_HIGHRES_CLK_DIVIDER_1 == highResClockDivider) ||
	        (TIMERD_HIGHRES_CLK_DIVIDER_2 == highResClockDivider) ||
	        (TIMERD_HIGHRES_CLK_DIVIDER_4 == highResClockDivider) ||
	        (TIMERD_HIGHRES_CLK_DIVIDER_8 == highResClockDivider)
	        );

	/**********how abt MCx and TDCLGRPx and CNTLx*/
	HWREG(baseAddress + OFS_TDxCTL0) &= ~(TDSSEL_3 + TDHD_3 + TDCLR);  
        
	HWREG(baseAddress + OFS_TDxCTL1)  &= ~(TDCLKM0 + TDCLKM1);
	
        privateTimerDProcessClockSourceDivider(baseAddress,
    		        clockSourceDivider
    		        );
        HWREG(baseAddress + OFS_TDxCTL0)  |=  clockSource;
        HWREG(baseAddress + OFS_TDxCTL1) |= clockingMode;

  	// Select Hi-res local clock
  	// Calibration and Hi-res mode enable
  	HWREG(baseAddress + OFS_TDxCTL1) |= TDCLKM_1;                      
  	// Select Hi-res local clock
  	HWREG(baseAddress + OFS_TDxHCTL0) =  TDHREGEN + TDHEN ;
  	HWREG(baseAddress + OFS_TDxHCTL0) |= highResClockMultiplyFactor +
  					highResClockDivider;
 
}
//*****************************************************************************
//
//! Combine TDCCRto get PWM
//!
//! \param baseAddress is the base address of the Timer module.
//! \param clockSource selects Clock source. Valid values are
//!         \b TIMERD_CLOCKSOURCE_EXTERNAL_TDCLK [Default value]
//!         \b TIMERD_CLOCKSOURCE_ACLK
//!         \b TIMERD_CLOCKSOURCE_SMCLK
//!         \b TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TDCLK
//! \param clockSourceDivider is the divider for Clock source.
//! 	Valid values are
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_1 [Default value]
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_2
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_4
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_8
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_3
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_5
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_6
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_7
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_10
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_12
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_14
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_16
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_20
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_24
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_28
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_32
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_40
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_48
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_56
//!        \b TIMERD_CLOCKSOURCE_DIVIDER_64
//! \param clockingMode is the selected clock mode register values.
//! Valid values are
//!     \b TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK [Default value]
//!     \b TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK
//!	\b TIMERD_CLOCKINGMODE_AUXILIARY_CLK
//!//! \param timerPeriod selects the desired timer period
//! \param combineCCRRegistersCombination selects desired CCR registers to
//!			combine
//!			\b TIMERD_COMBINE_CCR1_CCR2
//!			\b TIMERD_COMBINE_CCR3_CCR4 (available on Timer_D5, Timer_D7)
//!			\b TIMERD_COMBINE_CCR5_CCR6(available only on Timer_D7)
//! \param compareOutputMode specifies the ouput mode. Valid values are
//!        \b TIMERD_OUTPUTMODE_OUTBITVALUE,
//!        \b TIMERD_OUTPUTMODE_SET,
//!        \b TIMERD_OUTPUTMODE_TOGGLE_RESET,
//!        \b TIMERD_OUTPUTMODE_SET_RESET
//!        \b TIMERD_OUTPUTMODE_TOGGLE,
//!        \b TIMERD_OUTPUTMODE_RESET,
//!        \b TIMERD_OUTPUTMODE_TOGGLE_SET,
//!        \b TIMERD_OUTPUTMODE_RESET_SET
//! \param dutyCycle specifies the dutycycle for the generated waveform
//!
//! Modified registers are \b TDxCTL0, \b TDxCTL1, \b TDxCCR0,
//!						   \b TDxCCTL0,\b TDxCCTLn
//!
//! \return None
//
//*****************************************************************************
void TimerD_combineTDCCRToGeneratePWM (  unsigned int baseAddress,
    unsigned int clockSource,
    unsigned int clockSourceDivider,
    unsigned int clockingMode,
    unsigned int timerPeriod,
    unsigned int combineCCRRegistersCombination,
    unsigned int compareOutputMode,
    unsigned int dutyCycle1,
    unsigned int dutyCycle2
    )
{
	ASSERT(
                (TIMERD_COMBINE_CCR1_CCR2 == combineCCRRegistersCombination) ||
                (TIMERD_COMBINE_CCR3_CCR4 == combineCCRRegistersCombination) ||
                (TIMERD_COMBINE_CCR5_CCR6 == combineCCRRegistersCombination)
                );

	ASSERT(
        (TIMERD_CLOCKSOURCE_EXTERNAL_TXCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_ACLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_SMCLK == clockSource) ||
        (TIMERD_CLOCKSOURCE_INVERTED_EXTERNAL_TXCLK == clockSource)
        );


    ASSERT(
        (TIMERD_CLOCKSOURCE_DIVIDER_1 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_2 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_4 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_8 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_3 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_5 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_6 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_7 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_10 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_12 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_14 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_16 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_20 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_24 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_28 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_32 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_40 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_48 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_56 == clockSourceDivider) ||
        (TIMERD_CLOCKSOURCE_DIVIDER_64 == clockSourceDivider)
        );

    ASSERT(
        (TIMERD_CLOCKINGMODE_EXTERNAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_HIRES_LOCAL_CLOCK == clockingMode) ||
        (TIMERD_CLOCKINGMODE_AUXILIARY_CLK == clockingMode)
        );

	ASSERT((TIMERD_OUTPUTMODE_OUTBITVALUE == captureOutputMode) ||
	        (TIMERD_OUTPUTMODE_SET == captureOutputMode) ||
	        (TIMERD_OUTPUTMODE_TOGGLE_RESET == captureOutputMode) ||
	        (TIMERD_OUTPUTMODE_SET_RESET == captureOutputMode) ||
	        (TIMERD_OUTPUTMODE_TOGGLE == captureOutputMode) ||
	        (TIMERD_OUTPUTMODE_RESET == captureOutputMode) ||
	        (TIMERD_OUTPUTMODE_TOGGLE_SET == captureOutputMode) ||
	        (TIMERD_OUTPUTMODE_RESET_SET == captureOutputMode)
	        );


	HWREG(baseAddress + OFS_TDxCCTL2) &= ~OUTMOD_7;
	HWREG(baseAddress + OFS_TDxCCTL2)  |= compareOutputMode;

    HWREG(baseAddress + OFS_TDxCCR0)  = timerPeriod;
    
    HWREG(baseAddress + OFS_TDxCCR1 + (0x05 *
    		(combineCCRRegistersCombination - TIMERD_COMBINE_CCR1_CCR2))) = dutyCycle1;
    HWREG(baseAddress + OFS_TDxCCR2 + (0x05 *
    		(combineCCRRegistersCombination - TIMERD_COMBINE_CCR1_CCR2))) = dutyCycle2;
    
    HWREG(baseAddress + OFS_TDxCTL1)  &= ~(TDCLKM0 + TDCLKM1);

    privateTimerDProcessClockSourceDivider(baseAddress,
    		        clockSourceDivider
    		        );
    
    HWREG(baseAddress + OFS_TDxCTL0)  |=  clockSource;
    HWREG(baseAddress + OFS_TDxCTL1) |= clockingMode;
    HWREG(baseAddress + OFS_TDxCTL1)  |=
    		(TD2CMB << (combineCCRRegistersCombination - TIMERD_COMBINE_CCR1_CCR2));
}

//*****************************************************************************
//
//! Selects TimerD Latching Group
//!
//! \param baseAddress is the base address of the TimerD module.
//! \param groupLatch selects the value of counter length.
//! Valid values are
//!     \b TIMERD_GROUP_NONE [Default value]
//!     \b TIMERD_GROUP_CL12_CL23_CL56
//!     \b TIMERD_GROUP_CL123_CL456
//!     \b TIMERD_GROUP_ALL
//!
//! Modified bits are TDCLGRP of \b TDxCTL0 register
//!
//! \return None
//
//*****************************************************************************
void TimerD_selectLatchingGroup(unsigned int  baseAddress,
		unsigned int  groupLatch)
{
	ASSERT((TIMERD_GROUP_NONE  == groupLatch) ||
		   (TIMERD_GROUP_CL12_CL23_CL56 == groupLatch) ||
		   (TIMERD_GROUP_CL123_CL456 == groupLatch) ||
		   (TIMERD_GROUP_ALL == groupLatch)
		   );


	HWREG(baseAddress + OFS_TDxCTL0) &= ~TDCLGRP_3;
	HWREG(baseAddress + OFS_TDxCTL0) |= groupLatch;
}

//*****************************************************************************
//
//! Selects TimerD counter length
//!
//! \param baseAddress is the base address of the TimerD module.
//! \param counterLength selects the value of counter length.
//! Valid values are
//!     \b TIMERD_COUNTER_16BIT [Default value]
//!     \b TIMERD_COUNTER_12BIT
//!     \b TIMERD_COUNTER_10BIT
//!     \b TIMERD_COUNTER_8BIT
//!
//! Modified bits are CNTL of \b TDxCTL0 register
//!
//! \return None
//
//*****************************************************************************
void TimerD_selectCounterLength (unsigned int  baseAddress,
		unsigned int counterLength
		)
{
	ASSERT((TIMERD_COUNTER_8BIT == counterLength) ||
	        (TIMERD_COUNTER_10BIT == counterLength) ||
	        (TIMERD_COUNTER_12BIT == counterLength) ||
	        (TIMERD_COUNTER_16BIT == counterLength)
	        );


	HWREG(baseAddress + OFS_TDxCTL0) &= ~CNTL_3;
	HWREG(baseAddress + OFS_TDxCTL0) |= counterLength;
}

//*****************************************************************************
//
//! Selects Compare Latch Load Event
//!
//! \param baseAddress is the base address of the TimerD module.
//! \param captureCompareRegister selects the Capture-compare register being
//! used. Valid values are
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_0
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_1
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_2
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_3
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_4
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_5
//!     \b TIMERD_CAPTURECOMPARE_REGISTER_6
//! \param compareLatchLoadEvent selects the latch load event
//! Valid values are
//!     \b TIMERD_LATCH_ON_WRITE_TO_TDxCCRn_COMPARE_REGISTER [Default value]
//!     \b TIMERD_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UP_OR_CONT_MODE
//!     \b TIMERD_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UPDOWN_MODE
//!     \b TIMERD_LATCH_WHEN_COUNTER_COUNTS_TO_CURRENT_COMPARE_LATCH_VALUE
//!
//! Modified bits are CLLD of \b TDxCCTLn register
//!
//! \return None
//
//*****************************************************************************
void TimerD_initCompareLatchLoadEvent(unsigned int  baseAddress,
		unsigned int  compareRegister,
		unsigned int  compareLatchLoadEvent
		)
{
	ASSERT((TIMERD_LATCH_ON_WRITE_TO_TBxCCRn_COMPARE_REGISTER  == groupLatch) ||
		(TIMERD_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UP_OR_CONT_MODE == groupLatch) ||
		(TIMERD_LATCH_WHEN_COUNTER_COUNTS_TO_0_IN_UPDOWN_MODE == groupLatch) ||
		(TIMERD_LATCH_WHEN_COUNTER_COUNTS_TO_CURRENT_COMPARE_LATCH_VALUE
				== groupLatch)
		);

	HWREG(baseAddress + compareRegister)  &= ~CLLD_3;
	HWREG(baseAddress + compareRegister)  |= compareLatchLoadEvent;
}

//*****************************************************************************
//
//! Disable High Resolution fast wakeup
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified bits are TDHFW of TDxHCTL0 register.
//!
//! \returns NONE
//
//*****************************************************************************
void TimerD_disableHighResFastWakeup (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TDxHCTL0) &= ~TDHFW;
}

//*****************************************************************************
//
//! Enable High Resolution fast wakeup
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified bits are TDHFW of TDxHCTL0 register.
//!
//! \returns NONE
//
//*****************************************************************************
void TimerD_enableHighResFastWakeup (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TDxHCTL0) |= TDHFW;
}

//*****************************************************************************
//
//! Disable High Resolution Clock Enhanced Accuracy
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified bits are TDHEAEN of TDxHCTL0 register.
//!
//! \returns NONE
//
//*****************************************************************************
void TimerD_disableHighResClockEnhancedAccuracy (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TDxHCTL0) &= ~TDHEAEN;
}

//*****************************************************************************
//
//! Enable High Resolution Clock Enhanced Accuracy
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified bits are TDHEAEN of TDxHCTL0 register.
//!
//! \returns NONE
//
//*****************************************************************************
void TimerD_enableHighResClockEnhancedAccuracy (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TDxHCTL0) |= TDHEAEN;
}

//*****************************************************************************
//
//! Disable High Resolution Clock Enhanced Accuracy
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified bit is TDHRON of register TDxHCTL0
//!
//!High-resolution generator is on if the Timer_D counter
//!MCx bits are 01, 10 or 11.
//! \returns NONE
//
//*****************************************************************************
void TimerD_DisableHighResGeneratorForceON (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TDxHCTL0) &= ~TDHRON;
}

//*****************************************************************************
//
//! Enable High Resolution Clock Enhanced Accuracy
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! Modified bit is TDHRON of register TDxHCTL0
//!
//!High-resolution generator is on in all Timer_D MCx modes. The PMM
//!remains in high-current mode.
//! \returns NONE
//
//*****************************************************************************
void TimerD_EnableHighResGeneratorForceON (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_TDxHCTL0) |= TDHRON;
}

//*****************************************************************************
//
//! Select High Resolution Coarse Clock Range
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! \param highResCoarseClockRange selects the High Resolution Coarse
//! Clock Range
//! Valid values are
//!     \b TIMERD_HIGHRES_BELOW_15MHz [Default value]
//!     \b TIMERD_HIGHRES_ABOVE_15MHz
//!
//! Modified bits are TDHCLKCR of registers TDxHCTL1.
//!
//! \returns NONE
//
//*****************************************************************************
void TimerD_selectHighResCoarseClockRange (unsigned int baseAddress,
		unsigned int highResCoarseClockRange
		)
{
	ASSERT((TIMERD_HIGHRES_BELOW_15MHz  == highResCoarseClockRange) ||
		(TIMERD_HIGHRES_ABOVE_15MHz == highResCoarseClockRange)
		);
	HWREG(baseAddress + OFS_TDxHCTL1) &= ~TDHCLKCR;
    HWREG(baseAddress + OFS_TDxHCTL1) |= highResCoarseClockRange;
}

//*****************************************************************************
//
//! Select High Resolution Clock Range Selection
//!
//! \param baseAddress is the base address of the Timer module.
//!
//! \param highResClockRange selects the High Resolution
//! Clock Range
//! Valid values are
//!     \b TIMERD_CLOCK_RANGE0 [Default value]
//!     \b TIMERD_CLOCK_RANGE1
//!     \b TIMERD_CLOCK_RANGE2
//!Refer Datasheet for frequency details
//!
//!Modified bits are TDHCLKRx of registers TDxHCTL1
//! NOTE: In Regulated mode these bits are modified by hardware.
//!
//! \returns NONE
//
//*****************************************************************************
void TimerD_selectHighResClockRange (unsigned int baseAddress,
		unsigned int highResClockRange
		)
{
	ASSERT((TIMERD_CLOCK_RANGE0  == highResClockRange) ||
		(TIMERD_CLOCK_RANGE1 == highResClockRange) 	||
		(TIMERD_CLOCK_RANGE2 == highResClockRange)
		);
	HWREG(baseAddress + OFS_TDxHCTL1) &= ~TDHCLKCR;
    HWREG(baseAddress + OFS_TDxHCTL1) |= highResClockRange;
}


//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************

