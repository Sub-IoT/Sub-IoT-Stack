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
//pmm.c - Driver for the PMM Module.
//
//*****************************************************************************
#include "../inc/hw_types.h"
#include "debug.h"
#include "pmm.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "../deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif

//*****************************************************************************
//
//! Enables the low-side SVS circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_enableSvsL (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) |= SVSLE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables the low-side SVS circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_disableSvsL (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) &= ~SVSLE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables the low-side SVM circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_enableSvmL (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) |= SVMLE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables the low-side SVM circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_disableSvmL (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) &= ~SVMLE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables the high-side SVS circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMHCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_enableSvsH (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) |= SVSHE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables the high-side SVS circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMHCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_disableSvsH (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) &= ~SVSHE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables the high-side SVM circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMHCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_enableSvmH (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) |= SVMHE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables the high-side SVM circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMHCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_disableSvmH (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) &= ~SVMHE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables the low-side SVS and SVM circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_enableSvsLSvmL (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) |= (SVSLE + SVMLE);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables the low-side SVS and SVM circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_disableSvsLSvmL (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) &= ~(SVSLE + SVMLE);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables the high-side SVS and SVM circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMHCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_enableSvsHSvmH (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) |= (SVSHE + SVMHE);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables the high-side SVS and SVM circuitry
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMHCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_disableSvsHSvmH (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) &= ~(SVSHE + SVMHE);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables the POR signal generation when a low-voltage event is
//! registered by the low-side SVS
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_enableSvsLReset (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_PMMRIE) |= SVSLPE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables the POR signal generation when a low-voltage event is
//! registered by the low-side SVS
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_disableSvsLReset (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_PMMRIE) &= ~SVSLPE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables the interrupt generation when a low-voltage event is
//! registered by the low-side SVM
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_enableSvmLInterrupt (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_PMMRIE) |= SVMLIE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables the interrupt generation when a low-voltage event is
//! registered by the low-side SVM
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_disableSvmLInterrupt (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_PMMRIE) &= ~SVMLIE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables the POR signal generation when a low-voltage event is
//! registered by the high-side SVS
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_enableSvsHReset (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_PMMRIE) |= SVSHPE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables the POR signal generation when a low-voltage event is
//! registered by the high-side SVS
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_disableSvsHReset (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_PMMRIE) &= ~SVSHPE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables the interrupt generation when a low-voltage event is
//! registered by the high-side SVM
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_enableSvmHInterrupt (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_PMMRIE) |= SVMHIE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables the interrupt generation when a low-voltage event is
//! registered by the high-side SVM
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_disableSvmHInterrupt (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_PMMRIE) &= ~SVMHIE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Clear all interrupt flags for the PMM
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b PMMIFG.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_clearPMMIFGS (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREGB(baseAddress + OFS_PMMIFG) = 0;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables supervisor low side in LPM with twake-up-fast from LPM2, LPM3,
//! and LPM4
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************

void PMM_SvsLEnabledInLPMFastWake (unsigned int baseAddress)
{
    //These settings use SVSH/LACE = 0
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) |= (SVSLFP + SVSLMD);
    HWREG(baseAddress + OFS_SVSMLCTL) &= ~SVSMLACE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables supervisor low side in LPM with twake-up-slow  from LPM2, LPM3,
//! and LPM4
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_SvsLEnabledInLPMSlowWake (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) |= SVSLMD;
    HWREG(baseAddress + OFS_SVSMLCTL) &= ~(SVSLFP + SVSMLACE);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables supervisor low side in LPM with twake-up-fast from LPM2, LPM3,
//! and LPM4
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_SvsLDisabledInLPMFastWake (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) |= SVSLFP;
    HWREG(baseAddress + OFS_SVSMLCTL) &= ~(SVSLMD + SVSMLACE);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables supervisor low side in LPM with twake-up-slow from LPM2, LPM3,
//! and LPM4
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_SvsLDisabledInLPMSlowWake (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) &= ~(SVSLFP + SVSMLACE + SVSLMD);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables supervisor high side in LPM with tpd = 20 �s(1)
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMHCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_SvsHEnabledInLPMNormPerf (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) |= SVSHMD;
    HWREG(baseAddress + OFS_SVSMHCTL) &= ~(SVSMHACE + SVSHFP);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Enables supervisor high side in LPM with tpd = 2.5 �s(1)
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMHCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_SvsHEnabledInLPMFullPerf (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) |= (SVSHMD + SVSHFP);
    HWREG(baseAddress + OFS_SVSMHCTL) &= ~SVSMHACE;
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables supervisor high side in LPM with tpd = 20 �s(1)
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMHCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_SvsHDisabledInLPMNormPerf (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) &= ~(SVSMHACE + SVSHFP + SVSHMD);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Disables supervisor high side in LPM with tpd = 2.5 �s(1)
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMHCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_SvsHDisabledInLPMFullPerf (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) |= SVSHFP;
    HWREG(baseAddress + OFS_SVSMHCTL) &= ~(SVSMHACE + SVSHMD);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Optimized to provide twake-up-fast from LPM2, LPM3, and LPM4 with least
//! power
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_SvsLOptimizedInLPMFastWake (unsigned int baseAddress)
{
    //These setting use SVSH/LACE = 1
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMLCTL) |= (SVSLFP + SVSLMD + SVSMLACE);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Optimized to provide tpd = 2.5 �s(1) in LPM with least power
//!
//! \param baseAddress is the base address of the PMM module.
//!
//! Modified registers are \b PMMCTL0, \b SVSMLCTL.
//!
//! \return NONE
//
//*****************************************************************************
void PMM_SvsHOptimizedInLPMFullPerf (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;
    HWREG(baseAddress + OFS_SVSMHCTL) |= (SVSHMD + SVSHFP + SVSMHACE);
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
}

//*****************************************************************************
//
//! Increase Vcore by one level
//!
//! \param baseAddress is the base address of the I2C module.
//! \param level level to which Vcore needs to be increased
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE, \b PMMIFG, \b SVSMLCTL,
//! \b SVSMHCTL.
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//*****************************************************************************
unsigned int PMM_setVCoreUp (unsigned int baseAddress, unsigned char level)
{
    unsigned long PMMRIE_backup, SVSMHCTL_backup, SVSMLCTL_backup;

    //The code flow for increasing the Vcore has been altered to work around
    //the erratum FLASH37.
    //Please refer to the Errata sheet to know if a specific device is affected
    //DO NOT ALTER THIS FUNCTION

    //Open PMM registers for write access
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;

    //Disable dedicated Interrupts
    //Backup all registers
    PMMRIE_backup = HWREGB(baseAddress + OFS_PMMRIE);
    HWREGB(baseAddress + OFS_PMMRIE) &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE |
                                          SVSLPE | SVMHVLRIE | SVMHIE |
                                          SVSMHDLYIE | SVMLVLRIE | SVMLIE |
                                          SVSMLDLYIE
                                          );
    SVSMHCTL_backup = HWREGB(baseAddress + OFS_SVSMHCTL);
    SVSMLCTL_backup = HWREGB(baseAddress + OFS_SVSMLCTL);

    //Clear flags
    HWREGB(baseAddress + OFS_PMMIFG) = 0;

    //Set SVM highside to new level and check if a VCore increase is possible
    HWREGB(baseAddress + OFS_SVSMHCTL) = SVMHE | SVSHE | (SVSMHRRL0 * level);

    //Wait until SVM highside is settled
    while ((HWREGB(baseAddress + OFS_PMMIFG) & SVSMHDLYIFG) == 0) ;

    //Clear flag
    HWREGB(baseAddress + OFS_PMMIFG) &= ~SVSMHDLYIFG;

    //Check if a VCore increase is possible
    if ((HWREGB(baseAddress + OFS_PMMIFG) & SVMHIFG) == SVMHIFG){
        //-> Vcc is too low for a Vcore increase
        //recover the previous settings
        HWREGB(baseAddress + OFS_PMMIFG) &= ~SVSMHDLYIFG;
        HWREGB(baseAddress + OFS_SVSMHCTL) = SVSMHCTL_backup;

        //Wait until SVM highside is settled
        while ((HWREGB(baseAddress + OFS_PMMIFG) & SVSMHDLYIFG) == 0) ;

        //Clear all Flags
        HWREGB(baseAddress +
            OFS_PMMIFG) &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG |
                             SVMLVLRIFG | SVMLIFG |
                             SVSMLDLYIFG
                             );

        //Restore PMM interrupt enable register
        HWREGB(baseAddress + OFS_PMMRIE) = PMMRIE_backup;
        //Lock PMM registers for write access
        HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
        //return: voltage not set
        return ( STATUS_FAIL) ;
    }

    //Set also SVS highside to new level
    //Vcc is high enough for a Vcore increase
    HWREGB(baseAddress + OFS_SVSMHCTL) |= (SVSHRVL0 * level);

    //Wait until SVM highside is settled
    while ((HWREGB(baseAddress + OFS_PMMIFG) & SVSMHDLYIFG) == 0) ;

    //Clear flag
    HWREGB(baseAddress + OFS_PMMIFG) &= ~SVSMHDLYIFG;

    //Set VCore to new level
    HWREGB(baseAddress + OFS_PMMCTL0_L) = PMMCOREV0 * level;

    //Set SVM, SVS low side to new level
    HWREGB(baseAddress + OFS_SVSMLCTL) = SVMLE | (SVSMLRRL0 * level) |
                                         SVSLE | (SVSLRVL0 * level);

    //Wait until SVM, SVS low side is settled
    while ((HWREGB(baseAddress + OFS_PMMIFG) & SVSMLDLYIFG) == 0) ;

    //Clear flag
    HWREGB(baseAddress + OFS_PMMIFG) &= ~SVSMLDLYIFG;
    //SVS, SVM core and high side are now set to protect for the new core level

    //Restore Low side settings
    //Clear all other bits _except_ level settings
    HWREGB(baseAddress + OFS_SVSMLCTL) &= (SVSLRVL0 + SVSLRVL1 + SVSMLRRL0 +
                                           SVSMLRRL1 + SVSMLRRL2
                                           );

    //Clear level settings in the backup register,keep all other bits
    SVSMLCTL_backup &=
        ~(SVSLRVL0 + SVSLRVL1 + SVSMLRRL0 + SVSMLRRL1 + SVSMLRRL2);

    //Restore low-side SVS monitor settings
    HWREGB(baseAddress + OFS_SVSMLCTL) |= SVSMLCTL_backup;

    //Restore High side settings
    //Clear all other bits except level settings
    HWREGB(baseAddress + OFS_SVSMHCTL) &= (SVSHRVL0 + SVSHRVL1 +
                                           SVSMHRRL0 + SVSMHRRL1 +
                                           SVSMHRRL2
                                           );

    //Clear level settings in the backup register,keep all other bits
    SVSMHCTL_backup &=
        ~(SVSHRVL0 + SVSHRVL1 + SVSMHRRL0 + SVSMHRRL1 + SVSMHRRL2);

    //Restore backup
    HWREGB(baseAddress + OFS_SVSMHCTL) |= SVSMHCTL_backup;

    //Wait until high side, low side settled
    while (((HWREGB(baseAddress + OFS_PMMIFG) & SVSMLDLYIFG) == 0) &&
           ((HWREGB(baseAddress + OFS_PMMIFG) & SVSMHDLYIFG) == 0)) ;

    //Clear all Flags
    HWREGB(baseAddress + OFS_PMMIFG) &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG |
                                          SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG
                                          );

    //Restore PMM interrupt enable register
    HWREGB(baseAddress + OFS_PMMRIE) = PMMRIE_backup;

    //Lock PMM registers for write access
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;

    return ( STATUS_SUCCESS) ;
}

