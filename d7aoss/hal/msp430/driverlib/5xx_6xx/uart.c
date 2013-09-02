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
//uart.c - Driver for the UART Module.
//
//*****************************************************************************
#include "../inc/hw_types.h"
#include "debug.h"
#include "uart.h"
#include "uart_baudrate.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "../deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif

//*****************************************************************************
//
//! Initializes the UART block.
//!
//! \param baseAddress is the base address of the UART module.
//! \param selectClockSource selects Clock source. Valid values are
//!         \b UART_CLOCKSOURCE_SMCLK
//!         \b UART_CLOCKSOURCE_ACLK
//! \param clockSourceFrequency is the frequency of the slected clock source
//! \param desiredUartClock is the desired clock rate for UART communication
//! \param parity is the desired parity. Valid values are
//!        \b UART_NO_PARITY  [Default Value],
//!        \b UART_ODD_PARITY,
//!        \b UART_EVEN_PARITY
//! \param msborLsbFirst controls direction of receive and transmit shift
//!     register. Valid values are
//!        \b UART_MSB_FIRST
//!        \b UART_LSB_FIRST [Default Value]
//! \param numberofStopBits indicates one/two STOP bits
//!      Valid values are
//!        \b UART_ONE_STOP_BIT [Default Value]
//!        \b UART_TWO_STOP_BITS
//! \param uartMode selects the mode of operation
//!      Valid values are
//!        \b UART_MODE  [Default Value],
//!        \b UART_IDLE_LINE_MULTI_PROCESSOR_MODE,
//!        \b UART_ADDRESS_BIT_MULTI_PROCESSOR_MODE,
//!        \b UART_AUTOMATIC_BAUDRATE_DETECTION_MODE
//! \param overSampling indicates low frequency or oversampling baud generation
//!      Valid values are
//!        \b UART_OVERSAMPLING_BAUDRATE_GENERATION
//!        \b UART_LOW_FREQUENCY_BAUDRATE_GENERATION
//!
//! Upon successful initialization of the UART block, this function
//! will have initialized the module, but the UART block still remains
//! disabled and must be enabled with UART_enable()
//!
//! Modified bits are \b UCPEN, \b UCPAR, \b UCMSB, \b UC7BIT, \b UCSPB,
//! \b UCMODEx, \b UCSYNC bits of \b UCAxCTL0 and \b UCSSELx,
//! \b UCSWRST bits of \b UCAxCTL1
//!
//! \return STATUS_SUCCESS or
//!         STATUS_FAIL of the initialization process
//
//*****************************************************************************
unsigned short UART_init ( unsigned int baseAddress,
    unsigned char selectClockSource,
    unsigned long clockSourceFrequency,
    unsigned long desiredUartBaudRate,
    unsigned char parity,
    unsigned char msborLsbFirst,
    unsigned char numberofStopBits,
    unsigned char uartMode,
    unsigned short overSampling
    )
{
    ASSERT(
        (UART_MODE == uartMode) ||
        (UART_IDLE_LINE_MULTI_PROCESSOR_MODE == uartMode) ||
        (UART_ADDRESS_BIT_MULTI_PROCESSOR_MODE == uartMode) ||
        (UART_AUTOMATIC_BAUDRATE_DETECTION_MODE == uartMode)
        );

    ASSERT(
        (UART_CLOCKSOURCE_ACLK == selectClockSource) ||
        (UART_CLOCKSOURCE_SMCLK == selectClockSource)
        );

    ASSERT(
        (UART_MSB_FIRST == msborLsbFirst) ||
        (UART_LSB_FIRST == msborLsbFirst)
        );

    ASSERT(
        (UART_ONE_STOP_BIT == numberofStopBits) ||
        (UART_TWO_STOP_BITS == numberofStopBits)
        );

    ASSERT(
        (UART_NO_PARITY == parity) ||
        (UART_ODD_PARITY == parity) ||
        (UART_EVEN_PARITY == parity)
        );


    unsigned char retVal = STATUS_SUCCESS;
    unsigned char UCAxBR0_value = 0x00;
    unsigned char UCAxBR1_value = 0x00;
    unsigned int UCAxMCTL_value = 0x00;

    //Disable the USCI Module
    HWREGB(baseAddress + OFS_UCAxCTL1) |= UCSWRST;

    //Clock source select
    HWREGB(baseAddress + OFS_UCAxCTL1) &= ~UCSSEL_3;
    HWREGB(baseAddress + OFS_UCAxCTL1) |= selectClockSource;

    //MSB, LSB select
    HWREGB(baseAddress + OFS_UCAxCTL0) &= ~UCMSB;
    HWREGB(baseAddress + OFS_UCAxCTL0) |= msborLsbFirst;


    //UCSPB = 0(1 stop bit) OR 1(2 stop bits)
    HWREGB(baseAddress + OFS_UCAxCTL0) &= ~UCSPB;
    HWREGB(baseAddress + OFS_UCAxCTL0) |= numberofStopBits;


    //Parity
    switch (parity){
        case UART_NO_PARITY:
            //No Parity
            HWREGB(baseAddress + OFS_UCAxCTL0) &= ~UCPEN;
            break;
        case UART_ODD_PARITY:
            //Odd Parity
            HWREGB(baseAddress + OFS_UCAxCTL0) |= UCPEN;
            HWREGB(baseAddress + OFS_UCAxCTL0) &= ~UCPAR;
            break;
        case UART_EVEN_PARITY:
            //Even Parity
            HWREGB(baseAddress + OFS_UCAxCTL0) |= UCPEN;
            HWREGB(baseAddress + OFS_UCAxCTL0) |= UCPAR;
            break;
    }

    //Calculate Baud rate divider values for Modulation control registers
    if ( STATUS_FAIL == UARTBAUDRATE_calculateBaudDividers(clockSourceFrequency,
             desiredUartBaudRate,
             &UCAxBR0_value,
             &UCAxBR1_value,
             &UCAxMCTL_value,
             overSampling
             )){
        return ( STATUS_FAIL) ;
    }

    //Modulation Control Registers
    HWREGB(baseAddress + OFS_UCAxBR0 ) = UCAxBR0_value;
    HWREGB(baseAddress + OFS_UCAxBR1) = UCAxBR1_value;
    HWREGB(baseAddress + OFS_UCAxMCTL) = UCAxMCTL_value;

    //Asynchronous mode & 8 bit character select & clear mode
    HWREGB(baseAddress + OFS_UCAxCTL0) &=  ~(UCSYNC +
                                             UC7BIT +
                                             UCMODE_3
                                             );

    //Configure  UART mode.
    HWREGB(baseAddress + OFS_UCAxCTL0) |= uartMode ;

    //Reset UCRXIE, UCBRKIE, UCDORM, UCTXADDR, UCTXBRK
    HWREGB(baseAddress + OFS_UCAxCTL1)  &= ~(UCRXEIE + UCBRKIE + UCDORM +
                                             UCTXADDR + UCTXBRK
                                             );

    return ( retVal) ;
}

