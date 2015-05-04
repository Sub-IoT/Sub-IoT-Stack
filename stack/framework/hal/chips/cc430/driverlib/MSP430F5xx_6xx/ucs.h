/* --COPYRIGHT--,BSD
 * Copyright (c) 2014, Texas Instruments Incorporated
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
// ucs.h - Driver for the UCS Module.
//
//*****************************************************************************

#ifndef __MSP430WARE_UCS_H__
#define __MSP430WARE_UCS_H__

#include "inc/hw_memmap.h"

#ifdef __MSP430_HAS_UCS_RF__ // TODO this was __MSP430_HAS_UCS__ originally, which is not defined for cc430 ...

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Internal very low power VLOCLK, low frequency oscillator with 10 kHz typical
// frequency
//
//*****************************************************************************
#define UCS_VLOCLK_FREQUENCY                                              10000

//*****************************************************************************
//
// Internal, trimmed, low-frequency oscillator with 32768 Hz typical frequency
//
//*****************************************************************************
#define UCS_REFOCLK_FREQUENCY                                             32768

//*****************************************************************************
//
// The following are values that can be passed to the clockSourceDivider
// parameter for functions: UCS_clockSignalInit().
//
//*****************************************************************************
#define UCS_CLOCK_DIVIDER_1                                             DIVM__1
#define UCS_CLOCK_DIVIDER_2                                             DIVM__2
#define UCS_CLOCK_DIVIDER_4                                             DIVM__4
#define UCS_CLOCK_DIVIDER_8                                             DIVM__8
#define UCS_CLOCK_DIVIDER_12                                           DIVM__32
#define UCS_CLOCK_DIVIDER_16                                           DIVM__16
#define UCS_CLOCK_DIVIDER_32                                           DIVM__32

//*****************************************************************************
//
// The following are values that can be passed to the selectedClockSignal
// parameter for functions: UCS_clockSignalInit().
//
//*****************************************************************************
#define UCS_ACLK                                                           0x01
#define UCS_MCLK                                                           0x02
#define UCS_SMCLK                                                          0x04
#define UCS_FLLREF                                                         0x08

//*****************************************************************************
//
// The following are values that can be passed to the clockSource parameter for
// functions: UCS_clockSignalInit().
//
//*****************************************************************************
#define UCS_XT1CLK_SELECT                                          SELM__XT1CLK
#define UCS_VLOCLK_SELECT                                          SELM__VLOCLK
#define UCS_REFOCLK_SELECT                                        SELM__REFOCLK
#define UCS_DCOCLK_SELECT                                          SELM__DCOCLK
#define UCS_DCOCLKDIV_SELECT                                    SELM__DCOCLKDIV
#define UCS_XT2CLK_SELECT                                          SELM__XT2CLK

//*****************************************************************************
//
// The following are values that can be passed to the xcap parameter for
// functions: UCS_LFXT1Start(), and UCS_LFXT1StartWithTimeout().
//
//*****************************************************************************
#define UCS_XCAP_0                                                       XCAP_0
#define UCS_XCAP_1                                                       XCAP_1
#define UCS_XCAP_2                                                       XCAP_2
#define UCS_XCAP_3                                                       XCAP_3

//*****************************************************************************
//
// The following are values that can be passed to the xt1drive parameter for
// functions: UCS_LFXT1Start(), UCS_HFXT1Start(), UCS_LFXT1StartWithTimeout(),
// and UCS_HFXT1StartWithTimeout().
//
//*****************************************************************************
#define UCS_XT1_DRIVE0                                               XT1DRIVE_0
#define UCS_XT1_DRIVE1                                               XT1DRIVE_1
#define UCS_XT1_DRIVE2                                               XT1DRIVE_2
#define UCS_XT1_DRIVE3                                               XT1DRIVE_3

//*****************************************************************************
//
// The following are values that can be passed to the highOrLowFrequency
// parameter for functions: UCS_bypassXT1(), and UCS_bypassXT1WithTimeout().
//
//*****************************************************************************
#define UCS_XT1_HIGH_FREQUENCY                                              XTS
#define UCS_XT1_LOW_FREQUENCY                                              0x00

//*****************************************************************************
//
// The following are values that can be passed to the xt2drive parameter for
// functions: UCS_XT2Start(), and UCS_XT2StartWithTimeout().
//
//*****************************************************************************
#define UCS_XT2DRIVE_4MHZ_8MHZ                                       XT2DRIVE_0
#define UCS_XT2DRIVE_8MHZ_16MHZ                                      XT2DRIVE_1
#define UCS_XT2DRIVE_16MHZ_24MHZ                                     XT2DRIVE_2
#define UCS_XT2DRIVE_24MHZ_32MHZ                                     XT2DRIVE_3

//*****************************************************************************
//
// The following are values that can be passed to the selectClock parameter for
// functions: UCS_enableClockRequest(), and UCS_disableClockRequest().
//
//*****************************************************************************
#define UCS_ACLK                                                           0x01
#define UCS_SMCLK                                                          0x04
#define UCS_MCLK                                                           0x02
#define UCS_MODOSC                                                  MODOSCREQEN

//*****************************************************************************
//
// The following are values that can be passed to the mask parameter for
// functions: UCS_faultFlagStatus(), and UCS_clearFaultFlag() as well as
// returned by the UCS_clearAllOscFlagsWithTimeout() function.
//
//*****************************************************************************
#define UCS_XT2OFFG                                                     XT2OFFG
#define UCS_XT1HFOFFG                                                 XT1HFOFFG
#define UCS_XT1LFOFFG                                                 XT1LFOFFG
#define UCS_DCOFFG                                                       DCOFFG

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************

//*****************************************************************************
//
//! \brief Sets the external clock source
//!
//! This function sets the external clock sources XT1 and XT2 crystal
//! oscillator frequency values. This function must be called if an external
//! crystal XT1 or XT2 is used and the user intends to call UCS_getMCLK,
//! UCS_getSMCLK or UCS_getACLK APIs. If not, it is not necessary to invoke
//! this API.
//!
//! \param XT1CLK_frequency is the XT1 crystal frequencies in Hz
//! \param XT2CLK_frequency is the XT2 crystal frequencies in Hz
//!
//! \return None
//
//*****************************************************************************
extern void UCS_setExternalClockSource(uint32_t XT1CLK_frequency,
                                       uint32_t XT2CLK_frequency);

//*****************************************************************************
//
//! \brief Initializes a clock signal
//!
//! This function initializes each of the clock signals. The user must ensure
//! that this function is called for each clock signal. If not, the default
//! state is assumed for the particular clock signal. Refer MSP430Ware
//! documentation for UCS module or Device Family User's Guide for details of
//! default clock signal states.
//!
//! \param selectedClockSignal selected clock signal
//!        Valid values are:
//!        - \b UCS_ACLK
//!        - \b UCS_MCLK
//!        - \b UCS_SMCLK
//!        - \b UCS_FLLREF
//! \param clockSource is clock source for the selectedClockSignal
//!        Valid values are:
//!        - \b UCS_XT1CLK_SELECT
//!        - \b UCS_VLOCLK_SELECT
//!        - \b UCS_REFOCLK_SELECT
//!        - \b UCS_DCOCLK_SELECT
//!        - \b UCS_DCOCLKDIV_SELECT
//!        - \b UCS_XT2CLK_SELECT
//! \param clockSourceDivider selected the clock divider to calculate
//!        clocksignal from clock source.
//!        Valid values are:
//!        - \b UCS_CLOCK_DIVIDER_1 [Default]
//!        - \b UCS_CLOCK_DIVIDER_2
//!        - \b UCS_CLOCK_DIVIDER_4
//!        - \b UCS_CLOCK_DIVIDER_8
//!        - \b UCS_CLOCK_DIVIDER_12 - [Valid only for UCS_FLLREF]
//!        - \b UCS_CLOCK_DIVIDER_16
//!        - \b UCS_CLOCK_DIVIDER_32 - [Not valid for UCS_FLLREF]
//!
//! Modified bits of \b UCSCTL5 register, bits of \b UCSCTL4 register and bits
//! of \b UCSCTL3 register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_clockSignalInit(uint8_t selectedClockSignal,
                                uint16_t clockSource,
                                uint16_t clockSourceDivider);

//*****************************************************************************
//
//! \brief Initializes the XT1 crystal oscillator in low frequency mode
//!
//! Initializes the XT1 crystal oscillator in low frequency mode. Loops until
//! all oscillator fault flags are cleared, with no timeout. See the device-
//! specific data sheet for appropriate drive settings.
//!
//! \param xt1drive is the target drive strength for the XT1 crystal
//!        oscillator.
//!        Valid values are:
//!        - \b UCS_XT1_DRIVE0
//!        - \b UCS_XT1_DRIVE1
//!        - \b UCS_XT1_DRIVE2
//!        - \b UCS_XT1_DRIVE3 [Default]
//!        \n Modified bits are \b XT1DRIVE of \b UCSCTL6 register.
//! \param xcap is the selected capacitor value. This parameter selects the
//!        capacitors applied to the LF crystal (XT1) or resonator in the LF
//!        mode. The effective capacitance (seen by the crystal) is Ceff. (CXIN
//!        + 2 pF)/2. It is assumed that CXIN = CXOUT and that a parasitic
//!        capacitance of 2 pF is added by the package and the printed circuit
//!        board. For details about the typical internal and the effective
//!        capacitors, refer to the device-specific data sheet.
//!        Valid values are:
//!        - \b UCS_XCAP_0
//!        - \b UCS_XCAP_1
//!        - \b UCS_XCAP_2
//!        - \b UCS_XCAP_3 [Default]
//!
//! Modified bits are \b XCAP of \b UCSCTL6 register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_LFXT1Start(uint16_t xt1drive,
                           uint8_t xcap);

//*****************************************************************************
//
//! \brief Initializes the XT1 crystal oscillator in low frequency mode
//!
//! Initializes the XT1 crystal oscillator in high frequency mode. Loops until
//! all oscillator fault flags are cleared, with no timeout. See the device-
//! specific data sheet for appropriate drive settings.
//!
//! \param xt1drive is the target drive strength for the XT1 crystal
//!        oscillator.
//!        Valid values are:
//!        - \b UCS_XT1_DRIVE0
//!        - \b UCS_XT1_DRIVE1
//!        - \b UCS_XT1_DRIVE2
//!        - \b UCS_XT1_DRIVE3 [Default]
//!
//! Modified bits of \b UCSCTL7 register, bits of \b UCSCTL6 register and bits
//! of \b SFRIFG register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_HFXT1Start(uint16_t xt1drive);

//*****************************************************************************
//
//! \brief Bypass the XT1 crystal oscillator
//!
//! Bypasses the XT1 crystal oscillator. Loops until all oscillator fault flags
//! are cleared, with no timeout.
//!
//! \param highOrLowFrequency selects high frequency or low frequency mode for
//!        XT1.
//!        Valid values are:
//!        - \b UCS_XT1_HIGH_FREQUENCY
//!        - \b UCS_XT1_LOW_FREQUENCY [Default]
//!
//! Modified bits of \b UCSCTL7 register, bits of \b UCSCTL6 register and bits
//! of \b SFRIFG register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_bypassXT1(uint8_t highOrLowFrequency);

//*****************************************************************************
//
//! \brief Initializes the XT1 crystal oscillator in low frequency mode with
//! timeout
//!
//! Initializes the XT1 crystal oscillator in low frequency mode with timeout.
//! Loops until all oscillator fault flags are cleared or until a timeout
//! counter is decremented and equals to zero. See the device-specific
//! datasheet for appropriate drive settings.
//!
//! \param xt1drive is the target drive strength for the XT1 crystal
//!        oscillator.
//!        Valid values are:
//!        - \b UCS_XT1_DRIVE0
//!        - \b UCS_XT1_DRIVE1
//!        - \b UCS_XT1_DRIVE2
//!        - \b UCS_XT1_DRIVE3 [Default]
//! \param xcap is the selected capacitor value. This parameter selects the
//!        capacitors applied to the LF crystal (XT1) or resonator in the LF
//!        mode. The effective capacitance (seen by the crystal) is Ceff. (CXIN
//!        + 2 pF)/2. It is assumed that CXIN = CXOUT and that a parasitic
//!        capacitance of 2 pF is added by the package and the printed circuit
//!        board. For details about the typical internal and the effective
//!        capacitors, refer to the device-specific data sheet.
//!        Valid values are:
//!        - \b UCS_XCAP_0
//!        - \b UCS_XCAP_1
//!        - \b UCS_XCAP_2
//!        - \b UCS_XCAP_3 [Default]
//! \param timeout is the count value that gets decremented every time the loop
//!        that clears oscillator fault flags gets executed.
//!
//! Modified bits of \b UCSCTL7 register, bits of \b UCSCTL6 register and bits
//! of \b SFRIFG register.
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//*****************************************************************************
extern bool UCS_LFXT1StartWithTimeout(uint16_t xt1drive,
                                      uint8_t xcap,
                                      uint16_t timeout);

//*****************************************************************************
//
//! \brief Initializes the XT1 crystal oscillator in high frequency mode with
//! timeout
//!
//! Initializes the XT1 crystal oscillator in high frequency mode with timeout.
//! Loops until all oscillator fault flags are cleared or until a timeout
//! counter is decremented and equals to zero. See the device-specific data
//! sheet for appropriate drive settings.
//!
//! \param xt1drive is the target drive strength for the XT1 crystal
//!        oscillator.
//!        Valid values are:
//!        - \b UCS_XT1_DRIVE0
//!        - \b UCS_XT1_DRIVE1
//!        - \b UCS_XT1_DRIVE2
//!        - \b UCS_XT1_DRIVE3 [Default]
//! \param timeout is the count value that gets decremented every time the loop
//!        that clears oscillator fault flags gets executed.
//!
//! Modified bits of \b UCSCTL7 register, bits of \b UCSCTL6 register and bits
//! of \b SFRIFG register.
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//*****************************************************************************
extern bool UCS_HFXT1StartWithTimeout(uint16_t xt1drive,
                                      uint16_t timeout);

//*****************************************************************************
//
//! \brief Bypasses the XT1 crystal oscillator with time out
//!
//! Bypasses the XT1 crystal oscillator with time out. Loops until all
//! oscillator fault flags are cleared or until a timeout counter is
//! decremented and equals to zero.
//!
//! \param highOrLowFrequency selects high frequency or low frequency mode for
//!        XT1.
//!        Valid values are:
//!        - \b UCS_XT1_HIGH_FREQUENCY
//!        - \b UCS_XT1_LOW_FREQUENCY [Default]
//! \param timeout is the count value that gets decremented every time the loop
//!        that clears oscillator fault flags gets executed.
//!
//! Modified bits of \b UCSCTL7 register, bits of \b UCSCTL6 register and bits
//! of \b SFRIFG register.
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//*****************************************************************************
extern bool UCS_bypassXT1WithTimeout(uint8_t highOrLowFrequency,
                                     uint16_t timeout);

//*****************************************************************************
//
//! \brief Stops the XT1 oscillator using the XT1OFF bit.
//!
//!
//! \return None
//
//*****************************************************************************
extern void UCS_XT1Off(void);

//*****************************************************************************
//
//! \brief Initializes the XT2 crystal oscillator
//!
//! Initializes the XT2 crystal oscillator, which supports crystal frequencies
//! between 4 MHz and 32 MHz, depending on the selected drive strength. Loops
//! until all oscillator fault flags are cleared, with no timeout. See the
//! device-specific data sheet for appropriate drive settings.
//!
//! \param xt2drive is the target drive strength for the XT2 crystal
//!        oscillator.
//!        Valid values are:
//!        - \b UCS_XT2DRIVE_4MHZ_8MHZ
//!        - \b UCS_XT2DRIVE_8MHZ_16MHZ
//!        - \b UCS_XT2DRIVE_16MHZ_24MHZ
//!        - \b UCS_XT2DRIVE_24MHZ_32MHZ [Default]
//!
//! Modified bits of \b UCSCTL7 register, bits of \b UCSCTL6 register and bits
//! of \b SFRIFG register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_XT2Start(uint16_t xt2drive);

//*****************************************************************************
//
//! \brief Bypasses the XT2 crystal oscillator
//!
//! Bypasses the XT2 crystal oscillator, which supports crystal frequencies
//! between 4 MHz and 32 MHz. Loops until all oscillator fault flags are
//! cleared, with no timeout.
//!
//!
//! Modified bits of \b UCSCTL7 register, bits of \b UCSCTL6 register and bits
//! of \b SFRIFG register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_bypassXT2(void);

//*****************************************************************************
//
//! \brief Initializes the XT2 crystal oscillator with timeout
//!
//! Initializes the XT2 crystal oscillator, which supports crystal frequencies
//! between 4 MHz and 32 MHz, depending on the selected drive strength. Loops
//! until all oscillator fault flags are cleared or until a timeout counter is
//! decremented and equals to zero. See the device-specific data sheet for
//! appropriate drive settings.
//!
//! \param xt2drive is the target drive strength for the XT2 crystal
//!        oscillator.
//!        Valid values are:
//!        - \b UCS_XT2DRIVE_4MHZ_8MHZ
//!        - \b UCS_XT2DRIVE_8MHZ_16MHZ
//!        - \b UCS_XT2DRIVE_16MHZ_24MHZ
//!        - \b UCS_XT2DRIVE_24MHZ_32MHZ [Default]
//! \param timeout is the count value that gets decremented every time the loop
//!        that clears oscillator fault flags gets executed.
//!
//! Modified bits of \b UCSCTL7 register, bits of \b UCSCTL6 register and bits
//! of \b SFRIFG register.
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//*****************************************************************************
extern bool UCS_XT2StartWithTimeout(uint16_t xt2drive,
                                    uint16_t timeout);

//*****************************************************************************
//
//! \brief Bypasses the XT2 crystal oscillator with timeout
//!
//! Bypasses the XT2 crystal oscillator, which supports crystal frequencies
//! between 4 MHz and 32 MHz. Loops until all oscillator fault flags are
//! cleared or until a timeout counter is decremented and equals to zero.
//!
//! \param timeout is the count value that gets decremented every time the loop
//!        that clears oscillator fault flags gets executed.
//!
//! Modified bits of \b UCSCTL7 register, bits of \b UCSCTL6 register and bits
//! of \b SFRIFG register.
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//*****************************************************************************
extern bool UCS_bypassXT2WithTimeout(uint16_t timeout);

//*****************************************************************************
//
//! \brief Stops the XT2 oscillator using the XT2OFF bit.
//!
//!
//! Modified bits of \b UCSCTL6 register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_XT2Off(void);

//*****************************************************************************
//
//! \brief Initializes the DCO to operate a frequency that is a multiple of the
//! reference frequency into the FLL
//!
//! Initializes the DCO to operate a frequency that is a multiple of the
//! reference frequency into the FLL. Loops until all oscillator fault flags
//! are cleared, with a timeout. If the frequency is greater than 16 MHz, the
//! function sets the MCLK and SMCLK source to the undivided DCO frequency.
//! Otherwise, the function sets the MCLK and SMCLK source to the DCOCLKDIV
//! frequency. This function executes a software delay that is proportional in
//! length to the ratio of the target FLL frequency and the FLL reference.
//!
//! \param fsystem is the target frequency for MCLK in kHz
//! \param ratio is the ratio x/y, where x = fsystem and y = FLL reference
//!        frequency.
//!
//! Modified bits of \b UCSCTL0 register, bits of \b UCSCTL4 register, bits of
//! \b UCSCTL7 register, bits of \b UCSCTL1 register, bits of \b SFRIFG1
//! register and bits of \b UCSCTL2 register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_initFLLSettle(uint16_t fsystem,
                              uint16_t ratio);

//*****************************************************************************
//
//! \brief Initializes the DCO to operate a frequency that is a multiple of the
//! reference frequency into the FLL
//!
//! Initializes the DCO to operate a frequency that is a multiple of the
//! reference frequency into the FLL. Loops until all oscillator fault flags
//! are cleared, with no timeout. If the frequency is greater than 16 MHz, the
//! function sets the MCLK and SMCLK source to the undivided DCO frequency.
//! Otherwise, the function sets the MCLK and SMCLK source to the DCOCLKDIV
//! frequency.
//!
//! \param fsystem is the target frequency for MCLK in kHz
//! \param ratio is the ratio x/y, where x = fsystem and y = FLL reference
//!        frequency.
//!
//! Modified bits of \b UCSCTL0 register, bits of \b UCSCTL4 register, bits of
//! \b UCSCTL7 register, bits of \b UCSCTL1 register, bits of \b SFRIFG1
//! register and bits of \b UCSCTL2 register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_initFLL(uint16_t fsystem,
                        uint16_t ratio);

//*****************************************************************************
//
//! \brief Enables conditional module requests
//!
//! \param selectClock selects specific request enables
//!        Valid values are:
//!        - \b UCS_ACLK
//!        - \b UCS_SMCLK
//!        - \b UCS_MCLK
//!        - \b UCS_MODOSC
//!
//! Modified bits of \b UCSCTL8 register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_enableClockRequest(uint8_t selectClock);

//*****************************************************************************
//
//! \brief Disables conditional module requests
//!
//! \param selectClock selects specific request disable
//!        Valid values are:
//!        - \b UCS_ACLK
//!        - \b UCS_SMCLK
//!        - \b UCS_MCLK
//!        - \b UCS_MODOSC
//!
//! Modified bits of \b UCSCTL8 register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_disableClockRequest(uint8_t selectClock);

//*****************************************************************************
//
//! \brief Gets the current UCS fault flag status.
//!
//! \param mask is the masked interrupt flag status to be returned. Mask
//!        parameter can be either any of the following selection.
//!        Valid values are:
//!        - \b UCS_XT2OFFG - XT2 oscillator fault flag
//!        - \b UCS_XT1HFOFFG - XT1 oscillator fault flag (HF mode)
//!        - \b UCS_XT1LFOFFG - XT1 oscillator fault flag (LF mode)
//!        - \b UCS_DCOFFG - DCO fault flag
//!
//
//*****************************************************************************
extern uint8_t UCS_faultFlagStatus(uint8_t mask);

//*****************************************************************************
//
//! \brief Clears the current UCS fault flag status for the masked bit.
//!
//! \param mask is the masked interrupt flag status to be returned. mask
//!        parameter can be any one of the following
//!        Valid values are:
//!        - \b UCS_XT2OFFG - XT2 oscillator fault flag
//!        - \b UCS_XT1HFOFFG - XT1 oscillator fault flag (HF mode)
//!        - \b UCS_XT1LFOFFG - XT1 oscillator fault flag (LF mode)
//!        - \b UCS_DCOFFG - DCO fault flag
//!
//! Modified bits of \b UCSCTL7 register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_clearFaultFlag(uint8_t mask);

//*****************************************************************************
//
//! \brief Turns off SMCLK using the SMCLKOFF bit
//!
//!
//! Modified bits of \b UCSCTL6 register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_SMCLKOff(void);

//*****************************************************************************
//
//! \brief Turns ON SMCLK using the SMCLKOFF bit
//!
//!
//! Modified bits of \b UCSCTL6 register.
//!
//! \return None
//
//*****************************************************************************
extern void UCS_SMCLKOn(void);

//*****************************************************************************
//
//! \brief Get the current ACLK frequency
//!
//! Get the current ACLK frequency. The user of this API must ensure that
//! UCS_setExternalClockSource API was invoked before in case XT1 or XT2 is
//! being used.
//!
//!
//! \return Current ACLK frequency in Hz
//
//*****************************************************************************
extern uint32_t UCS_getACLK(void);

//*****************************************************************************
//
//! \brief Get the current SMCLK frequency
//!
//! Get the current SMCLK frequency. The user of this API must ensure that
//! UCS_setExternalClockSource API was invoked before in case XT1 or XT2 is
//! being used.
//!
//!
//! \return Current SMCLK frequency in Hz
//
//*****************************************************************************
extern uint32_t UCS_getSMCLK(void);

//*****************************************************************************
//
//! \brief Get the current MCLK frequency
//!
//! Get the current MCLK frequency. The user of this API must ensure that
//! UCS_setExternalClockSource API was invoked before in case XT1 or XT2 is
//! being used.
//!
//!
//! \return Current MCLK frequency in Hz
//
//*****************************************************************************
extern uint32_t UCS_getMCLK(void);

//*****************************************************************************
//
//! \brief Clears all the Oscillator Flags
//!
//! \param timeout is the count value that gets decremented every time the loop
//!        that clears oscillator fault flags gets executed.
//!
//! \return Logical OR of any of the following:
//!         - \b UCS_XT2OFFG XT2 oscillator fault flag
//!         - \b UCS_XT1HFOFFG XT1 oscillator fault flag (HF mode)
//!         - \b UCS_XT1LFOFFG XT1 oscillator fault flag (LF mode)
//!         - \b UCS_DCOFFG DCO fault flag
//!         \n indicating the status of the oscillator fault flags
//
//*****************************************************************************
extern uint16_t UCS_clearAllOscFlagsWithTimeout(uint16_t timeout);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif
#endif // __MSP430WARE_UCS_H__
