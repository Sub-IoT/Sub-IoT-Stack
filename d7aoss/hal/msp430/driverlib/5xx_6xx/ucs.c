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
//******************************************************************************
//
//ucs.c - Driver for the UCS Module.
//
//******************************************************************************

#include "../inc/hw_types.h"
#include "ucs.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "../deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif
#include "debug.h"
#include "../inc/sfr_sys_baseAddress.h"
#ifdef __GNUC__
#define __get_SR_register() READ_SR
#endif

//******************************************************************************
//
//The XT1 crystal frequency. Should be set with UCS_externalClockSourceInit
//if XT1 is used and user intends to invoke UCS_getSMCLK, UCS_getMCLK or
//UCS_getACLK
//
//******************************************************************************
unsigned long UCS_XT1ClockFrequency = 0;

//******************************************************************************
//
//The XT2 crystal frequency. Should be set with UCS_externalClockSourceInit
//if XT1 is used and user intends to invoke UCS_getSMCLK, UCS_getMCLK or
//UCS_getACLK
//
//******************************************************************************
unsigned long UCS_XT2ClockFrequency = 0;

//******************************************************************************
//
//! This function sets the external clock sources XT1 and XT2 crystal
//! oscillator frequency values. This function must be called if an external
//! crystal XT1 or XT2 is used and the user intends to call UCS_getMCLK,
//! UCS_getSMCLK or UCS_getACLK APIs. If not, it is not necessary to invoke this
//! API.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param XT1CLK_frequency is the XT1 crystal frequencies in Hz
//! \param XT2CLK_frequency is the XT2 crystal frequencies in Hz
//!
//! \return None
//
//******************************************************************************
void
UCS_setExternalClockSource (unsigned int baseaddress,
    unsigned long XT1CLK_frequency,
    unsigned long XT2CLK_frequency
    )
{
    UCS_XT1ClockFrequency = XT1CLK_frequency;
    UCS_XT2ClockFrequency = XT2CLK_frequency;
}

//******************************************************************************
//
//! This function initializes each of the clock signals. The user must ensure
//! that this function is called for each clock signal. If not, the default
//! state is assumed for the particular clock signal. Refer MSP430ware
//! documentation for UCS module or Device Family User's Guide for details of
//! default clock signal states
//!
//! \param baseAddress is the base address of the UCS module.
//! \param selectedClockSignal - Valid values are
//!           \b UCS_ACLK,
//!           \b UCS_MCLK,
//!           \b UCS_SMCLK,
//!           \b UCS_FLLREF
//! \param clockSource is Clock source for the selectedClock Signal
//!            Valid values are
//!            \b UCS_XT1CLK_SELECT,
//!            \b UCS_VLOCLK_SELECT,
//!            \b UCS_REFOCLK_SELECT,
//!            \b UCS_DCOCLK_SELECT,
//!            \b UCS_DCOCLKDIV_SELECT
//!            \b UCS_XT2CLK_SELECT
//! \param clockSourceDivider - selected the clock divider to calculate
//!         clocksignal from clock source. Valid values are
//!           \b UCS_CLOCK_DIVIDER_1 [Default Value],
//!           \b UCS_CLOCK_DIVIDER_2,
//!           \b UCS_CLOCK_DIVIDER_4,
//!           \b UCS_CLOCK_DIVIDER_8,
//!           \b UCS_CLOCK_DIVIDER_12, [Valid ONLY for \b UCS_FLLREF]
//!           \b UCS_CLOCK_DIVIDER_16,
//!           \b UCS_CLOCK_DIVIDER_32 [Not valid for \b UCS_FLLREF]
//!
//! Modified registers are \b UCSCTL4, \b UCSCTL5, \b UCSCTL3
//! Note that the dividers for \b UCS_FLLREF are different from the available
//! clock dividers.
//!
//! \return None
//
//******************************************************************************
void
UCS_clockSignalInit ( unsigned int baseaddress,
    unsigned char selectedClockSignal,
    unsigned int clockSource,
    unsigned char clockSourceDivider
    )
{
    ASSERT(
        (UCS_XT1CLK_SELECT == clockSource) ||
        (UCS_VLOCLK_SELECT == clockSource) ||
        (UCS_REFOCLK_SELECT == clockSource) ||
        (UCS_DCOCLK_SELECT == clockSource) ||
        (UCS_DCOCLKDIV_SELECT == clockSource) ||
        (UCS_XT2CLK_SELECT == clockSource)
        );

    ASSERT(
        (UCS_CLOCK_DIVIDER_1 == clockSourceDivider) ||
        (UCS_CLOCK_DIVIDER_2 == clockSourceDivider) ||
        (UCS_CLOCK_DIVIDER_4 == clockSourceDivider) ||
        (UCS_CLOCK_DIVIDER_8 == clockSourceDivider) ||
        (UCS_CLOCK_DIVIDER_16 == clockSourceDivider) ||
        (UCS_CLOCK_DIVIDER_32 == clockSourceDivider)
        );

    switch (selectedClockSignal){
        case UCS_ACLK:
            HWREG(baseaddress + OFS_UCSCTL4) &= ~(SELA_7);
            clockSource = clockSource << 8;
            HWREG(baseaddress + OFS_UCSCTL4) |= (clockSource);

            HWREG(baseaddress + OFS_UCSCTL5) &= ~(DIVA_7);
            clockSourceDivider = clockSourceDivider << 8;
            HWREG(baseaddress + OFS_UCSCTL5) |= clockSourceDivider;
            break;
        case UCS_SMCLK:
            HWREG(baseaddress + OFS_UCSCTL4) &= ~(SELS_7);
            clockSource = clockSource << 4;
            HWREG(baseaddress + OFS_UCSCTL4) |= (clockSource);

            HWREG(baseaddress + OFS_UCSCTL5) &= ~(DIVS_7);
            clockSourceDivider = clockSourceDivider << 4;
            HWREG(baseaddress + OFS_UCSCTL5) |= clockSourceDivider;
            break;
        case UCS_MCLK:
            HWREG(baseaddress + OFS_UCSCTL4) &= ~(SELM_7);
            HWREG(baseaddress + OFS_UCSCTL4) |= (clockSource);

            HWREG(baseaddress + OFS_UCSCTL5) &= ~(DIVA_7);
            HWREG(baseaddress + OFS_UCSCTL5) |= clockSourceDivider;
            break;
        case UCS_FLLREF:
            ASSERT(clockSource <= SELA_5);
            HWREGB(baseaddress + OFS_UCSCTL3) &=  ~(SELREF_7);

            clockSource = clockSource << 4;
            HWREGB(baseaddress + OFS_UCSCTL3) |= (clockSource);

            HWREGB(baseaddress + OFS_UCSCTL3) &= ~(FLLREFDIV_7);
            //Note that dividers for FLLREF are slightly different
            //Hence handled differently from other CLK signals
            switch(clockSourceDivider)
            {
              case UCS_CLOCK_DIVIDER_12:  
                HWREGB(baseaddress + OFS_UCSCTL3) |= UCS_CLOCK_DIVIDER_16;
                break;
              case UCS_CLOCK_DIVIDER_16:  
                HWREGB(baseaddress + OFS_UCSCTL3) |= UCS_CLOCK_DIVIDER_12;
                break;
              default:       
                HWREGB(baseaddress + OFS_UCSCTL3) |= clockSourceDivider;
                break;
            }
            
            break;
    }
}