//*****************************************************************************
//
//! Advanced initialization routine for the UART block. The values to be written
//! into the UCAxBRW and UCAxMCTLW registers should be pre-computed and passed
//! into the initialization function
//!
//! \param baseAddress is the base address of the UART module.
//! \param selectClockSource selects Clock source. Valid values are
//!         \b UART_CLOCKSOURCE_SMCLK
//!         \b UART_CLOCKSOURCE_ACLK
//! \param clockPrescalar is the value to be written into UCBRx bits
//! \param firstModReg  is First modulation stage register setting. This value
//! 	is a pre-calculated value which can be obtained from the Device User�s
//!		Guide.This value is written into UCBRFx bits of UCAxMCTLW.
//! \param secondModReg is Second modulation stage register setting.
//! 	This value is a pre-calculated value which can be obtained from the Device
//! 	User�s Guide. This value is written into UCBRSx bits of UCAxMCTLW.
//! \param parity is the desired parity. Valid values are
//!        \b UART_NO_PARITY  [Default Value],
//!        \b UART_ODD_PARITY,
//!        \b UART_EVEN_PARITY
//! \param msborLsbFirst controls direction of receive and transmit shift
//!     register. Valid values are
//!        \b UART_MSB_FIRST
//!        \b UART_LSB_FIRST [Default Value]
//! \param numberofStopBits indicates one/two STOP bits
//!      Valid values are
//!        \b UART_ONE_STOP_BIT [Default Value]
//!        \b UART_TWO_STOP_BITS
//! \param uartMode selects the mode of operation
//!      Valid values are
//!        \b UART_MODE  [Default Value],
//!        \b UART_IDLE_LINE_MULTI_PROCESSOR_MODE,
//!        \b UART_ADDRESS_BIT_MULTI_PROCESSOR_MODE,
//!        \b UART_AUTOMATIC_BAUDRATE_DETECTION_MODE
//! \param overSampling indicates low frequency or oversampling baud generation
//!      Valid values are
//!        \b UART_OVERSAMPLING_BAUDRATE_GENERATION
//!        \b UART_LOW_FREQUENCY_BAUDRATE_GENERATION
//!
//! Upon successful initialization of the UART block, this function
//! will have initialized the module, but the UART block still remains
//! disabled and must be enabled with UART_enable()
//!
//! Modified bits are \b UCPEN, \b UCPAR, \b UCMSB, \b UC7BIT, \b UCSPB,
//! \b UCMODEx, \b UCSYNC bits of \b UCAxCTL0 and \b UCSSELx,
//! \b UCSWRST bits of \b UCAxCTL1
//!
//! \return STATUS_SUCCESS or
//!         STATUS_FAIL of the initialization process
//
//*****************************************************************************
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
    )
{
    ASSERT(
        (UART_MODE == uartMode) ||
        (UART_IDLE_LINE_MULTI_PROCESSOR_MODE == uartMode) ||
        (UART_ADDRESS_BIT_MULTI_PROCESSOR_MODE == uartMode) ||
        (UART_AUTOMATIC_BAUDRATE_DETECTION_MODE == uartMode)
        );

    ASSERT(
        (UART_CLOCKSOURCE_ACLK == selectClockSource) ||
        (UART_CLOCKSOURCE_SMCLK == selectClockSource)
        );

    ASSERT(
        (UART_MSB_FIRST == msborLsbFirst) ||
        (UART_LSB_FIRST == msborLsbFirst)
        );

    ASSERT(
        (UART_ONE_STOP_BIT == numberofStopBits) ||
        (UART_TWO_STOP_BITS == numberofStopBits)
        );

    ASSERT(
        (UART_NO_PARITY == parity) ||
        (UART_ODD_PARITY == parity) ||
        (UART_EVEN_PARITY == parity)
        );


    unsigned char retVal = STATUS_SUCCESS;

    //Disable the USCI Module
    HWREGB(baseAddress + OFS_UCAxCTL1) |= UCSWRST;

    //Clock source select
    HWREGB(baseAddress + OFS_UCAxCTL1) &= ~UCSSEL_3;
    HWREGB(baseAddress + OFS_UCAxCTL1) |= selectClockSource;

    //MSB, LSB select
    HWREGB(baseAddress + OFS_UCAxCTL0) &= ~UCMSB;
    HWREGB(baseAddress + OFS_UCAxCTL0) |= msborLsbFirst;


    //UCSPB = 0(1 stop bit) OR 1(2 stop bits)
    HWREGB(baseAddress + OFS_UCAxCTL0) &= ~UCSPB;
    HWREGB(baseAddress + OFS_UCAxCTL0) |= numberofStopBits;


    //Parity
    switch (parity){
        case UART_NO_PARITY:
            //No Parity
            HWREGB(baseAddress + OFS_UCAxCTL0) &= ~UCPEN;
            break;
        case UART_ODD_PARITY:
            //Odd Parity
            HWREGB(baseAddress + OFS_UCAxCTL0) |= UCPEN;
            HWREGB(baseAddress + OFS_UCAxCTL0) &= ~UCPAR;
            break;
        case UART_EVEN_PARITY:
            //Even Parity
            HWREGB(baseAddress + OFS_UCAxCTL0) |= UCPEN;
            HWREGB(baseAddress + OFS_UCAxCTL0) |= UCPAR;
            break;
    }

    //Modulation Control Registers
    HWREGB(baseAddress + OFS_UCAxBRW ) = clockPrescalar;
    HWREGB(baseAddress + OFS_UCAxMCTL) = ((firstModReg<<4) + (secondModReg <<1) +
    									  overSampling );

    //Asynchronous mode & 8 bit character select & clear mode
    HWREGB(baseAddress + OFS_UCAxCTL0) &=  ~(UCSYNC +
                                             UC7BIT +
                                             UCMODE_3
                                             );

    //Configure  UART mode.
    HWREGB(baseAddress + OFS_UCAxCTL0) |= uartMode ;

    //Reset UCRXIE, UCBRKIE, UCDORM, UCTXADDR, UCTXBRK
    HWREGB(baseAddress + OFS_UCAxCTL1)  &= ~(UCRXEIE + UCBRKIE + UCDORM +
                                             UCTXADDR + UCTXBRK
                                             );

    return ( retVal) ;
}

