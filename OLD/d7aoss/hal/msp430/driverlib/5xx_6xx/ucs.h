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
#ifndef __MSP430WARE_UCS_H__
#define __MSP430WARE_UCS_H__

//*****************************************************************************
//
//The following are the defines to include the required modules for this
//peripheral in msp430xgeneric.h file
//
//*****************************************************************************
#define __MSP430_HAS_SFR__
#define __MSP430_HAS_UCS__

//*****************************************************************************
//
//The following are values that can be passed to the UCS_clockSignalInit() API
//as the selectedClockSignal parameter.
//
//*****************************************************************************
#define UCS_ACLK                0x01
#define UCS_MCLK                0x02
#define UCS_SMCLK               0x04
#define UCS_FLLREF              0x08

//*****************************************************************************
//
//The following along with UCS_ACLK, UCS_MCLK, UCS_SMCLK may be passed to the
//UCS_clockRequestEnable() and UCS_clockRequestDisable() API
//as the selectClock parameter.
//
//*****************************************************************************
#define UCS_MODOSC              MODOSCREQEN

//*****************************************************************************
//
//The following are values that can be passed to the UCS_clockSignalInit() API
//as the clockSource parameter. UCS_VLOCLK_SELECT may not be used for
//selectedClockSignal UCS_FLLREF
//
//*****************************************************************************
#define UCS_XT1CLK_SELECT        SELM__XT1CLK
#define UCS_VLOCLK_SELECT        SELM__VLOCLK
#define UCS_REFOCLK_SELECT       SELM__REFOCLK
#define UCS_DCOCLK_SELECT        SELM__DCOCLK
#define UCS_DCOCLKDIV_SELECT     SELM__DCOCLKDIV
#define UCS_XT2CLK_SELECT        SELM__XT2CLK

//*****************************************************************************
//
//The following are values that can be passed to the UCS_clockSignalInit() API
//as the clockSourceDivider parameter.
//
//*****************************************************************************
#define UCS_CLOCK_DIVIDER_1     DIVM__1
#define UCS_CLOCK_DIVIDER_2     DIVM__2
#define UCS_CLOCK_DIVIDER_4     DIVM__4
#define UCS_CLOCK_DIVIDER_8     DIVM__8
#define UCS_CLOCK_DIVIDER_12    DIVM__32
#define UCS_CLOCK_DIVIDER_16    DIVM__16
#define UCS_CLOCK_DIVIDER_32    DIVM__32

//*****************************************************************************
//
//The following are values that can be passed to the UCS_LFXT1Start(),
//UCS_HFXT1Start(), UCS_LFXT1StartWithTimeout(), UCS_HFXT1StartWithTimeout()
//as the xt1drive parameter.
//
//*****************************************************************************
#define UCS_XT1_DRIVE0  XT1DRIVE_0
#define UCS_XT1_DRIVE1  XT1DRIVE_1
#define UCS_XT1_DRIVE2  XT1DRIVE_2
#define UCS_XT1_DRIVE3  XT1DRIVE_3

//*****************************************************************************
//
//The following are values that can be passed to the UCS_XT1_Start() API
//as the highOrLowFrequency parameter.
//
//*****************************************************************************
#define UCS_XT1_HIGH_FREQUENCY  XTS
#define UCS_XT1_LOW_FREQUENCY   0x00

//*****************************************************************************
//
//The following are values that can be passed to the UCS_XT2_Start() API
//as the xt2drive parameter.
//
//*****************************************************************************
#define UCS_XT2DRIVE_4MHZ_8MHZ       XT2DRIVE_0
#define UCS_XT2DRIVE_8MHZ_16MHZ      XT2DRIVE_1
#define UCS_XT2DRIVE_16MHZ_24MHZ     XT2DRIVE_2
#define UCS_XT2DRIVE_24MHZ_32MHZ     XT2DRIVE_3

//*****************************************************************************
//
//The following are values that can be passed to the UCS_faultFlagStatus() and
//UCS_clearFaultFlag API as the mask parameter.
//
//*****************************************************************************
#define UCS_XT2OFFG     XT2OFFG
#define UCS_XT1HFOFFG   XT1HFOFFG
#define UCS_XT1LFOFFG   XT1LFOFFG
#define UCS_DCOFFG      DCOFFG

