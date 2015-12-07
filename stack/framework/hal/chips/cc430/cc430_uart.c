/*!
 *
 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *     	glenn.ergeerts@uantwerpen.be
 *
 */
#include "hwuart.h"
#include <cc430f5137.h>
//#include "driverlib/5xx_6xx/uart.h"

//cc430f5137
// UART
// RX: P2.0
// TX: P2.1

#define PLATFORM_UCA0RXD	P2MAP0
#define PLATFORM_UCA0TXD	P2MAP1

#define PLATFORM_PxDIR 		P2DIR
#define PLATFORM_PxDIRBIT	BIT1
#define PLATFORM_PxSEL		P2SEL
#define PLATFORM_PxSELBIT	BIT0 + BIT1

#define BAUDRATE 115200
#define clock_speed 4000000

void uart_init()
{
    PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
    PLATFORM_UCA0RXD = PM_UCA0RXD;            // Map UCA0RXD output to Px
    PLATFORM_UCA0TXD = PM_UCA0TXD;            // Map UCA0TXD output to Px
    PMAPPWD = 0;                              // Lock port mapping registers
    PLATFORM_PxDIR |= PLATFORM_PxDIRBIT;      // Set Px as TX output
    PLATFORM_PxSEL |= PLATFORM_PxSELBIT;
    PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA0MCTL |= UCBRS_6+UCBRF_0;              // Modulation UCBRSx=6, UCBRFx=0 // TODO hardcoded for current clock speed and baudrate
    UCA0CTL1 |= UCSSEL_2;
    UCA0BRW = 34;                             // TODO hardcoded for current clock speed and baudrate
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}

void uart_enable_interrupt()
{
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

void uart_transmit_data(int8_t data)
{
    while(!(UCA0IFG & UCTXIFG));

    UCA0TXBUF = data;
}

void uart_transmit_message(void const *data, size_t length)
{
    unsigned char i=0;
    for (; i<length; i++)
    {
        uart_transmit_data(((char const*)data)[i]);
    }
}