//******************************************************************************
//
//! Initializes the XT1 crystal oscillator in low frequency mode. Loops until
//! all oscillator fault flags are cleared, with no timeout. See the
//! device-specific data sheet for appropriate drive settings.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param xt1drive is the target drive strength for the XT1 crystal oscillator.
//!         Valid values are
//!         \b UCS_XT1_DRIVE0,
//!         \b UCS_XT1_DRIVE1,
//!         \b UCS_XT1_DRIVE2,
//!         \b UCS_XT1_DRIVE3 [Default value].
//!        Modified bits are \b XT1DRIVE of \b UCSCTL6 register.
//! \param xcap is the selected capactor value.
//!         Valid values are:
//!         \b UCS_XCAP_0,
//!         \b UCS_XCAP_1,
//!         \b UCS_XCAP_2
//!         \b UCS_XCAP_3 [Default value]
//!         This parameter selects the capacitors applied to the LF crystal
//!         (XT1) or resonator in the LF mode. The  effective capacitance
//!         (seen by the crystal) is Ceff . (CXIN + 2 pF)/2. It is assumed that
//!         CXIN = CXOUT and that a parasitic capacitance of 2 pF is added by
//!         the package and the printed circuit board. For details about the
//!         typical internal and the effective capacitors, refer to the
//!         device-specific data sheet.
//!
//!        Modified bits are \b XCAP of \b UCSCTL6 register.
//!
//! \return None
//
//******************************************************************************
void
UCS_LFXT1Start ( unsigned int baseAddress,
    unsigned int xt1drive,
    unsigned char xcap
    )
{
    ASSERT((xcap == UCS_XCAP_0) ||
        (xcap == UCS_XCAP_1) ||
        (xcap == UCS_XCAP_2) ||
        (xcap == UCS_XCAP_3) );

    ASSERT((xt1drive == UCS_XT1_DRIVE0 ) ||
        (xt1drive == UCS_XT1_DRIVE1 ) ||
        (xt1drive == UCS_XT1_DRIVE2 ) ||
        (xt1drive == UCS_XT1_DRIVE3 ));

    //If the drive setting is not already set to maximum
    //Set it to max for LFXT startup
    if ((HWREG(baseAddress + OFS_UCSCTL6) & XT1DRIVE_3) != XT1DRIVE_3){
        //Highest drive setting for XT1startup
        HWREG(baseAddress + OFS_UCSCTL6_L) |= XT1DRIVE1_L + XT1DRIVE0_L;
    }

    //Enable LF mode and clear xcap and bypass
    HWREG(baseAddress + OFS_UCSCTL6) &= ~(XTS + XCAP_3 + XT1BYPASS);
    HWREG(baseAddress + OFS_UCSCTL6) |= xcap;

    while (HWREGB(baseAddress + OFS_UCSCTL7) & XT1LFOFFG)
    {
        //Clear OSC flaut Flags fault flags
        HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1LFOFFG);

        //Clear OFIFG fault flag
        HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
    }

    //set requested Drive mode
    HWREG(baseAddress + OFS_UCSCTL6) = ( HWREG(baseAddress + OFS_UCSCTL6) &
                                         ~(XT1DRIVE_3)
                                         ) |
                                       (xt1drive);


    //Switch ON XT1 oscillator
    HWREG(baseAddress + OFS_UCSCTL6) &= ~XT1OFF;
}

//******************************************************************************
//
//! Initializes the XT1 crystal oscillator in high frequency mode. Loops until
//! all oscillator fault flags are cleared, with no timeout. See the
//! device-specific data sheet for appropriate drive settings.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param xt1drive is the target drive strength for the XT1 crystal oscillator.
//!        Valid values are
//!         \b UCS_XT1_DRIVE0 ,
//!         \b UCS_XT1_DRIVE1,
//!         \b UCS_XT1_DRIVE2 ,
//!         \b UCS_XT1_DRIVE3[Default Value]
//!
//! Modified registers are \b UCSCTL6, \b UCSCTL7, \b SFRIFG
//!
//! \return None
//
//******************************************************************************
void
UCS_HFXT1Start (
    unsigned int baseAddress,
    unsigned int xt1drive
    )
{
    //Check if drive value is the expected one
    if ((HWREG(baseAddress + OFS_UCSCTL6) & XT1DRIVE_3) != xt1drive){
        //Clear XT1drive field
        HWREG(baseAddress + OFS_UCSCTL6) &= ~XT1DRIVE_3;

        //Set requested value
        HWREG(baseAddress + OFS_UCSCTL6) |= xt1drive;
    }

    //Enable HF mode
    HWREG(baseAddress + OFS_UCSCTL6) |= XTS;

    HWREG(baseAddress + OFS_UCSCTL6) &= ~XT1BYPASS;

    // Check XT1 fault flags
    while((HWREGB(baseAddress + OFS_UCSCTL7) & (XT1HFOFFG))){
        //Clear OSC flaut Flags fault flags
        HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1HFOFFG);

        //Clear OFIFG fault flag
        HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
    }

    //Switch ON XT1 oscillator
    HWREG(baseAddress + OFS_UCSCTL6) &= ~XT1OFF;
}

