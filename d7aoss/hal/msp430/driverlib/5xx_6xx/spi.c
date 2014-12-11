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
//spi.c - Driver for the SPI Module.
//
//*****************************************************************************
#include "../inc/hw_types.h"
#include "debug.h"
#include "spi.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif

//*****************************************************************************
//
//! Initializes the SPI Master block.
//!
//! \param baseAddress is the base address of the I2C Master module.
//! \param selectClockSource selects Clock source. Valid values are
//!         \b SPI_CLOCKSOURCE_ACLK
//!         \b SPI_CLOCKSOURCE_SMCLK
//! \param clockSourceFrequency is the frequency of the slected clock source
//! \param desiredSpiClock is the desired clock rate for SPI communication
//! \param msbFirst controls the direction of the receive and transmit shift
//!      register. Valid values are
//!         \b SPI_MSB_FIRST
//!         \b SPI_LSB_FIRST [Default Value]
//! \param clockPhase is clock phase select.
//!         Valid values are
//!         \b SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT  [Default Value]
//!         \b SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT
//! \param clockPolarity_InactivityHIGH is clock polarity select.
//!         Valid values are
//!         \b SPI_CLOCKPOLARITY_INACTIVITY_HIGH
//!         \b SPI_CLOCKPOLARITY_INACTIVITY_LOW  [Default Value]
//! Upon successful initialization of the SPI master block, this function
//! will have set the bus speed for the master, but the SPI Master block
//! still remains disabled and must be enabled with SPI_enable()
//!
//! Modified bits are \b UCCKPH, \b UCCKPL, \b UC7BIT, \b UCMSB bits of
//! \b UCBxCTL0 register
//! and \b UCSSELx, \b UCSWRST bits of \b UCBxCTL1 register
//!
//! \return STATUS_SUCCESS
//
//*****************************************************************************
unsigned short SPI_masterInit (unsigned int baseAddress,
    unsigned char selectClockSource,
    unsigned long clockSourceFrequency,
    unsigned long desiredSpiClock,
    unsigned char msbFirst,
    unsigned char clockPhase,
    unsigned char clockPolarity
    )
{
    ASSERT(
        (SPI_CLOCKSOURCE_ACLK == selectClockSource) ||
        (SPI_CLOCKSOURCE_SMCLK == selectClockSource)
        );

    ASSERT(  (SPI_MSB_FIRST == msbFirst) ||
        (SPI_LSB_FIRST == msbFirst)
        );

    ASSERT(  (SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT == clockPhase) ||
        (SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT == clockPhase)
        );

    ASSERT(  (SPI_CLOCKPOLARITY_INACTIVITY_HIGH == clockPolarity) ||
        (SPI_CLOCKPOLARITY_INACTIVITY_LOW == clockPolarity)
        );

    //Disable the USCI Module
    HWREGB(baseAddress + OFS_UCAxCTL1) |= UCSWRST;

    //Reset OFS_UCAxCTL0 values
    HWREGB(baseAddress + OFS_UCAxCTL0) &= ~(UCCKPH + UCCKPL + UC7BIT + UCMSB);


    //Reset OFS_UCAxCTL1 values
    HWREGB(baseAddress + OFS_UCAxCTL1) &= ~(UCSSEL_3);

    //Select Clock
    HWREGB(baseAddress + OFS_UCAxCTL1) |= selectClockSource;


    HWREGB(baseAddress + OFS_UCAxBRW) =
        (unsigned int)(clockSourceFrequency / desiredSpiClock);


    /*
     * Configure as SPI master mode.
     * Clock phase select, polarity, msb
     * UCMST = Master mode
     * UCSYNC = Synchronous mode
     * UCMODE_0 = 3-pin SPI
     */
    HWREGB(baseAddress + OFS_UCAxCTL0) |= (
        msbFirst +
        clockPhase +
        clockPolarity +
        UCMST +
        UCSYNC +
        UCMODE_0
        );
    //No modulation
    HWREGB(baseAddress + OFS_UCAxMCTL) = 0;

    return ( STATUS_SUCCESS) ;
}