//*****************************************************************************
//
//! Transmits a byte from the UART Module.
//!
//! \param baseAddress is the base address of the UART module.
//! \param transmitData data to be transmitted from the UART module
//!
//! This function will place the supplied data into UART trasmit data register
//! to start transmission
//!
//! Modified register is \b UCAxTXBUF
//! \return None.
//
//*****************************************************************************
void UART_transmitData ( unsigned int baseAddress,
    unsigned char transmitData
    )
{
    HWREGB(baseAddress + OFS_UCAxTXBUF) = transmitData;
}

//*****************************************************************************
//
//! Receives a byte that has been sent to the UART Module.
//!
//! \param baseAddress is the base address of the UART module.
//!
//! This function reads a byte of data from the UART receive data Register.
//!
//! Modified register is \b UCAxRXBUF
//!
//! \return Returns the byte received from by the UART module, cast as an
//! unsigned char.
//
//*****************************************************************************
unsigned char UART_receiveData (unsigned int baseAddress)
{
    return ( HWREGB(baseAddress + OFS_UCAxRXBUF)) ;
}

//*****************************************************************************
//
//! Enables individual UART interrupt sources.
//!
//! \param baseAddress is the base address of the UART module.
//! \param mask is the bit mask of the interrupt sources to be enabled.
//!
//! Enables the indicated UART interrupt sources.  The interrupt flag is first
//! and then the corresponfing interrupt is enabled. Only the sources that
//! are enabled can be reflected to the processor interrupt; disabled sources
//! have no effect on the processor.
//!
//! The mask parameter is the logical OR of any of the following:
//! - \b UART_RECEIVE_INTERRUPT -Receive interrupt
//! - \b UART_TRANSMIT_INTERRUPT - Transmit interrupt
//! - \b UART_RECEIVE_ERRONEOUSCHAR_INTERRUPT - Receive erroneous-character
//!                             interrupt enable
//! - \b UART_BREAKCHAR_INTERRUPT - Receive break character interrupt enable
//!
//! Modified register is \b UCAxIFG, \b UCAxIE and \b UCAxCTL1
//!
//! \return None.
//
//*****************************************************************************
void UART_enableInterrupt (unsigned int baseAddress,
    unsigned char mask
    )
{
    ASSERT((UART_RECEIVE_INTERRUPT == mask) ||
        (UART_TRANSMIT_INTERRUPT == mask) ||
        (UART_RECEIVE_ERRONEOUSCHAR_INTERRUPT == mask) ||
        (UART_BREAKCHAR_INTERRUPT == mask)
        );
    switch (mask){
        case UART_RECEIVE_INTERRUPT:
        case UART_TRANSMIT_INTERRUPT:
            //Clear interrupt flag
            HWREG(baseAddress + OFS_UCAxIFG) &= ~(mask);
            //Enable Interrupt
            HWREGB(baseAddress + OFS_UCAxIE) |= mask;
            break;
        case UART_RECEIVE_ERRONEOUSCHAR_INTERRUPT:
        case UART_BREAKCHAR_INTERRUPT:
            //Enable Interrupt
            HWREGB(baseAddress + OFS_UCAxCTL1) |= mask;
            break;
    }
}