//*****************************************************************************
//
//! Decrease Vcore by one level
//!
//! \param baseAddress is the base address of the I2C module.
//! \param level level to which Vcore needs to be decreased
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE, \b PMMIFG, \b SVSMLCTL,
//! \b SVSMHCTL.
//!
//! \return STATUS_SUCCESS
//
//*****************************************************************************
unsigned int PMM_setVCoreDown (unsigned int baseAddress, unsigned char level)
{
    unsigned long PMMRIE_backup, SVSMHCTL_backup, SVSMLCTL_backup;

    //The code flow for decreasing the Vcore has been altered to work around
    //the erratum FLASH37.
    //Please refer to the Errata sheet to know if a specific device is affected
    //DO NOT ALTER THIS FUNCTION

    //Open PMM registers for write access
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0xA5;

    //Disable dedicated Interrupts
    //Backup all registers
    PMMRIE_backup = HWREGB(baseAddress + OFS_PMMRIE);
    HWREGB(baseAddress + OFS_PMMRIE) &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE |
                                          SVSLPE | SVMHVLRIE | SVMHIE |
                                          SVSMHDLYIE | SVMLVLRIE | SVMLIE |
                                          SVSMLDLYIE
                                          );
    SVSMHCTL_backup = HWREGB(baseAddress + OFS_SVSMHCTL);
    SVSMLCTL_backup = HWREGB(baseAddress + OFS_SVSMLCTL);

    //Clear flags
    HWREGB(baseAddress + OFS_PMMIFG) &= ~(SVMHIFG | SVSMHDLYIFG |
                                          SVMLIFG | SVSMLDLYIFG
                                          );

    //Set SVM, SVS high & low side to new settings in normal mode
    HWREGB(baseAddress + OFS_SVSMHCTL) = SVMHE | (SVSMHRRL0 * level) |
                                         SVSHE | (SVSHRVL0 * level);
    HWREGB(baseAddress + OFS_SVSMLCTL) = SVMLE | (SVSMLRRL0 * level) |
                                         SVSLE | (SVSLRVL0 * level);

    //Wait until SVM high side and SVM low side is settled
    while ((HWREGB(baseAddress + OFS_PMMIFG) & SVSMHDLYIFG) == 0 ||
           (HWREGB(baseAddress + OFS_PMMIFG) & SVSMLDLYIFG) == 0) ;

    //Clear flags
    HWREGB(baseAddress + OFS_PMMIFG) &= ~(SVSMHDLYIFG + SVSMLDLYIFG);
    //SVS, SVM core and high side are now set to protect for the new core level

    //Set VCore to new level
    HWREGB(baseAddress + OFS_PMMCTL0_L) = PMMCOREV0 * level;

    //Restore Low side settings
    //Clear all other bits _except_ level settings
    HWREGB(baseAddress + OFS_SVSMLCTL) &= (SVSLRVL0 + SVSLRVL1 + SVSMLRRL0 +
                                           SVSMLRRL1 + SVSMLRRL2
                                           );

    //Clear level settings in the backup register,keep all other bits
    SVSMLCTL_backup &=
        ~(SVSLRVL0 + SVSLRVL1 + SVSMLRRL0 + SVSMLRRL1 + SVSMLRRL2);

    //Restore low-side SVS monitor settings
    HWREGB(baseAddress + OFS_SVSMLCTL) |= SVSMLCTL_backup;

    //Restore High side settings
    //Clear all other bits except level settings
    HWREGB(baseAddress + OFS_SVSMHCTL) &= (SVSHRVL0 + SVSHRVL1 + SVSMHRRL0 +
                                           SVSMHRRL1 + SVSMHRRL2
                                           );

    //Clear level settings in the backup register, keep all other bits
    SVSMHCTL_backup &=
        ~(SVSHRVL0 + SVSHRVL1 + SVSMHRRL0 + SVSMHRRL1 + SVSMHRRL2);

    //Restore backup
    HWREGB(baseAddress + OFS_SVSMHCTL) |= SVSMHCTL_backup;

    //Wait until high side, low side settled
    while (((HWREGB(baseAddress + OFS_PMMIFG) & SVSMLDLYIFG) == 0) &&
           ((HWREGB(baseAddress + OFS_PMMIFG) & SVSMHDLYIFG) == 0)) ;

    //Clear all Flags
    HWREGB(baseAddress + OFS_PMMIFG) &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG |
                                          SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG
                                          );

    //Restore PMM interrupt enable register
    HWREGB(baseAddress + OFS_PMMRIE) = PMMRIE_backup;
    //Lock PMM registers for write access
    HWREGB(baseAddress + OFS_PMMCTL0_H) = 0x00;
    //Return: OK
    return ( STATUS_SUCCESS) ;
}