//*****************************************************************************
//
//! Initializes the SPI Master clock.At the end of this function call, SPI 
//! module is left enabled.
//!
//! \param baseAddress is the base address of the I2C Master module.
//! \param clockSourceFrequency is the frequency of the slected clock source
//! \param desiredSpiClock is the desired clock rate for SPI communication
//!
//! \return None
//
//*****************************************************************************
void SPI_masterChangeClock (unsigned int baseAddress,
    unsigned long clockSourceFrequency,
    unsigned long desiredSpiClock
    )
{
  //Disable the USCI Module
  HWREGB(baseAddress + OFS_UCAxCTL1) |= UCSWRST;

  HWREGB(baseAddress + OFS_UCAxBRW) =
        (unsigned int)(clockSourceFrequency / desiredSpiClock);
  
  //Reset the UCSWRST bit to enable the USCI Module
  HWREGB(baseAddress + OFS_UCAxCTL1) &= ~(UCSWRST);
}

//*****************************************************************************
//
//! Initializes the SPI Slave block.
//!
//! \param baseAddress is the base address of the SPI Slave module.
//! \param msbFirst controls the direction of the receive and transmit shift
//!      register. Valid values are
//!         \b SPI_MSB_FIRST
//!         \b SPI_LSB_FIRST [Default Value]
//! \param clockPhase is clock phase select.
//!         Valid values are
//!         \b SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT [Default Value]
//!         \b SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT
//! \param clockPolarity_InactivityHIGH is clock polarity select.
//!         Valid values are
//!         \b SPI_CLOCKPOLARITY_INACTIVITY_HIGH
//!         \b SPI_CLOCKPOLARITY_INACTIVITY_LOW [Default Value]
//! Upon successful initialization of the SPI slave block, this function
//! will have initailized the slave block, but the SPI Slave block
//! still remains disabled and must be enabled with SPI_enable()
//!
//! Modified bits are \b UCMSB, \b UC7BIT, \b UCMST, \b UCCKPL, \b UCCKPH, 
//!           \b UCMODE_3 bits of \b UCAxCTL0 and \b UCSWRST bits of \b UCAxCTL1
//!
//! \return STATUS_SUCCESS
//*****************************************************************************
unsigned short SPI_slaveInit (unsigned int baseAddress,
    unsigned char msbFirst,
    unsigned char clockPhase,
    unsigned char clockPolarity
    )
{
    ASSERT(
        (SPI_MSB_FIRST == msbFirst) ||
        (SPI_LSB_FIRST == msbFirst)
        );

    ASSERT(
        (SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT == clockPhase) ||
        (SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT == clockPhase)
        );

    ASSERT(
        (SPI_CLOCKPOLARITY_INACTIVITY_HIGH == clockPolarity) ||
        (SPI_CLOCKPOLARITY_INACTIVITY_LOW == clockPolarity)
        );

    //Disable USCI Module
    HWREGB(baseAddress + OFS_UCAxCTL1)  |= UCSWRST;

    //Reset OFS_UCAxCTL0 register
    HWREGB(baseAddress + OFS_UCAxCTL0) &= ~(UCMSB +
                                            UC7BIT +
                                            UCMST +
                                            UCCKPL +
                                            UCCKPH +
                                            UCMODE_3
                                            );


    //Clock polarity, phase select, msbFirst, SYNC, Mode0
    HWREGB(baseAddress + OFS_UCAxCTL0) |= ( clockPhase +
                                            clockPolarity +
                                            msbFirst +
                                            UCSYNC +
                                            UCMODE_0
                                            );


    return ( STATUS_SUCCESS) ;
}


//*****************************************************************************
//
//! Changes the SPI colock phase and polarity.At the end of this function call, 
//! SPI module is left enabled.
//!
//! \param baseAddress is the base address of the I2C Master module.
//! \param clockPhase is clock phase select.
//!         Valid values are
//!         \b SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT  [Default Value]
//!         \b SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT
//! \param clockPolarity_InactivityHIGH is clock polarity select.
//!         Valid values are
//!         \b SPI_CLOCKPOLARITY_INACTIVITY_HIGH
//!         \b SPI_CLOCKPOLARITY_INACTIVITY_LOW  [Default Value]
//!
//! \return None
//
//*****************************************************************************
void SPI_changeClockPhasePolarity (unsigned int baseAddress,
    unsigned char clockPhase,
    unsigned char clockPolarity
    )
{
  
   ASSERT(  (SPI_CLOCKPOLARITY_INACTIVITY_HIGH == clockPolarity) ||
        (SPI_CLOCKPOLARITY_INACTIVITY_LOW == clockPolarity)
        );

  //Disable the USCI Module
  HWREGB(baseAddress + OFS_UCAxCTL1) |= UCSWRST;
  
  HWREGB(baseAddress + OFS_UCAxCTL0) &= ~(UCCKPH + UCCKPL);

  HWREGB(baseAddress + OFS_UCAxCTL0) |= (
        clockPhase +
        clockPolarity
          );
  
  //Reset the UCSWRST bit to enable the USCI Module
  HWREGB(baseAddress + OFS_UCAxCTL1) &= ~(UCSWRST);
}

