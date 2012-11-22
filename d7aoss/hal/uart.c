/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "uart.h"

#include <msp430.h>
#include "driverlib/5xx_6xx/gpio.h"
#include "driverlib/5xx_6xx/uart.h"

void Uart_Init()
{
    PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
    P2MAP0 = PM_UCA0RXD;                      // Map UCA0RXD output to P2.6
    P2MAP1 = PM_UCA0TXD;                      // Map UCA0TXD output to P2.7
    PMAPPWD = 0;                              // Lock port mapping registers

    P2DIR |= BIT1;                            // Set P2.7 as TX output
    P2SEL |= BIT0 + BIT1;                     // Select P2.6 & P2.7 to UART function

    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 9;                              // 1MHz 115200 (see User's Guide)
    UCA0BR1 = 0;                              // 1MHz 115200
    UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**


}

void Uart_EnableInterrupt()
{    
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}


void Uart_TransmitData(unsigned char data)
{
	while(!Uart_TxReady());
    UCA0TXBUF = data;
}


void Uart_TransmitMessage(unsigned char *data, unsigned char length)
{
    unsigned char i=0;
    for (; i<length; i++)
    {
        Uart_TransmitData(data[i]);
    }

}

unsigned char Uart_TxReady()
{
    return UCA0IFG&UCTXIFG;
}

unsigned char Uart_ReceiveData()
{
    return UCA0RXBUF;
}