//*****************************************************************************
//
//! Disables individual UART interrupt sources.
//!
//! \param baseAddress is the base address of the UART module.
//! \param mask is the bit mask of the interrupt sources to be
//! disabled.
//!
//! Disables the indicated UART interrupt sources.  Only the sources that
//! are enabled can be reflected to the processor interrupt; disabled sources
//! have no effect on the processor.
//!
//! The mask parameter is the logical OR of any of the following:
//! - \b UART_RECEIVE_INTERRUPT -Receive interrupt
//! - \b UART_TRANSMIT_INTERRUPT - Transmit interrupt
//! - \b UART_RECEIVE_ERRONEOUSCHAR_INTERRUPT - Receive erroneous-character
//!                             interrupt enable
//! - \b UART_BREAKCHAR_INTERRUPT - Receive break character interrupt enable
//!
//! Modified register is \b UCAxIFG, \b UCAxIE and \b UCAxCTL1
//! \return None.
//
//*****************************************************************************
void UART_disableInterrupt (unsigned int baseAddress,
    unsigned char mask
    )
{
    ASSERT((UART_RECEIVE_INTERRUPT == mask) ||
        (UART_TRANSMIT_INTERRUPT == mask) ||
        (UART_RECEIVE_ERRONEOUSCHAR_INTERRUPT == mask) ||
        (UART_BREAKCHAR_INTERRUPT == mask)
        );

    switch (mask){
        case UART_RECEIVE_INTERRUPT:
        case UART_TRANSMIT_INTERRUPT:
            //Disable Interrupt
            HWREGB(baseAddress + OFS_UCAxIE) &= ~mask;
            break;
        case UART_RECEIVE_ERRONEOUSCHAR_INTERRUPT:
        case UART_BREAKCHAR_INTERRUPT:
            //Disable Interrupt
            HWREGB(baseAddress + OFS_UCAxCTL1) &= ~mask;
            break;
    }
}