//******************************************************************************
//
//! Bypasses the XT1 crystal oscillator. Loops until all oscillator fault
//! flags are cleared, with no timeout.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param highOrLowFrequency selects high frequency or low frequency mode for
//!         XT1. Valid values are
//!        \b UCS_XT1_HIGH_FREQUENCY,
//!        \b UCS_XT1_LOW_FREQUENCY [Default Value]
//!
//! Modified registers are \b UCSCTL6, \b UCSCTL7, \b SFRIFG
//! \return None
//
//******************************************************************************
void
UCS_bypassXT1 ( unsigned int baseAddress,
    unsigned char highOrLowFrequency
    )
{
    ASSERT((UCS_XT1_LOW_FREQUENCY == highOrLowFrequency) ||
        (UCS_XT1_HIGH_FREQUENCY == highOrLowFrequency )
        );

    //Enable HF/LF mode
    HWREG(baseAddress + OFS_UCSCTL6) &= ~XTS;
    HWREG(baseAddress + OFS_UCSCTL6) |= highOrLowFrequency;

    //Switch OFF XT1 oscillator and enable BYPASS mode
    HWREG(baseAddress + OFS_UCSCTL6) |= (XT1BYPASS + XT1OFF);

   
    if (UCS_XT1_LOW_FREQUENCY == highOrLowFrequency){
      while (HWREGB(baseAddress + OFS_UCSCTL7) & (XT1LFOFFG)) {
        //Clear OSC flaut Flags fault flags
        HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1LFOFFG);
        
        // Clear the global fault flag. In case the XT1 caused the global fault 
        // flag to get set this will clear the global error condition. If any 
        // error condition persists, global flag will get again.
        HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
      }
    } else   {
        while (HWREGB(baseAddress + OFS_UCSCTL7) & (XT1HFOFFG)) {
          //Clear OSC flaut Flags fault flags
          HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1HFOFFG);
          
          //Clear the global fault flag. In case the XT1 caused the global fault 
          //flag to get set this will clear the global error condition. If any 
          //error condition persists, global flag will get again.
          HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
        }
      }
 
}

//******************************************************************************
//
//! Initializes the XT1 crystal oscillator in low frequency mode with timeout.
//! Loops until all oscillator fault flags are cleared or until a timeout
//! counter is decremented and equals to zero. See the device-specific
//! datasheet for appropriate drive settings.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param xt1drive is the target drive strength for the XT1 crystal oscillator.
//!        Valid values are
//!         \b UCS_XT1_DRIVE0,
//!         \b UCS_XT1_DRIVE1,
//!         \b UCS_XT1_DRIVE2,
//!         \b UCS_XT1_DRIVE3[Default Value]
//! \param xcap is the selected capactor value. Valid values are:
//!        \b UCS_XCAP_0,
//!        \b UCS_XCAP_1,
//!        \b UCS_XCAP_2,
//!        \b UCS_XCAP_3[Default Value]
//!  This parameter selects the capacitors applied to the LF crystal
//! (XT1) or resonator in the LF mode. The  effective capacitance
//! (seen by the crystal) is Ceff . (CXIN + 2 pF)/2. It is assumed that
//! CXIN = CXOUT and that a parasitic capacitance of 2 pF is added by
//! the package and the printed circuit board. For details about the
//! typical internal and the effective capacitors, refer to the
//! device-specific data sheet.
//!
//! \param timeout is the count value that gets decremented every time the loop
//!         that clears oscillator fault flags gets executed.
//!
//! Modified registers are \b UCSCTL6, \b UCSCTL7, \b SFRIFG
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//******************************************************************************
unsigned short
UCS_LFXT1StartWithTimeout (
    unsigned int baseAddress,
    unsigned int xt1drive,
    unsigned char xcap,
    unsigned int timeout
    )
{
    ASSERT((xcap == UCS_XCAP_0) ||
        (xcap == UCS_XCAP_1) ||
        (xcap == UCS_XCAP_2) ||
        (xcap == UCS_XCAP_3) );

    ASSERT((xt1drive == UCS_XT1_DRIVE0 ) ||
        (xt1drive == UCS_XT1_DRIVE1 ) ||
        (xt1drive == UCS_XT1_DRIVE2 ) ||
        (xt1drive == UCS_XT1_DRIVE3 ));

    //If the drive setting is not already set to maximum
    //Set it to max for LFXT startup
    if ((HWREG(baseAddress + OFS_UCSCTL6) & XT1DRIVE_3) != XT1DRIVE_3){
        //Highest drive setting for XT1startup
        HWREG(baseAddress + OFS_UCSCTL6_L) |= XT1DRIVE1_L + XT1DRIVE0_L;
    }

    //Enable LF mode
    HWREG(baseAddress + OFS_UCSCTL6) &= ~(
        XTS +
        XT1BYPASS
        );

    do
    {
        HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1LFOFFG);

        //Clear OFIFG fault flag
        HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
    }while ((HWREGB(baseAddress + OFS_UCSCTL7) & XT1LFOFFG) && --timeout);

    if (timeout){
        //set requested Drive mode
        HWREG(baseAddress + OFS_UCSCTL6) = ( HWREG(baseAddress + OFS_UCSCTL6) &
                                             ~(XT1DRIVE_3)
                                             ) |
                                           (xt1drive);
        //Switch ON XT1 oscillator
        HWREG(baseAddress + OFS_UCSCTL6) &= ~XT1OFF;

        return (STATUS_SUCCESS);
    } else   {
        return (STATUS_FAIL);
    }
}

//******************************************************************************
//
//! Initializes the XT1 crystal oscillator in high freqquency mode with timeout.
//! Loops until all oscillator fault flags are cleared or until a timeout
//! counter is decremented and equals to zero. See the device-specific data
//! sheet for appropriate drive settings.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param xt1drive is the target drive strength for the XT1 crystal oscillator.
//!        Valid values are
//!         \b UCS_XT1_DRIVE0,
//!         \b UCS_XT1_DRIVE1,
//!         \b UCS_XT1_DRIVE2,
//!         \b UCS_XT1_DRIVE3 [Default Value]
//! \param timeout is the count value that gets decremented every time the loop
//!         that clears oscillator fault flags gets executed.
//!
//! Modified registers are \b UCSCTL6, \b UCSCTL7, \b SFRIFG
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//******************************************************************************
unsigned short
UCS_HFXT1StartWithTimeout (  unsigned int baseAddress,
    unsigned int xt1drive,
    unsigned int timeout
    )
{
    ASSERT((xt1drive == UCS_XT1_DRIVE0 ) ||
        (xt1drive == UCS_XT1_DRIVE1 ) ||
        (xt1drive == UCS_XT1_DRIVE2 ) ||
        (xt1drive == UCS_XT1_DRIVE3 ));


    //Check if drive value is the expected one
    if ((HWREG(baseAddress + OFS_UCSCTL6) & XT1DRIVE_3) != xt1drive){
        //Clear XT1drive field
        HWREG(baseAddress + OFS_UCSCTL6) &= ~XT1DRIVE_3;

        //Set requested value
        HWREG(baseAddress + OFS_UCSCTL6) |= xt1drive;
    }

    //Enable HF mode
    HWREG(baseAddress + OFS_UCSCTL6) |= XTS;

    HWREG(baseAddress + OFS_UCSCTL6) &= ~XT1BYPASS;

    // Check XT1 fault flags
    do
    {
        HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1HFOFFG);

        //Clear OFIFG fault flag
        HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
    }while ((HWREGB(baseAddress + OFS_UCSCTL7) & ( XT1HFOFFG))
            && --timeout); 

    if (timeout){
        //Switch ON XT1 oscillator
        HWREG(baseAddress + OFS_UCSCTL6) &= ~XT1OFF;

        return (STATUS_SUCCESS);
    } else   {
        return (STATUS_FAIL);
    }
}