//*****************************************************************************
//
//The following are values that can be passed to the UCS_LFXT1Start() and
//UCS_LFXT1StartWithTimeout() API as the xcap parameter.
//
//*****************************************************************************
#define UCS_XCAP_0  XCAP_0
#define UCS_XCAP_1  XCAP_1
#define UCS_XCAP_2  XCAP_2
#define UCS_XCAP_3  XCAP_3

//*****************************************************************************
//
//Internal very low power VLOCLK, low frequency oscillator with
//10 kHz typical frequency
//
//*****************************************************************************
#define UCS_VLOCLK_FREQUENCY    10000

//*****************************************************************************
//
//Internal, trimmed, low-frequency oscillator with 32768 Hz typical frequency
//
//*****************************************************************************
#define UCS_REFOCLK_FREQUENCY   32768

//*****************************************************************************
//
//Prototypes for the APIs.
//
//*****************************************************************************
extern void
UCS_setExternalClockSource (unsigned int baseaddress,
    unsigned long XT1CLK_frequency,
    unsigned long XT2CLK_frequency
    );
extern void
UCS_clockSignalInit ( unsigned int baseaddress,
    unsigned char selectedClockSignal,
    unsigned int clockSource,
    unsigned char clockSourceDivider
    );
extern void
UCS_LFXT1Start ( unsigned int baseAddress,
    unsigned int xt1drive,
    unsigned char xcap
    );
extern void
UCS_HFXT1Start (
    unsigned int baseAddress,
    unsigned int xt1drive
    );
extern void
UCS_bypassXT1 ( unsigned int baseAddress,
    unsigned char highOrLowFrequency
    );
extern unsigned short
UCS_bypassXT1WithTimeout (
    unsigned int baseAddress,
    unsigned char highOrLowFrequency,
    unsigned int timeout
    );
extern void
UCS_XT1Off (unsigned int baseAddress);

extern unsigned short
UCS_LFXT1StartWithTimeout (
    unsigned int baseAddress,
    unsigned int xt1drive,
    unsigned char xcap,
    unsigned int timeout
    );
extern unsigned short
UCS_HFXT1StartWithTimeout (  unsigned int baseAddress,
    unsigned int xt1drive,
    unsigned int timeout
    );
/*
extern void UCS_XT2Start (  unsigned int baseAddress,
    unsigned int xt2drive
    );
extern void UCS_bypassXT2 (  unsigned int baseAddress );

extern unsigned short
UCS_XT2StartWithTimeout ( unsigned int baseAddress,
    unsigned int xt2drive,
    unsigned int timeout
    );
extern unsigned short
UCS_bypassXT2WithTimeout ( unsigned int baseAddress,
    unsigned int timeout
    );
extern void
UCS_XT2Off (unsigned int baseAddress);

extern void
UCS_XT2Start (  unsigned int baseAddress,
    unsigned int xt2drive
    );
 */
extern void
UCS_initFLL ( unsigned int baseAddress,
    unsigned int fsystem,
    unsigned int ratio
    );
extern void UCS_initFLLSettle (    unsigned int baseAddress,
    unsigned int fsystem,
    unsigned int ratio
    );

extern void UCS_enableClockRequest (    unsigned int baseAddress,
    unsigned char selectClock
    );
extern void UCS_disableClockRequest (
    unsigned int baseAddress,
    unsigned char selectClock
    );
extern unsigned char UCS_faultFlagStatus (
    unsigned int baseAddress,
    unsigned char mask
    );

extern void UCS_clearFaultFlag (
    unsigned int baseAddress,
    unsigned char mask
    );
extern void UCS_SMCLKOff (unsigned int baseAddress);

extern void UCS_SMCLKOn (unsigned int baseAddress);
extern unsigned long UCS_getACLK (unsigned int baseAddress);
extern unsigned long UCS_getSMCLK (unsigned int baseAddress);
extern unsigned long UCS_getMCLK (unsigned int baseAddress);
extern unsigned int UCS_clearAllOscFlagsWithTimeout(unsigned int baseAddress, 
                                             unsigned int timeout);
       

#endif