//*****************************************************************************
//
//! Gets the current UART interrupt status.
//!
//! \param baseAddress is the base address of the UART module.
//! \param mask is the masked interrupt flag status to be returned.
//!
//! This returns the interrupt status for the UART  module based on which
//! flag is passed. mask parameter can be either any of the following
//! selection.
//! - \b UART_RECEIVE_INTERRUPT_FLAG -Receive interrupt flag
//! - \b UART_TRANSMIT_INTERRUPT_FLAG - Transmit interrupt flag
//!
//! Modified register is \b UCAxIFG.
//!
//! \return The current interrupt status, returned as with the respective bits
//! set if the corresponding interrupt flag is set
//
//*****************************************************************************
unsigned char UART_getInterruptStatus (unsigned int baseAddress,
    unsigned char mask)
{
    ASSERT(  (UART_RECEIVE_INTERRUPT_FLAG == mask) ||
        (UART_TRANSMIT_INTERRUPT_FLAG == mask)
        );

    return ( HWREGB(baseAddress + OFS_UCAxIFG) & mask );
}

//*****************************************************************************
//
//! Clears UART interrupt sources.
//!
//! \param baseAddress is the base address of the UART module.
//! \param mask is a bit mask of the interrupt sources to be cleared.
//!
//! The UART interrupt source is cleared, so that it no longer asserts.
//! The highest interrupt flag is automatically cleared when an interrupt vector
//! generator is used.
//!
//! The mask parameter has the same definition as the mask parameter to
//! UART_enableInterrupt().
//!
//! Modified register is \b UCAxIFG
//!
//! \return None.
//
//*****************************************************************************
void UART_clearInterruptFlag (unsigned int baseAddress, unsigned char mask)
{
    //Clear the UART interrupt source.
    HWREG(baseAddress + OFS_UCAxIFG) &= ~(mask);
}