//*****************************************************************************
//
//! Transmits a byte from the SPI Module.
//!
//! \param baseAddress is the base address of the SPI module.
//! \param transmitData data to be transmitted from the SPI module
//!
//! This function will place the supplied data into SPI trasmit data register
//! to start transmission
//!
//! \return None.
//
//*****************************************************************************
void SPI_transmitData ( unsigned int baseAddress,
    unsigned char transmitData
    )
{
    HWREGB(baseAddress + OFS_UCAxTXBUF) = transmitData;
}

//*****************************************************************************
//
//! Receives a byte that has been sent to the SPI Module.
//!
//! \param baseAddress is the base address of the SPI module.
//!
//! This function reads a byte of data from the SPI receive data Register.
//!
//! \return Returns the byte received from by the SPI module, cast as an
//! unsigned char.
//
//*****************************************************************************
unsigned char SPI_receiveData (unsigned int baseAddress)
{
    return ( HWREGB(baseAddress + OFS_UCAxRXBUF)) ;
}

//*****************************************************************************
//
//! Enables individual SPI interrupt sources.
//!
//! \param baseAddress is the base address of the SPI module.
//! \param mask is the bit mask of the interrupt sources to be enabled.
//!
//! Enables the indicated SPI interrupt sources.  Only the sources that
//! are enabled can be reflected to the processor interrupt; disabled sources
//! have no effect on the processor.
//!
//! The mask parameter is the logical OR of any of the following:
//! - \b SPI_RECEIVE_INTERRUPT -Receive interrupt
//! - \b SPI_TRANSMIT_INTERRUPT - Transmit interrupt
//!
//! Modified registers are \b UCAxIFG and \b UCAxIE
//!
//! \return None.
//
//*****************************************************************************
void SPI_enableInterrupt (unsigned int baseAddress,
    unsigned char mask
    )
{
    ASSERT( (SPI_RECEIVE_INTERRUPT == mask) ||
        (SPI_TRANSMIT_INTERRUPT == mask)
        );

    HWREGB(baseAddress + OFS_UCAxIFG) &=  ~mask;
    HWREGB(baseAddress + OFS_UCAxIE) |= mask;
}

//*****************************************************************************
//
//! Disables individual SPI interrupt sources.
//!
//! \param baseAddress is the base address of the SPI module.
//! \param mask is the bit mask of the interrupt sources to be
//! disabled.
//!
//! Disables the indicated SPI interrupt sources.  Only the sources that
//! are enabled can be reflected to the processor interrupt; disabled sources
//! have no effect on the processor.
//!
//! The mask parameter is the logical OR of any of the following:
//! - \b SPI_RECEIVE_INTERRUPT -Receive interrupt
//! - \b SPI_TRANSMIT_INTERRUPT - Transmit interrupt
//!
//! Modified register is \b UCAxIE
//!
//! \return None.
//
//*****************************************************************************
void SPI_disableInterrupt (unsigned int baseAddress,
    unsigned char mask
    )
{
    ASSERT(  (SPI_RECEIVE_INTERRUPT == mask) ||
        (SPI_TRANSMIT_INTERRUPT == mask)
        );

    HWREGB(baseAddress + OFS_UCAxIE) &= ~mask;
}

