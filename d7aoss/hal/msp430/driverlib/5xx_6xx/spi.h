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
#ifndef __MSP430WARE_SPI_H__
#define __MSP430WARE_SPI_H__

//*****************************************************************************
//
//The following are the defines to include the required modules for this
//peripheral in msp430xgeneric.h file
//
//*****************************************************************************
#define __MSP430_HAS_USCI_Ax__

//*****************************************************************************
//
//The following are values that can be passed to the SPI_masterInit() API
//as the selectClockSource parameter.
//
//*****************************************************************************
#define SPI_CLOCKSOURCE_ACLK    UCSSEL__ACLK
#define SPI_CLOCKSOURCE_SMCLK   UCSSEL__SMCLK

//*****************************************************************************
//
//The following are values that can be passed to the SPI_masterInit() ,
//SPI_slaveInit() API as the msbFirst parameter.
//
//*****************************************************************************
#define SPI_MSB_FIRST    UCMSB
#define SPI_LSB_FIRST    0x00

//*****************************************************************************
//
//The following are values that can be returned by the SPI_isBusy() API
//
//*****************************************************************************
#define SPI_BUSY        UCBUSY
#define SPI_NOT_BUSY    0x00

//*****************************************************************************
//
//The following are values that can be passed to the SPI_masterInit() ,
//SPI_slaveInit() API as the clockPhase parameter.
//
//*****************************************************************************
#define SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT    0x00
#define SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT    UCCKPH

//*****************************************************************************
//
//The following are values that can be passed to the SPI_masterInit() ,
//SPI_slaveInit() API as the clockPolarity parameter.
//
//*****************************************************************************
#define SPI_CLOCKPOLARITY_INACTIVITY_HIGH    UCCKPL
#define SPI_CLOCKPOLARITY_INACTIVITY_LOW     0x00

//*****************************************************************************
//
//The following are values that can be passed to the SPI_enableInterrupt() ,
//SPI_disableInterrupt(), SPI_getInterruptStatus(),  API as the mask parameter.
//
//*****************************************************************************
#define SPI_TRANSMIT_INTERRUPT    UCTXIE
#define SPI_RECEIVE_INTERRUPT     UCRXIE

//*****************************************************************************
//
//Prototypes for the APIs.
//
//*****************************************************************************
extern unsigned short SPI_masterInit (unsigned int baseAddress,
    unsigned char selectClockSource,
    unsigned long clockSourceFrequency,
    unsigned long desiredSpiClock,
    unsigned char msbFirst,
    unsigned char clockPhase,
    unsigned char clockPolarity
    );
extern void SPI_masterChangeClock (unsigned int baseAddress,
    unsigned long clockSourceFrequency,
    unsigned long desiredSpiClock
    );

extern unsigned short SPI_slaveInit (unsigned int baseAddress,
    unsigned char msbFirst,
    unsigned char clockPhase,
    unsigned char clockPolarity
    );
extern void SPI_changeClockPhasePolarity (unsigned int baseAddress,
    unsigned char clockPhase,
    unsigned char clockPolarity
    );
extern void SPI_transmitData ( unsigned int baseAddress,
    unsigned char transmitData
    );

extern unsigned char SPI_receiveData (unsigned int baseAddress);
extern void SPI_enableInterrupt (unsigned int baseAddress,
    unsigned char mask
    );
extern void SPI_disableInterrupt (unsigned int baseAddress,
    unsigned char mask
    );
extern unsigned char SPI_getInterruptStatus (unsigned int baseAddress,
    unsigned char mask
    );
extern void SPI_enable (unsigned int baseAddress);
extern void SPI_disable (unsigned int baseAddress);
extern unsigned long SPI_getReceiveBufferAddressForDMA
    (unsigned int baseAddress);
extern unsigned long SPI_getTransmitBufferAddressForDMA
    (unsigned int baseAddress);
extern unsigned char SPI_isBusy (unsigned int baseAddress);
extern void SPI_clearInterruptFlag (unsigned int baseAddress,
    unsigned char mask
    );
#endif