//*****************************************************************************
//
//! Enables the UART block.
//!
//! \param baseAddress is the base address of the USCI UART module.
//!
//! This will enable operation of the UART block.
//!
//! Modified register is \b UCAxCTL1
//!
//! \return None.
//
//*****************************************************************************
void UART_enable (unsigned int baseAddress)
{
    //Reset the UCSWRST bit to enable the USCI Module
    HWREGB(baseAddress + OFS_UCAxCTL1) &= ~(UCSWRST);
}

//*****************************************************************************
//
//! Disables the UART block.
//!
//! \param baseAddress is the base address of the USCI UART module.
//!
//! This will disable operation of the UART block.
//!
//! Modified register is \b UCAxCTL1
//!
//! \return None.
//
//*****************************************************************************
void UART_disable (unsigned int baseAddress)
{
    //Set the UCSWRST bit to disable the USCI Module
    HWREGB(baseAddress + OFS_UCAxCTL1) |= UCSWRST;
}

//*****************************************************************************
//
//! Gets the current UART status flags.
//!
//! \param baseAddress is the base address of the UART module.
//! \param mask is the masked interrupt flag status to be returned.
//!
//! This returns the status for the UART  module based on which
//! flag is passed. mask parameter can be either any of the following
//! selection.
//! - \b UART_LISTEN_ENABLE
//! - \b UART_FRAMING_ERROR
//! - \b UART_OVERRUN_ERROR
//! - \b UART_PARITY_ERROR
//! - \b UARTBREAK_DETECT
//! - \b UART_RECEIVE_ERROR
//! - \b UART_ADDRESS_RECEIVED
//! - \b UART_IDLELINE
//! - \b UART_BUSY
//!
//! Modified register is \b UCAxSTAT
//!
//! \return the masked status flag
//
//*****************************************************************************
unsigned char UART_queryStatusFlags (unsigned int baseAddress,
    unsigned char mask)
{
    ASSERT((UART_LISTEN_ENABLE == mask) ||
        (UART_FRAMING_ERROR == mask) ||
        (UART_OVERRUN_ERROR == mask) ||
        (UART_PARITY_ERROR == mask) ||
        (UARTBREAK_DETECT == mask) ||
        (UART_RECEIVE_ERROR == mask) ||
        (UART_ADDRESS_RECEIVED == mask) ||
        (UART_IDLELINE == mask) ||
        (UART_BUSY == mask)
        );
    return ( HWREGB(baseAddress + OFS_UCAxSTAT) & mask );
}

//*****************************************************************************
//
//! Sets the UART module in dormant mode
//!
//! \param baseAddress is the base address of the UART module.
//!
//! Puts USCI in sleep mode
//! Only characters that are preceded by an idle-line or with address bit set
//! UCRXIFG. In UART mode with automatic baud-rate detection, only the
//! combination of a break and synch field sets UCRXIFG.
//!
//! Modified register is \b UCAxCTL1
//!
//! \return None.
//
//*****************************************************************************
void UART_setDormant (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_UCAxCTL1) |= UCDORM;
}

