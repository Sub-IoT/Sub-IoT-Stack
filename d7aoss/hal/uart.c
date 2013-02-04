/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "uart.h"
#include "platforms/platform.h"

#include <msp430.h>
#include "driverlib/5xx_6xx/gpio.h"
#include "driverlib/5xx_6xx/uart.h"

void uart_init()
{
    PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
    UART_RX = PM_UCA0RXD;                      // Map UCA0RXD output to P2.6
    UART_TX = PM_UCA0TXD;                      // Map UCA0TXD output to P2.7
    PMAPPWD = 0;                              // Lock port mapping registers

    P2DIR |= BIT5;                            // Set P1.6 as TX output
    P2SEL |= BIT5 + BIT6;                     // Select P1.5 & P1.6 to UART function

    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 9;                              // 1MHz 115200 (see User's Guide)
    UCA0BR1 = 0;                              // 1MHz 115200
    UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**


}

void uart_enable_interrupt()
{    
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}


void uart_transmit_data(unsigned char data)
{
	while(!uart_tx_ready());
    UCA0TXBUF = data;
}


void uart_transmit_message(unsigned char *data, unsigned char length)
{
    unsigned char i=0;
    for (; i<length; i++)
    {
        uart_transmit_data(data[i]);
    }

}

unsigned char uart_tx_ready()
{
    return UCA0IFG&UCTXIFG;
}

unsigned char uart_receive_data()
{
    return UCA0RXBUF;
}