//******************************************************************************
//
//! Bypasses the XT1 crystal oscillator with time out. Loops until all
//! oscillator fault flags are cleared or until a timeout counter is
//! decremented and equals to zero.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param highOrLowFrequency selects high frequency or low frequency mode for
//!        XT1. Valid values are
//!        \b UCS_XT1_HIGH_FREQUENCY,
//!        \b UCS_XT1_LOW_FREQUENCY [Default Value]
//! \param timeout is the count value that gets decremented every time the loop
//!         that clears oscillator fault flags gets executed.
//!
//! Modified registers are \b UCSCTL6, \b UCSCTL7, \b SFRIFG
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//******************************************************************************
unsigned short
UCS_bypassXT1WithTimeout (
    unsigned int baseAddress,
    unsigned char highOrLowFrequency,
    unsigned int timeout
    )
{
    ASSERT((UCS_XT1_LOW_FREQUENCY == highOrLowFrequency) ||
        (UCS_XT1_HIGH_FREQUENCY == highOrLowFrequency )
        );

    //Enable HF/LF mode
    HWREG(baseAddress + OFS_UCSCTL6) &= ~XTS;
    HWREG(baseAddress + OFS_UCSCTL6) |= highOrLowFrequency;

    //Switch OFF XT1 oscillator  and enable bypass
    HWREG(baseAddress + OFS_UCSCTL6) |= (XT1BYPASS + XT1OFF);

       
    if (UCS_XT1_LOW_FREQUENCY == highOrLowFrequency){
      do {
        //Clear OSC flaut Flags fault flags
        HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1LFOFFG);
        
        // Clear the global fault flag. In case the XT1 caused the global fault 
        // flag to get set this will clear the global error condition. If any 
        // error condition persists, global flag will get again.
        HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
      }while ((HWREGB(baseAddress + OFS_UCSCTL7) & (XT1LFOFFG)) && --timeout);
      
    } else   {
        do {
          //Clear OSC flaut Flags fault flags
          HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1HFOFFG);
          
          //Clear the global fault flag. In case the XT1 caused the global fault 
          //flag to get set this will clear the global error condition. If any 
          //error condition persists, global flag will get again.
          HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
        }while ((HWREGB(baseAddress + OFS_UCSCTL7) & (XT1HFOFFG))&& --timeout);
    }


    if (timeout){
        return (STATUS_SUCCESS);
    } else {
        return (STATUS_FAIL);
    }
}

//******************************************************************************
//
//! Stops the XT1 oscillator using the XT1OFF bit.
//!
//! \param baseAddress is the base address of the UCS module.
//!
//! \return None
//
//******************************************************************************
void
UCS_XT1Off (unsigned int baseAddress)
{
    //Switch off XT1 oscillator
    HWREG(baseAddress + OFS_UCSCTL6) |= XT1OFF;
}

//******************************************************************************
//
//! Initializes the XT2 crystal oscillator, which supports crystal frequencies
//! between 4 MHz and 32 MHz, depending on the selected drive strength. Loops
//! until all oscillator fault flags are cleared, with no timeout. See the
//! device-specific data sheet for appropriate drive settings.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param xt2drive is the target drive strength for the XT2 crystal oscillator.
//!      Valid values are
//!     \b UCS_XT2DRIVE_4MHZ_8MHZ,
//!     \b UCS_XT2DRIVE_8MHZ_16MHZ,
//!     \b UCS_XT2DRIVE_16MHZ_24MHZ,
//!     \b UCS_XT2DRIVE_24MHZ_32MHZ [Default Value]
//!
//! Modified registers are \b UCSCTL6, \b UCSCTL7, \b SFRIFG
//!
//! \return None
//
//******************************************************************************
void
UCS_XT2Start (  unsigned int baseAddress,
    unsigned int xt2drive
    )
{
#if !defined (__CC430F5133__) || (__CC430F5135__) || (__CC430F5137__) || \
            (__CC430F6125__) || (__CC430F6126__) || (__CC430F6127__) || \
            (__CC430F6135__) || (__CC430F6137__) || (__CC430F5123__) || \
            (__CC430F5125__) || (__CC430F5143__) || (__CC430F5145__) || \
            (__CC430F5147__) || (__CC430F6143__) || (__CC430F6145__) || \
            (__CC430F6147__)

    //Check if drive value is the expected one
    if ((HWREG(baseAddress + OFS_UCSCTL6) & XT2DRIVE_3) != xt2drive){
        //Clear XT2drive field
        HWREG(baseAddress + OFS_UCSCTL6) &= ~XT2DRIVE_3;

        //Set requested value
        HWREG(baseAddress + OFS_UCSCTL6) |= xt2drive;
    }
#endif     

    //Enable XT2 and Switch on XT2 oscillator
    HWREG(baseAddress + OFS_UCSCTL6) &= ~XT2BYPASS;
    HWREG(baseAddress + OFS_UCSCTL6) &= ~XT2OFF;

    while (HWREGB(baseAddress + OFS_UCSCTL7) & XT2OFFG){
     //Clear OSC flaut Flags
     HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT2OFFG);
     
#if defined (__CC430F5133__) || (__CC430F5135__) || (__CC430F5137__) || \
            (__CC430F6125__) || (__CC430F6126__) || (__CC430F6127__) || \
            (__CC430F6135__) || (__CC430F6137__) || (__CC430F5123__) || \
            (__CC430F5125__) || (__CC430F5143__) || (__CC430F5145__) || \
            (__CC430F5147__) || (__CC430F6143__) || (__CC430F6145__) || \
            (__CC430F6147__)
        // CC430 uses a different fault mechanism. It requires 3 VLO clock 
        // cycles delay.If 20MHz CPU, 5000 clock cycles are required in worst 
        // case.
        __delay_cycles(5000);
#endif

     //Clear OFIFG fault flag
     HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
    }
}