//*****************************************************************************
//
//! Set Vcore to expected level
//!
//! \param baseAddress is the base address of the I2C module.
//! \param level level to which Vcore needs to be decreased/increased
//!         Valid values are
//!         \b PMM_CORE_LEVEL_0 [Default Value],
//!         \b PMM_CORE_LEVEL_1
//!         \b PMM_CORE_LEVEL_2
//!         \b PMM_CORE_LEVEL_3
//!
//! Modified registers are \b PMMCTL0, \b PMMRIE, \b PMMIFG, \b SVSMLCTL,
//! \b SVSMHCTL.
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//*****************************************************************************
unsigned short PMM_setVCore (unsigned int baseAddress, unsigned char level)
{
    ASSERT(
        (PMM_CORE_LEVEL_0 == level) ||
        (PMM_CORE_LEVEL_1 == level) ||
        (PMM_CORE_LEVEL_2 == level) ||
        (PMM_CORE_LEVEL_3 == level)
        );

    unsigned char actlevel;
    unsigned int status = STATUS_SUCCESS;

    //Set Mask for Max. level
    level &= PMMCOREV_3;

    //Get actual VCore
    actlevel = (HWREGB(baseAddress + OFS_PMMCTL0) & PMMCOREV_3);

    //step by step increase or decrease
    while ((level != actlevel) && (status == STATUS_SUCCESS))
    {
        if (level > actlevel){
            status = PMM_setVCoreUp(baseAddress, ++actlevel);
        } else   {
            status = PMM_setVCoreDown(baseAddress, --actlevel);
        }
    }

    return ( status) ;
}

//*****************************************************************************
//
//! Returns interrupt status
//!
//! \param baseAddress is the base address of the I2C module.
//! \param mask is the mask for specifying the required flag
//!        Valid values are
//!            \b  PMM_SVSMLDLYIFG
//!            \b  PMM_SVMLIFG,
//!            \b  PMM_SVMLVLRIFG,
//!            \b  PMM_SVSMHDLYIFG,
//!            \b  PMM_SVMHIFG,
//!            \b  PMM_SVMHVLRIFG,
//!            \b  PMM_PMMBORIFG,
//!            \b  PMM_PMMRSTIFG,
//!            \b  PMM_PMMPORIFG,
//!            \b  PMM_SVSHIFG,
//!            \b  PMM_SVSLIFG,
//!            \b  PMM_PMMLPM5IFG
//!
//! \return STATUS_SUCCESS or STATUS_FAIL
//
//*****************************************************************************
unsigned int PMM_getInterruptStatus (unsigned int baseAddress,
    unsigned int mask)
{
    return ( (HWREG(baseAddress + OFS_PMMIFG)) & mask );
}

//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************
