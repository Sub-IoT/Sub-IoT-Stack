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
#ifndef __MSP430WARE_PMM_H__
#define __MSP430WARE_PMM_H__

#define  __MSP430_HAS_PMM__

//*****************************************************************************
//
//The following are values that can be passed to the
//PMM_setVCore() API as the level parameter.
//
//*****************************************************************************
#define PMM_CORE_LEVEL_0    PMMCOREV_0
#define PMM_CORE_LEVEL_1    PMMCOREV_1
#define PMM_CORE_LEVEL_2    PMMCOREV_2
#define PMM_CORE_LEVEL_3    PMMCOREV_3

//*****************************************************************************
//
//The following are values that can be passed to the
//PMMInterruptStatus() API as the mask parameter.
//
//*****************************************************************************
#define PMM_SVSMLDLYIFG SVSMLDLYIFG
#define PMM_SVMLIFG     SVMLIFG
#define PMM_SVMLVLRIFG  SVMLVLRIFG
#define PMM_SVSMHDLYIFG SVSMHDLYIFG
#define PMM_SVMHIFG     SVMHIFG
#define PMM_SVMHVLRIFG  SVMHVLRIFG
#define PMM_PMMBORIFG   PMMBORIFG
#define PMM_PMMRSTIFG   PMMRSTIFG
#define PMM_PMMPORIFG   PMMPORIFG
#define PMM_SVSHIFG     SVSHIFG
#define PMM_SVSLIFG     SVSLIFG
#define PMM_PMMLPM5IFG  PMMLPM5IFG
//*****************************************************************************
//
//Prototypes for the APIs.
//
//*****************************************************************************
extern void PMM_enableSvsL (unsigned int baseAddress);
extern void PMM_disableSvsL (unsigned int baseAddress);
extern void PMM_enableSvmL (unsigned int baseAddress);
extern void PMM_disableSvmL (unsigned int baseAddress);
extern void PMM_enableSvsH (unsigned int baseAddress);
extern void PMM_disableSvsH (unsigned int baseAddress);
extern void PMM_enableSvmH (unsigned int baseAddress);
extern void PMM_disableSvmH (unsigned int baseAddress);
extern void PMM_enableSvsL_SVML (unsigned int baseAddress);
extern void PMM_disableSvsL_SVML (unsigned int baseAddress);
extern void PMM_enableSvsH_SVMH (unsigned int baseAddress);
extern void PMM_disableSvsH_SVMH (unsigned int baseAddress);
extern void PMM_enableSvsLReset (unsigned int baseAddress);
extern void PMM_disableSvsLReset (unsigned int baseAddress);
extern void PMM_enableSvmLInterrupt (unsigned int baseAddress);
extern void PMM_disableSvmLInterrupt (unsigned int baseAddress);
extern void PMM_enableSvsHReset (unsigned int baseAddress);
extern void PMM_disableSvsHReset (unsigned int baseAddress);
extern void PMM_enableSvmHInterrupt (unsigned int baseAddress);
extern void PMM_disableSvmHInterrupt (unsigned int baseAddress);
extern void PMM_clearPMMIFGS (unsigned int baseAddress);
extern void PMM_SvsLEnabledInLPMFastWake (unsigned int baseAddress);
extern void PMM_SvsLEnabledInLPMSlowWake (unsigned int baseAddress);
extern void PMM_SvsLDisabledInLPMFastWake (unsigned int baseAddress);
extern void PMM_SvsLDisabledInLPMSlowWake (unsigned int baseAddress);
extern void PMM_SvsHEnabledInLPMNormPerf (unsigned int baseAddress) ;
extern void PMM_SvsHEnabledInLPMFullPerf (unsigned int baseAddress) ;
extern void PMM_SvsHDisabledInLPMNormPerf (unsigned int baseAddress);
extern void PMM_SvsHDisabledInLPMFullPerf (unsigned int baseAddress);
extern void PMM_SvsLOptimizedInLPMFastWake (unsigned int baseAddress);
extern void PMM_SvsHOptimizedInLPMFullPerf (unsigned int baseAddress) ;
extern unsigned short PMM_setVCore (unsigned int baseAddress,
    unsigned char level
    );
extern unsigned int PMM_getInterruptStatus (unsigned int baseAddress,
    unsigned int mask
    );

#endif