//******************************************************************************
//
//! Bypasses the XT2 crystal oscillator, which supports crystal frequencies
//! between 4 MHz and 32 MHz. Loops until all oscillator fault flags are
//! cleared, with no timeout.
//!
//! \param baseAddress is the base address of the UCS module.
//!
//! Modified registers are \b UCSCTL6, \b UCSCTL7, \b SFRIFG
//!
//! \return None
//
//******************************************************************************
void
UCS_bypassXT2 (  unsigned int baseAddress )
{
    //Switch on XT2 oscillator
    HWREG(baseAddress + OFS_UCSCTL6) |= ( XT2BYPASS + XT2OFF );

    while (HWREGB(baseAddress + OFS_UCSCTL7) & XT2OFFG){
     //Clear OSC flaut Flags
     HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT2OFFG);
     
#if defined (__CC430F5133__) || (__CC430F5135__) || (__CC430F5137__) || \
            (__CC430F6125__) || (__CC430F6126__) || (__CC430F6127__) || \
            (__CC430F6135__) || (__CC430F6137__) || (__CC430F5123__) || \
            (__CC430F5125__) || (__CC430F5143__) || (__CC430F5145__) || \
            (__CC430F5147__) || (__CC430F6143__) || (__CC430F6145__) || \
            (__CC430F6147__)
        // CC430 uses a different fault mechanism. It requires 3 VLO clock 
        // cycles delay.If 20MHz CPU, 5000 clock cycles are required in worst 
        // case.
        __delay_cycles(5000);
#endif

     //Clear OFIFG fault flag
     HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
    }
}

//******************************************************************************
//
//! Initializes the XT2 crystal oscillator, which supports crystal frequencies
//! between 4 MHz and 32 MHz, depending on the selected drive strength. Loops
//! until all oscillator fault flags are cleared or until a timeout counter is
//! decremented and equals to zero. See the device-specific data sheet for
//! appropriate drive settings.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param xt2drive is the target drive strength for the XT2 crystal oscillator.
//!        Valid values are
//!        \b UCS_XT2_4MHZ_8MHZ,
//!        \b UCS_XT2_8MHZ_16MHZ,
//!        \b UCS_XT2_16MHZ_24MHZ
//!        \b UCS_XT2_24MHZ_32MHZ [Default Value]
//! \param timeout is the count value that gets decremented every time the loop
//!         that clears oscillator fault flags gets executed.
//!
//! Modified registers are \b UCSCTL6, \b UCSCTL7, \b SFRIFG
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//******************************************************************************
unsigned short
UCS_XT2StartWithTimeout ( unsigned int baseAddress,
    unsigned int xt2drive,
    unsigned int timeout
    )
{
  
#if !defined (__CC430F5133__) || (__CC430F5135__) || (__CC430F5137__) || \
            (__CC430F6125__) || (__CC430F6126__) || (__CC430F6127__) || \
            (__CC430F6135__) || (__CC430F6137__) || (__CC430F5123__) || \
            (__CC430F5125__) || (__CC430F5143__) || (__CC430F5145__) || \
            (__CC430F5147__) || (__CC430F6143__) || (__CC430F6145__) || \
            (__CC430F6147__)
    //Check if drive value is the expected one
    if ((HWREG(baseAddress + OFS_UCSCTL6) & XT2DRIVE_3) != xt2drive){
        //Clear XT2drive field
        HWREG(baseAddress + OFS_UCSCTL6) &= ~XT2DRIVE_3;

        //Set requested value
        HWREG(baseAddress + OFS_UCSCTL6) |= xt2drive;
    }

#endif

    HWREG(baseAddress + OFS_UCSCTL6) &= ~XT2BYPASS;

    //Switch on XT2 oscillator
    HWREG(baseAddress + OFS_UCSCTL6) &= ~XT2OFF;

    do{             
     //Clear OSC flaut Flags
     HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT2OFFG);
     
#if defined (__CC430F5133__) || (__CC430F5135__) || (__CC430F5137__) || \
            (__CC430F6125__) || (__CC430F6126__) || (__CC430F6127__) || \
            (__CC430F6135__) || (__CC430F6137__) || (__CC430F5123__) || \
            (__CC430F5125__) || (__CC430F5143__) || (__CC430F5145__) || \
            (__CC430F5147__) || (__CC430F6143__) || (__CC430F6145__) || \
            (__CC430F6147__)
        // CC430 uses a different fault mechanism. It requires 3 VLO clock 
        // cycles delay.If 20MHz CPU, 5000 clock cycles are required in worst 
        // case.
        __delay_cycles(5000);
#endif

     //Clear OFIFG fault flag
     HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
    }while ((HWREGB(baseAddress + OFS_UCSCTL7) & XT2OFFG) && --timeout);
    
    if (timeout){
        return (STATUS_SUCCESS);
    } else   {
        return (STATUS_FAIL);
    }
}

//******************************************************************************
//
//! Bypasses the XT2 crystal oscillator, which supports crystal frequencies
//! between 4 MHz and 32 MHz. Loops until all oscillator fault flags are
//! cleared or until a timeout counter is decremented and equals to zero.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param timeout is the count value that gets decremented every time the loop
//!         that clears oscillator fault flags gets executed.
//!
//! Modified registers are \b UCSCTL6, \b UCSCTL7, \b SFRIFG
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//******************************************************************************
unsigned short
UCS_bypassXT2WithTimeout ( unsigned int baseAddress,
    unsigned int timeout
    )
{
    //Switch off XT2 oscillator and enable BYPASS mode
    HWREG(baseAddress + OFS_UCSCTL6) |= (XT2BYPASS + XT2OFF );


    do{             
     //Clear OSC flaut Flags
     HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT2OFFG);

#if defined (__CC430F5133__) || (__CC430F5135__) || (__CC430F5137__) || \
            (__CC430F6125__) || (__CC430F6126__) || (__CC430F6127__) || \
            (__CC430F6135__) || (__CC430F6137__) || (__CC430F5123__) || \
            (__CC430F5125__) || (__CC430F5143__) || (__CC430F5145__) || \
            (__CC430F5147__) || (__CC430F6143__) || (__CC430F6145__) || \
            (__CC430F6147__)
        // CC430 uses a different fault mechanism. It requires 3 VLO clock 
        // cycles delay.If 20MHz CPU, 5000 clock cycles are required in worst 
        // case.
        __delay_cycles(5000);
#endif
        
     //Clear OFIFG fault flag
     HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
    }while ((HWREGB(baseAddress + OFS_UCSCTL7) & XT2OFFG) && --timeout);
    
    if (timeout){
        return (STATUS_SUCCESS);
    } else   {
        return (STATUS_FAIL);
    }
}

//******************************************************************************
//
//! Stops the XT2 oscillator using the XT2OFF bit.
//!
//! \param baseAddress is the base address of the UCS module.
//!
//! Modified registers are \b UCSCTL6
//!
//! \return None
//
//******************************************************************************
void
UCS_XT2Off (unsigned int baseAddress)
{
    //Switch off XT2 oscillator
    HWREG(baseAddress + OFS_UCSCTL6) |= XT2OFF;
}