//*****************************************************************************
//
//! Gets the current SPI interrupt status.
//!
//! \param baseAddress is the base address of the SPI module.
//! \param mask is the masked interrupt flag status to be returned.
//!
//! This returns the interrupt status for the SPI  module based on which
//! flag is passed. mask parameter can be either any of the following
//! selection.
//! - \b SPI_RECEIVE_INTERRUPT -Receive interrupt
//! - \b SPI_TRANSMIT_INTERRUPT - Transmit interrupt
//!
//! Modified registers are \b UCAxIFG.
//!
//! \return The current interrupt status as the mask of the set flags
//
//*****************************************************************************
unsigned char SPI_getInterruptStatus (unsigned int baseAddress,
    unsigned char mask
    )
{
    ASSERT(
        (SPI_RECEIVE_INTERRUPT == mask) ||
        (SPI_TRANSMIT_INTERRUPT == mask)
        );

    return ( HWREGB(baseAddress + OFS_UCAxIFG) & mask );
}

//*****************************************************************************
//
//! Clears the selected SPI interrupt status flag.
//!
//! \param baseAddress is the base address of the SPI module.
//! \param mask is the masked interrupt flag to be cleared.
//!
//! The mask parameter is the logical OR of any of the following:
//! - \b SPI_RECEIVE_INTERRUPT -Receive interrupt
//! - \b SPI_TRANSMIT_INTERRUPT - Transmit interrupt
//!
//! Modified registers are \b UCAxIFG.
//!
//! \return None
//
//*****************************************************************************
void SPI_clearInterruptFlag (unsigned int baseAddress,
    unsigned char mask
    )
{
    ASSERT(
        (SPI_RECEIVE_INTERRUPT == mask) ||
        (SPI_TRANSMIT_INTERRUPT == mask)
        );

    HWREGB(baseAddress + OFS_UCAxIFG) &=  ~mask;
}

//*****************************************************************************
//
//! Enables the SPI block.
//!
//! \param baseAddress is the base address of the USCI SPI module.
//!
//! This will enable operation of the SPI block.
//! Modified bits are \b UCSWRST bit of \b UCAxCTL1 register.
//!
//! \return None.
//
//*****************************************************************************
void SPI_enable (unsigned int baseAddress)
{
    //Reset the UCSWRST bit to enable the USCI Module
    HWREGB(baseAddress + OFS_UCAxCTL1) &= ~(UCSWRST);
}

//*****************************************************************************
//
//! Disables the SPI block.
//!
//! \param baseAddress is the base address of the USCI SPI module.
//!
//! This will disable operation of the SPI block.
//!
//! Modified bits are \b UCSWRST bit of \b UCAxCTL1 register.
//!
//! \return None.
//
//*****************************************************************************
void SPI_disable (unsigned int baseAddress)
{
    //Set the UCSWRST bit to disable the USCI Module
    HWREGB(baseAddress + OFS_UCAxCTL1) |= UCSWRST;
}

//*****************************************************************************
//
//! Returns the address of the RX Buffer of the SPI for the DMA module.
//!
//! \param baseAddress is the base address of the SPI module.
//!
//! Returns the address of the SPI RX Buffer. This can be used in conjunction
//! with the DMA to store the received data directly to memory.
//!
//! \return None
//
//*****************************************************************************
unsigned long SPI_getReceiveBufferAddressForDMA (unsigned int baseAddress)
{
    return ( baseAddress + OFS_UCAxRXBUF );
}

//*****************************************************************************
//
//! Returns the address of the TX Buffer of the SPI for the DMA module.
//!
//! \param baseAddress is the base address of the SPI module.
//!
//! Returns the address of the SPI TX Buffer. This can be used in conjunction
//! with the DMA to obtain transmitted data directly from memory.
//!
//! \return None
//
//*****************************************************************************
unsigned long SPI_getTransmitBufferAddressForDMA (unsigned int baseAddress)
{
    return ( baseAddress + OFS_UCAxTXBUF );
}

//*****************************************************************************
//
//! Indicates whether or not the SPI bus is busy.
//!
//! \param baseAddress is the base address of the SPI module.
//!
//! This function returns an indication of whether or not the SPI bus is
//! busy.This function checks the status of the bus via UCBBUSY bit
//!
//! \return SPI_BUSY if the SPI module trasmitting or receiving
//! is busy; otherwise, returns SPI_NOT_BUSY.
//
//*****************************************************************************
unsigned char SPI_isBusy (unsigned int baseAddress)
{
    //Return the bus busy status.
    return (HWREGB(baseAddress + OFS_UCAxSTAT) & UCBBUSY);
}

//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************