//*****************************************************************************
//
//! Re-enables UART module from dormant mode
//!
//! \param baseAddress is the base address of the UART module.
//!
//! Not dormant. All received characters set UCRXIFG.
//!
//! Modified register is \b UCAxCTL1
//!
//! \return None.
//
//*****************************************************************************
void UART_resetDormant (unsigned int baseAddress)
{
    HWREGB(baseAddress + OFS_UCAxCTL1) &= ~UCDORM;
}

//*****************************************************************************
//
//! Transmits the next byte to be transmitted marked as address depending on
//! selected multiprocessor mode
//!
//! \param baseAddress is the base address of the UART module.
//! \param transmitAddress is the next byte to be transmitted
//!
//! Modified register is \b UCAxCTL1, \b UCAxTXBUF
//!
//! \return None.
//
//*****************************************************************************
void UART_transmitAddress (unsigned int baseAddress,
    unsigned char transmitAddress)
{
    //Set UCTXADDR bit
    HWREGB(baseAddress + OFS_UCAxCTL1) |= UCTXADDR;

    //Place next byte to be sent into the transmit buffer
    HWREGB(baseAddress + OFS_UCAxTXBUF) = transmitAddress;
}

//*****************************************************************************
//
//! Transmit break. Transmits a break with the next write to the transmit
//! buffer. In UART mode with automatic baud-rate detection,
//! UART_AUTOMATICBAUDRATE_SYNC(0x55) must be written into UCAxTXBUF to
//! generate the required break/synch fields.
//! Otherwise, DEFAULT_SYNC(0x00) must be written into the transmit buffer.
//! Also ensures module is ready for transmitting the next data
//!
//! \param baseAddress is the base address of the UART module.
//!
//! Modified register is \b UCAxCTL1, \b UCAxTXBUF
//!
//! \return None.
//
//*****************************************************************************
void UART_transmitBreak (unsigned int baseAddress)
{
    //Set UCTXADDR bit
    HWREGB(baseAddress + OFS_UCAxCTL1) |= UCTXBRK;

    //If current mode is automatic baud-rate detection
    if (UART_AUTOMATIC_BAUDRATE_DETECTION_MODE ==
        (HWREGB(baseAddress + OFS_UCAxCTL0) &
         UART_AUTOMATIC_BAUDRATE_DETECTION_MODE)){
        HWREGB(baseAddress + OFS_UCAxTXBUF) = UART_AUTOMATICBAUDRATE_SYNC;
    } else   {
        HWREGB(baseAddress + OFS_UCAxTXBUF) = DEFAULT_SYNC;
    }

    //USCI TX buffer ready?
    while (!UART_getInterruptStatus(baseAddress, UCTXIFG)) ;
}

//*****************************************************************************
//
//! Returns the address of the RX Buffer of the UART for the DMA module.
//!
//! \param baseAddress is the base address of the UART module.
//!
//! Returns the address of the UART RX Buffer. This can be used in conjunction
//! with the DMA to store the received data directly to memory.
//!
//! \return None
//
//*****************************************************************************
unsigned long UART_getReceiveBufferAddressForDMA (unsigned int baseAddress)
{
    return ( baseAddress + OFS_UCAxRXBUF );
}

//*****************************************************************************
//
//! Returns the address of the TX Buffer of the UART for the DMA module.
//!
//! \param baseAddress is the base address of the UART module.
//!
//! Returns the address of the UART TX Buffer. This can be used in conjunction
//! with the DMA to obtain transmitted data directly from memory.
//!
//! \return None
//
//*****************************************************************************
unsigned long UART_getTransmitBufferAddressForDMA (unsigned int baseAddress)
{
    return ( baseAddress + OFS_UCAxTXBUF );
}

//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************