//******************************************************************************
//
//! Initializes the DCO to operate a frequency that is a multiple of the
//! reference frequency into the FLL. Loops until all oscillator fault flags are
//! cleared, with a timeout. If the frequency is greater than 16 MHz,
//! the function sets the MCLK and SMCLK source to the undivided DCO frequency.
//! Otherwise, the function sets the MCLK and SMCLK source to the
//! DCOCLKDIV frequency.
//! This function executes a software delay that is proportional in length to
//! the ratio of the target FLL frequency and the FLL reference.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param fsystem is the target frequency for MCLK in kHz
//! \param ratio is the ratio x/y, where x = fsystem and
//!         y = FLL reference frequency.
//!
//! Modified registers are \b UCSCTL0, \b UCSCTL1, \b UCSCTL2, \b UCSCTL4,
//! \b UCSCTL7, \b SFRIFG1
//!
//! \return None
//
//******************************************************************************
void
UCS_initFLLSettle ( unsigned int baseAddress,
    unsigned int fsystem,
    unsigned int ratio
    )
{
    volatile unsigned int x = ratio * 32;

    UCS_initFLL(baseAddress, fsystem, ratio);

    while (x--)
    {
        __delay_cycles(30);
    }
}

//******************************************************************************
//
//! Initializes the DCO to operate a frequency that is a multiple of the
//! reference frequency into the FLL. Loops until all oscillator fault flags are
//! cleared, with no timeout. If the frequency is greater than 16 MHz,
//! the function sets the MCLK and SMCLK source to the undivided DCO frequency.
//! Otherwise, the function sets the MCLK and SMCLK source to the
//! DCOCLKDIV frequency.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param fsystem is the target frequency for MCLK in kHz
//! \param ratio is the ratio x/y, where x = fsystem and
//!         y = FLL reference frequency.
//!
//! Modified registers are \b UCSCTL0, \b UCSCTL1, \b UCSCTL2, \b UCSCTL4,
//! \b UCSCTL7, \b SFRIFG1
//!
//! \return None
//
//******************************************************************************
void
UCS_initFLL ( unsigned int baseAddress,
    unsigned int fsystem,
    unsigned int ratio
    )
{
    unsigned int d, dco_div_bits;
    unsigned int mode = 0;

    //Save actual state of FLL loop control, then disable it. This is needed to
    //prevent the FLL from acting as we are making fundamental modifications to
    //the clock setup.
    unsigned int srRegisterState = __get_SR_register() & SCG0;

    d = ratio;
    //Have at least a divider of 2
    dco_div_bits = FLLD__2;

    if (fsystem > 16000){
        d >>= 1 ;
        mode = 1;
    } else   {
        //fsystem = fsystem * 2
        fsystem <<= 1;
    }

    while (d > 512)
    {
        //Set next higher div level
        dco_div_bits = dco_div_bits + FLLD0;
        d >>= 1;
    }

    // Disable FLL
    __bis_SR_register(SCG0);                                    

    //Set DCO to lowest Tap
    HWREGB(baseAddress + OFS_UCSCTL0_H) = 0x0000;

    //Reset FN bits
    HWREG(baseAddress + OFS_UCSCTL2) &= ~(0x03FF);
    HWREG(baseAddress + OFS_UCSCTL2) = dco_div_bits | (d - 1);

    if (fsystem <= 630){           //fsystem < 0.63MHz
        HWREGB(baseAddress + OFS_UCSCTL1) = DCORSEL_0;
    } else if (fsystem <  1250){      //0.63MHz < fsystem < 1.25MHz
        HWREGB(baseAddress + OFS_UCSCTL1) = DCORSEL_1;
    } else if (fsystem <  2500){      //1.25MHz < fsystem <  2.5MHz
        HWREGB(baseAddress + OFS_UCSCTL1) = DCORSEL_2;
    } else if (fsystem <  5000){      //2.5MHz  < fsystem <    5MHz
        HWREGB(baseAddress + OFS_UCSCTL1) = DCORSEL_3;
    } else if (fsystem <  10000){     //5MHz    < fsystem <   10MHz
        HWREGB(baseAddress + OFS_UCSCTL1) = DCORSEL_4;
    } else if (fsystem <  20000){     //10MHz   < fsystem <   20MHz
        HWREGB(baseAddress + OFS_UCSCTL1) = DCORSEL_5;
    } else if (fsystem <  40000){     //20MHz   < fsystem <   40MHz
        HWREGB(baseAddress + OFS_UCSCTL1) = DCORSEL_6;
    } else {
        HWREGB(baseAddress + OFS_UCSCTL1) = DCORSEL_7;
    }

    // Re-enable FLL
     __bic_SR_register(SCG0);                                    
    
    while (HWREGB(baseAddress + OFS_UCSCTL7_L) & DCOFFG)
    {
        //Clear OSC flaut Flags
        HWREGB(baseAddress + OFS_UCSCTL7_L) &= ~(DCOFFG);

        //Clear OFIFG fault flag
        HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
    }

    // Restore previous SCG0
    __bis_SR_register(srRegisterState);                         
    
    if (mode == 1){
        //fsystem > 16000
        //Select DCOCLK
        HWREG(baseAddress + OFS_UCSCTL4) &=  ~(SELM_7 + SELS_7);
        HWREG(baseAddress + OFS_UCSCTL4) |= SELM__DCOCLK + SELS__DCOCLK;
    } else   {
        //Select DCODIVCLK
        HWREG(baseAddress + OFS_UCSCTL4) &=  ~(SELM_7 + SELS_7);
        HWREG(baseAddress + OFS_UCSCTL4) |= SELM__DCOCLKDIV + SELS__DCOCLKDIV;
    }

}

//******************************************************************************
//
//! Enables conditional module requests
//!
//! \param baseAddress is the base address of the UCS module.
//! \param selectClock selects specific request enables. Valid values are
//!        \b UCS_ACLK,
//!        \b UCS_SMCLK,
//!        \b UCS_MCLK,
//!        \b UCS_MODOSC
//!
//! Modified registers are \b UCSCTL8
//!
//! \return None
//
//******************************************************************************
void
UCS_enableClockRequest (
    unsigned int baseAddress,
    unsigned char selectClock
    )
{
    HWREGB(baseAddress + OFS_UCSCTL8) |= selectClock;
}

//******************************************************************************
//
//! Disables conditional module requests
//!
//! \param baseAddress is the base address of the UCS module.
//! \param selectClock selects specific request enables. Valid values are
//!        \b UCS_ACLK,
//!        \b UCS_SMCLK,
//!        \b UCS_MCLK,
//!        \b UCS_MODOSC
//!
//! Modified registers are \b UCSCTL8
//!
//! \return None
//
//******************************************************************************
void
UCS_disableClockRequest (
    unsigned int baseAddress,
    unsigned char selectClock
    )
{
    HWREGB(baseAddress + OFS_UCSCTL8) &= ~selectClock;
}

