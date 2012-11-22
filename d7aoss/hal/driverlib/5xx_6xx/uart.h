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
#ifndef __MSP430WARE_UART_H__
#define __MSP430WARE_UART_H__

//*****************************************************************************
//
//The following are the defines to include the required modules for this
//peripheral in msp430xgeneric.h file
//
//*****************************************************************************
#define __MSP430_HAS_USCI_Ax__

//*****************************************************************************
//
//The following are values that can be passed to the UART_init() API
//as the parity parameter.
//
//*****************************************************************************
#define UART_NO_PARITY   0x00
#define UART_ODD_PARITY  0x01
#define UART_EVEN_PARITY 0x02

//*****************************************************************************
//
//The following are values that can be passed to the UART_init() API
//as the selectClockSource parameter.
//
//*****************************************************************************
#define UART_CLOCKSOURCE_ACLK    UCSSEL__ACLK
#define UART_CLOCKSOURCE_SMCLK   UCSSEL__SMCLK

//*****************************************************************************

//*****************************************************************************
//
//The following are values that can be passed to the UART_init() API
//as the numberofStopBits parameter.
//
//*****************************************************************************
#define UART_ONE_STOP_BIT    0x00
#define UART_TWO_STOP_BITS   UCSPB


//*****************************************************************************
//
//The following are values that can be passed to the UART_init() API
//as the msborLsbFirst parameter.
//
//*****************************************************************************
#define UART_MSB_FIRST    UCMSB
#define UART_LSB_FIRST    0x00

//*****************************************************************************
//
//The following are values that can be passed to the UART_getInterruptStatus(),
//as the mask parameter.
//
//*****************************************************************************
#define UART_RECEIVE_INTERRUPT_FLAG            UCRXIFG
#define UART_TRANSMIT_INTERRUPT_FLAG           UCTXIFG

//*****************************************************************************
//
//The following are values that can be passed to the UART_enableInterrupt(),
//UART_disableInterrupt() API as the mask parameter.
//
//*****************************************************************************
#define UART_RECEIVE_INTERRUPT                  UCRXIE
#define UART_TRANSMIT_INTERRUPT                 UCTXIE
#define UART_RECEIVE_ERRONEOUSCHAR_INTERRUPT    UCRXEIE
#define UART_BREAKCHAR_INTERRUPT                UCBRKIE

//*****************************************************************************
//
//The following are values that can be passed to the UART_queryStatusFlags()
//API as the mask parameter.
//
//*****************************************************************************
#define UART_LISTEN_ENABLE      UCLISTEN
#define UART_FRAMING_ERROR      UCFE
#define UART_OVERRUN_ERROR      UCOE
#define UART_PARITY_ERROR       UCPE
#define UARTBREAK_DETECT        UCBRK
#define UART_RECEIVE_ERROR      UCRXERR
#define UART_ADDRESS_RECEIVED   UCADDR
#define UART_IDLELINE           UCIDLE
#define UART_BUSY               UCBUSY

//*****************************************************************************
//
//The following are values that can be passed to the UART_init()
//API as the mode parameter.
//
//*****************************************************************************
#define UART_MODE                              UCMODE_0
#define UART_IDLE_LINE_MULTI_PROCESSOR_MODE    UCMODE_1
#define UART_ADDRESS_BIT_MULTI_PROCESSOR_MODE  UCMODE_2
#define UART_AUTOMATIC_BAUDRATE_DETECTION_MODE UCMODE_3

//*****************************************************************************
//
//The following are values that can be passed to the UART_init()
//API as the overSampling parameter.
//
//*****************************************************************************
#define UART_OVERSAMPLING_BAUDRATE_GENERATION     0x01
#define UART_LOW_FREQUENCY_BAUDRATE_GENERATION    0x00


//*****************************************************************************
//
//The following are values are the sync characters possible
//
//*****************************************************************************
#define DEFAULT_SYNC 0x00
#define UART_AUTOMATICBAUDRATE_SYNC 0x55

//*****************************************************************************
//
//Prototypes for the APIs.
//
//*****************************************************************************
extern
unsigned short UART_init ( unsigned int baseAddress,
    unsigned char selectClockSource,
    unsigned long clockSourceFrequency,
    unsigned long desiredUartBaudRate,
    unsigned char parity,
    unsigned char msborLsbFirst,
    unsigned char numberofStopBits,
    unsigned char uartMode,
    unsigned short overSampling
    );
extern
unsigned short UART_initAdvance ( unsigned int baseAddress,
    unsigned char selectClockSource,
    unsigned int clockPrescalar,
    unsigned char firstModReg,
    unsigned char secondModReg,
    unsigned char parity,
    unsigned char msborLsbFirst,
    unsigned char numberofStopBits,
    unsigned char uartMode,
    unsigned short overSampling
    );
extern
void UART_transmitData ( unsigned int baseAddress,
    unsigned char transmitData
    );
extern
unsigned char UART_receiveData (unsigned int baseAddress);
extern
void UART_enableInterrupt (unsigned int baseAddress,
    unsigned char mask
    );
extern
void UART_disableInterrupt (unsigned int baseAddress,
    unsigned char mask
    );
extern
unsigned char UART_getInterruptStatus (unsigned int baseAddress,
    unsigned char mask
    );
extern
void UART_clearInterruptFlag (unsigned int baseAddress,
    unsigned char mask
    );
extern
void UART_enable (unsigned int baseAddress);
extern
void UART_disable (unsigned int baseAddress);
extern
unsigned char UART_queryStatusFlags (unsigned int baseAddress,
    unsigned char mask);
extern
void UART_setDormant (unsigned int baseAddress);
extern
void UART_resetDormant (unsigned int baseAddress);
extern
void UART_transmitAddress (unsigned int baseAddress,
    unsigned char transmitAddress);
extern
void UART_transmitBreak (unsigned int baseAddress);
extern
unsigned long UART_getReceiveBufferAddressForDMA (unsigned int baseAddress);
extern
unsigned long UART_getTransmitBufferAddressForDMA (unsigned int baseAddress);
#endif
