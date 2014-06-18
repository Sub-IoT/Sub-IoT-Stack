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
//wdt.c - Driver for the WDT Module.
//
//*****************************************************************************
#include "wdt.h"
#include "../inc/hw_types.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "../deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif
#include "debug.h"

//*****************************************************************************
//
//! Holds the Watchdog Timer.
//!
//! \param baseAddress is the base address of the WDT module.
//!
//! This function stops the watchdog timer from running, that way no interrupt
//! or PUC is asserted.
//!
//! \return None
//
//*****************************************************************************
void WDT_hold (unsigned int baseAddress)
{
    //Set Hold bit
    unsigned char newWDTStatus = ( HWREGB(baseAddress + OFS_WDTCTL_L) | WDTHOLD );

    HWREG(baseAddress + OFS_WDTCTL) = WDTPW + newWDTStatus;
}

//*****************************************************************************
//
//! Starts the Watchdog Timer.
//!
//! \param baseAddress is the base address of the WDT module.
//!
//! This function starts the watchdog timer functionality to start counting
//! again.
//!
//! \return NONE
//
//*****************************************************************************
void WDT_start (unsigned int baseAddress)
{
    //Reset Hold bit
    unsigned char newWDTStatus =
        ( HWREGB(baseAddress + OFS_WDTCTL_L) & ~(WDTHOLD) );

    HWREG(baseAddress + OFS_WDTCTL) = WDTPW + newWDTStatus;
}

//*****************************************************************************
//
//! Clears the timer counter of the Watchdog Timer.
//!
//! \param baseAddress is the base address of the WDT module.
//!
//! This function clears the watchdog timer to 0x0000h.
//!
//! \return None
//
//*****************************************************************************
void WDT_resetTimer (unsigned int baseAddress)
{
    //Set Counter Clear bit
    unsigned char newWDTStatus =
        ( HWREGB(baseAddress + OFS_WDTCTL_L) | WDTCNTCL );

    HWREG(baseAddress + OFS_WDTCTL) = WDTPW + newWDTStatus;
}

//*****************************************************************************
//
//! Sets the clock source for the Watchdog Timer in timer interval mode.
//!
//! \param baseAddress is the base address of the WDT module.
//! \param clockSelect is the clock source that the watchdog timer will use.
//!        Valid values are
//!        \b WDT_CLOCKSOURCE_SMCLK [Default]
//!        \b WDT_CLOCKSOURCE_ACLK
//!        \b WDT_CLOCKSOURCE_VLOCLK
//!        \b WDT_CLOCKSOURCE_XCLK
//!        Modified bits are \b WDTSSEL of \b WDTCTL register.
//! \param clockDivider is the divider of the clock source, in turn setting the
//!       watchdog timer interval.
//!        Valid values are
//!        \b WDT_CLOCKDIVIDER_2G
//!        \b WDT_CLOCKDIVIDER_128M
//!        \b WDT_CLOCKDIVIDER_8192K
//!        \b WDT_CLOCKDIVIDER_512K
//!        \b WDT_CLOCKDIVIDER_32K [Default]
//!        \b WDT_CLOCKDIVIDER_8192
//!        \b WDT_CLOCKDIVIDER_512
//!        \b WDT_CLOCKDIVIDER_64
//!        Modifed bits are \b WDTIS of \b WDTCTL register.
//!
//! This function sets the watchdog timer as timer interval mode, which will
//! assert an interrupt without causing a PUC.
//!
//! \return None
//
//*****************************************************************************
void WDT_watchdogTimerInit (unsigned int baseAddress,
    unsigned char clockSelect,
    unsigned char clockDivider)
{
    HWREG(baseAddress + OFS_WDTCTL) =
        WDTPW + WDTCNTCL + clockSelect + clockDivider;
}

//*****************************************************************************
//
//! Sets the clock source for the Watchdog Timer in watchdog mode.
//!
//! \param baseAddress is the base address of the WDT module.
//! \param clockSelect is the clock source that the watchdog timer will use.
//!        Valid values are
//!        \b WDT_CLOCKSOURCE_SMCLK [Default]
//!        \b WDT_CLOCKSOURCE_ACLK
//!        \b WDT_CLOCKSOURCE_VLOCLK
//!        \b WDT_CLOCKSOURCE_XCLK
//!        Modified bits are \b WDTSSEL of \b WDTCTL register.
//! \param clockDivider is the divider of the clock source, in turn setting the
//!       watchdog timer interval.
//!        Valid values are
//!        \b WDT_CLOCKDIVIDER_2G
//!        \b WDT_CLOCKDIVIDER_128M
//!        \b WDT_CLOCKDIVIDER_8192K
//!        \b WDT_CLOCKDIVIDER_512K
//!        \b WDT_CLOCKDIVIDER_32K [Default]
//!        \b WDT_CLOCKDIVIDER_8192
//!        \b WDT_CLOCKDIVIDER_512
//!        \b WDT_CLOCKDIVIDER_64
//!        Modifed bits are \b WDTIS of \b WDTCTL register.
//!
//! This function sets the watchdog timer in watchdog mode, which will cause a
//! PUC when the timer overflows. When in the mode, a PUC can be avoided with a
//! call to WDT_resetTimer() before the timer runs out.
//!
//! \return None
//
//*****************************************************************************
void WDT_intervalTimerInit (unsigned int baseAddress,
    unsigned char clockSelect,
    unsigned char clockDivider)
{
    HWREG(baseAddress + OFS_WDTCTL) =
        WDTPW + WDTCNTCL + WDTTMSEL + clockSelect + clockDivider;
}

//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************