//******************************************************************************
//
//! Gets the current UCS fault flag status.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param mask is the masked interrupt flag status to be returned.
//!      Mask parameter can be either any of the following selection.
//!         - \b UCS_XT2OFFG - XT2 oscillator fault flag
//!         - \b UCS_XT1HFOFFG - XT1 oscillator fault flag (HF mode)
//!         - \b UCS_XT1LFOFFG - XT1 oscillator fault flag (LF mode)
//!         - \b UCS_DCOFFG - DCO fault flag
//!
//! Modified registers are \b UCSCTL7
//!
//! \return The current flag status for the corresponding masked bit
//
//******************************************************************************
unsigned char
UCS_faultFlagStatus (
    unsigned int baseAddress,
    unsigned char mask
    )
{
    ASSERT(mask <= UCS_XT2OFFG );
    return (HWREGB(baseAddress + OFS_UCSCTL7) & mask);
}

//******************************************************************************
//
//! Clears the current UCS fault flag status for the masked bit.
//!
//! \param baseAddress is the base address of the UCS module.
//! \param mask is the masked interrupt flag status to be returned.
//!         mask parameter can be any one of the following
//!             - \b UCS_XT2OFFG - XT2 oscillator fault flag
//!             - \b UCS_XT1HFOFFG - XT1 oscillator fault flag (HF mode)
//!             - \b UCS_XT1LFOFFG - XT1 oscillator fault flag (LF mode)
//!             - \b UCS_DCOFFG - DCO fault flag
//!
//! Modified registers are \b UCSCTL7
//!
//! \return None
//
//******************************************************************************
void
UCS_clearFaultFlag (
    unsigned int baseAddress,
    unsigned char mask
    )
{
    ASSERT(mask <= UCS_XT2OFFG );
    HWREGB(baseAddress + OFS_UCSCTL7) &= ~mask;
}

//******************************************************************************
//
//! Turns off SMCLK using the SMCLKOFF bit
//!
//! \param baseAddress is the base address of the UCS module.
//!
//! Modified registers are UCSCTL6
//!
//! \return None
//
//******************************************************************************
void
UCS_SMCLKOff (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_UCSCTL6) |= SMCLKOFF;
}

//******************************************************************************
//
//! Turns ON SMCLK using the SMCLKOFF bit
//!
//! \param baseAddress is the base address of the UCS module.
//!
//! Modified registers are \b UCSCTL6
//!
//! \return None
//
//******************************************************************************
void
UCS_SMCLKOn (unsigned int baseAddress)
{
    HWREG(baseAddress + OFS_UCSCTL6) &= ~SMCLKOFF;
}

//******************************************************************************
//
//Compute the clock frequency when clock is sourced from DCO
//
//\param baseAddress is the base address of the UCS module.
//\param FLLRefCLKSource is clock source for FLL reference. Valid values are
//\b SELM__DCOCLK
//\b SELM__DCOCLKDIV
//
//\return Calculated clock frequency in Hz
//
//******************************************************************************
unsigned long
privateUCSSourceClockFromDCO ( unsigned int baseAddress,
    unsigned int FLLRefCLKSource
    )
{
    ASSERT((SELM__DCOCLKDIV == FLLRefCLKSource) ||
        (SELM__DCOCLK == FLLRefCLKSource)
        );
    unsigned int D_value = 1;
    unsigned int N_value;
    unsigned int n_value;
    unsigned long Fref_value = 0;
    unsigned char i;

    N_value = (HWREG(baseAddress + OFS_UCSCTL2)) & 0x03FF;
    unsigned int tempDivider = HWREGB(baseAddress + OFS_UCSCTL3) & FLLREFDIV_7;

    n_value = 1;

    for ( i = 0; i < tempDivider; i++){
        n_value *= 2;
    }

    switch ( (HWREGB(baseAddress + OFS_UCSCTL3)) & SELREF_7){
        case SELREF__XT1CLK:
            Fref_value = UCS_XT1ClockFrequency;

            if(XTS != (HWREG(baseAddress + OFS_UCSCTL6) & XTS)) {
              if (HWREGB(baseAddress + OFS_UCSCTL7) & XT1LFOFFG){
                HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1LFOFFG);
                //Clear OFIFG fault flag
                HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;

                if (HWREGB(baseAddress + OFS_UCSCTL7) & XT1LFOFFG){
                    Fref_value = UCS_REFOCLK_FREQUENCY;
                }
              }
            }
            else {
              if (HWREGB(baseAddress + OFS_UCSCTL7) & XT1HFOFFG){
                HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1HFOFFG);
                //Clear OFIFG fault flag
                HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;

                if (HWREGB(baseAddress + OFS_UCSCTL7) & XT1HFOFFG){
                    Fref_value = UCS_REFOCLK_FREQUENCY;
                }
              }
            }

            break;
        case SELREF__REFOCLK:
            Fref_value = UCS_REFOCLK_FREQUENCY;
            break;
        case SELREF__XT2CLK:
            Fref_value = UCS_XT2ClockFrequency;

            if (HWREGB(baseAddress + OFS_UCSCTL7) & XT2OFFG){
                HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT2OFFG);

                //Clear OFIFG fault flag
                HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;

                if (HWREGB(baseAddress + OFS_UCSCTL7) & XT2OFFG){
                    Fref_value = UCS_REFOCLK_FREQUENCY;
                }
            }

            break;
        default: ASSERT(0);
    }

    unsigned long CLKFrequency = Fref_value * ( N_value + 1) / n_value;

    if (SELM__DCOCLK == FLLRefCLKSource){
        unsigned char i;
        tempDivider = (HWREG(baseAddress + OFS_UCSCTL2)) & FLLD_7;
        tempDivider = tempDivider >> 12;

        for (i = 0; i < tempDivider; i++){
            D_value =  D_value * 2;
        }

        CLKFrequency *= D_value;
    }
    return ( CLKFrequency) ;
}

//******************************************************************************
//
//Compute the clock frequency given the clock source and divider
//
//\param baseAddress is the base address of the UCS module.
//\param CLKSource is the clock source. Valid values are
//\b SELM__XT1CLK,
//\b SELM__VLOCLK,
//\b SELM__REFOCLK,
//\b SELM__XT2CLK,
//\b SELM__DCOCLK,
//\b SELM__DCOCLKDIV
//\param CLKSourceDivider is the Clock source divider
//
//\return Calculated clock frequency in Hz
//
//******************************************************************************
unsigned long
privateUCSComputeCLKFrequency ( unsigned int baseAddress,
    unsigned int CLKSource,
    unsigned int CLKSourceDivider
    )
{
    unsigned long CLKFrequency = 0;
    unsigned char CLKSourceFrequencyDivider = 1;
    unsigned char i = 0;

    for ( i = 0; i < CLKSourceDivider; i++){
        CLKSourceFrequencyDivider *= 2;
    }

    switch (CLKSource){
        case SELM__XT1CLK:
            CLKFrequency = (UCS_XT1ClockFrequency /
                            CLKSourceFrequencyDivider);

            if(XTS != (HWREG(baseAddress + OFS_UCSCTL6) & XTS))  {
              if (HWREGB(baseAddress + OFS_UCSCTL7) & XT1LFOFFG){
                HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1LFOFFG);
                //Clear OFIFG fault flag
                HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;

                if (HWREGB(baseAddress + OFS_UCSCTL7) & XT1LFOFFG){
                    CLKFrequency = UCS_REFOCLK_FREQUENCY;
                }
              }
            }
            else {
              if (HWREGB(baseAddress + OFS_UCSCTL7) & XT1HFOFFG){
                HWREGB(baseAddress + OFS_UCSCTL7) &= ~(XT1HFOFFG);
                //Clear OFIFG fault flag
                HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;

                if (HWREGB(baseAddress + OFS_UCSCTL7) & XT1HFOFFG){
                    CLKFrequency = UCS_REFOCLK_FREQUENCY;
                }
              }
            }
            break;

        case SELM__VLOCLK:
            CLKFrequency =
                (UCS_VLOCLK_FREQUENCY / CLKSourceFrequencyDivider);
            break;
        case SELM__REFOCLK:
            CLKFrequency =
                (UCS_REFOCLK_FREQUENCY / CLKSourceFrequencyDivider);
            break;
        case SELM__XT2CLK:
            CLKFrequency =
                (UCS_XT2ClockFrequency / CLKSourceFrequencyDivider);

            if (HWREGB(baseAddress + OFS_UCSCTL7) & XT2OFFG){

              HWREGB(baseAddress + OFS_UCSCTL7) &=  ~XT2OFFG;
              //Clear OFIFG fault flag
              HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
            }

            if (HWREGB(baseAddress + OFS_UCSCTL7) & XT2OFFG){
                CLKFrequency =
                    privateUCSSourceClockFromDCO(baseAddress, SELM__DCOCLKDIV);
            }
            break;
        case SELM__DCOCLK:
        case SELM__DCOCLKDIV:
            CLKFrequency = privateUCSSourceClockFromDCO(baseAddress,
            CLKSource
            );
            break;
    }

    return ( CLKFrequency) ;
}

//******************************************************************************
//
//! Get the current ACLK frequency.  The user of this API must ensure that
//! UCS_externalClockSourceInit API was invoked before in case XT1 or XT2
//! is being used.
//!
//! \param baseAddress is the base address of the UCS module.
//!
//! \return Current ACLK frequency in Hz
//
//******************************************************************************
unsigned long
UCS_getACLK (unsigned int baseAddress)
{
    //Find ACLK source
    unsigned int ACLKSource = (HWREG(baseAddress + OFS_UCSCTL4) & SELA_7);

    ACLKSource = ACLKSource >> 8;

    unsigned int ACLKSourceDivider =  HWREG(baseAddress + OFS_UCSCTL5) & DIVA_7;
    ACLKSourceDivider = ACLKSourceDivider >> 8;

    return (privateUCSComputeCLKFrequency(baseAddress,
                ACLKSource,
                ACLKSourceDivider
                ));
}

//******************************************************************************
//
//! Get the current SMCLK frequency.  The user of this API must ensure that
//! UCS_externalClockSourceInit API was invoked before in case XT1 or XT2
//! is being used.
//!
//! \param baseAddress is the base address of the UCS module.
//!
//! \return Current SMCLK frequency in Hz
//
//******************************************************************************
unsigned long
UCS_getSMCLK (unsigned int baseAddress)
{
    unsigned int SMCLKSource = HWREGB(baseAddress + OFS_UCSCTL4_L) & SELS_7;

    SMCLKSource = SMCLKSource >> 4;

    unsigned int SMCLKSourceDivider =
        HWREG(baseAddress + OFS_UCSCTL5) & DIVS_7;
    SMCLKSourceDivider = SMCLKSourceDivider >> 4;

    return (privateUCSComputeCLKFrequency(baseAddress,
                SMCLKSource,
                SMCLKSourceDivider )
            );
}

//******************************************************************************
//
//! Get the current MCLK frequency. The user of this API must ensure that
//! UCS_externalClockSourceInit API was invoked before in case XT1 or XT2
//! is being used.
//!
//! \param baseAddress is the base address of the UCS module.
//!
//! \return Current MCLK frequency in Hz
//
//******************************************************************************
unsigned long
UCS_getMCLK (unsigned int baseAddress)
{
    //Find AMCLK source
    unsigned int MCLKSource = (HWREG(baseAddress + OFS_UCSCTL4) & SELM_7);

    unsigned int MCLKSourceDivider =  HWREG(baseAddress + OFS_UCSCTL5) & DIVM_7;

    return (privateUCSComputeCLKFrequency(baseAddress,
                MCLKSource,
                MCLKSourceDivider )
            );
}

//******************************************************************************
//
//! Clears all the Oscillator Flags
//!
//! \param baseAddress is the base address of the UCS module.
//! \param timeout is the count value that gets decremented every time the loop
//!         that clears oscillator fault flags gets executed.
//!
//! \return the mask of the oscillator flag status.
//
//******************************************************************************
unsigned int UCS_clearAllOscFlagsWithTimeout(unsigned int baseAddress, 
                                             unsigned int timeout
                                             )
{
    do {
      // Clear all osc fault flags
      HWREGB(baseAddress + OFS_UCSCTL7) &= ~(DCOFFG +
                                             XT1LFOFFG + 
                                             XT1HFOFFG + 
                                             XT2OFFG
                                             ); 



#if defined (__CC430F5133__) || (__CC430F5135__) || (__CC430F5137__) || \
            (__CC430F6125__) || (__CC430F6126__) || (__CC430F6127__) || \
            (__CC430F6135__) || (__CC430F6137__) || (__CC430F5123__) || \
            (__CC430F5125__) || (__CC430F5143__) || (__CC430F5145__) || \
            (__CC430F5147__) || (__CC430F6143__) || (__CC430F6145__) || \
            (__CC430F6147__)
        // CC430 uses a different fault mechanism. It requires 3 VLO clock 
        // cycles delay.If 20MHz CPU, 5000 clock cycles are required in worst 
        // case.
        __delay_cycles(5000);
#endif

      // Clear the global osc fault flag.
      HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1) &= ~OFIFG;
      
      // Check XT1 fault flags
    } while ((HWREGB(SFR_BASEADDRESS + OFS_SFRIFG1)) && --timeout);

    return (HWREGB(baseAddress + OFS_UCSCTL7) & (DCOFFG +
                                                 XT1LFOFFG + 
                                                 XT1HFOFFG + 
                                                 XT2OFFG)
                                                );
}
//******************************************************************************
//
//Close the Doxygen group.
//! @}
//
//******************************************************************************
