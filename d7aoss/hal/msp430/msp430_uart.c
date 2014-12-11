/*
 *  Created on: Nov 22, 2012
 *  Authors:
 *      maarten.weyn@artesis.be
 *      glenn.ergeerts@artesis.be
 *      jan.stevens@ieee.org
 */

#include "../uart.h"
#include "platforms/platform.h"
#include "../system.h"

#include <msp430.h>
#include "driverlib/5xx_6xx/gpio.h"
#include "driverlib/5xx_6xx/uart.h"

void uart_init()
{
#ifdef PLATFORM_MSP430
    PMAPPWD = 0x02D52;                                            // Get write-access to port mapping regs
    P1DIR |= BIT6 + BIT7;
    P1MAP7 = PM_UCA0RXD;                      // Map UCA0RXD output to P1.6
    P1MAP6 = PM_UCA0TXD;                      // Map UCA0TXD output to P1.7
    PMAPPWD = 0;                              // Lock port mapping registers
    P1DIR |= BIT1;                            // Set P1.6 as TX output
    P1SEL |= BIT6 + BIT7;                     // Select P1.5 & P1.6 to UART function
#endif

    /*
     * CUBR1 = UARTCLK/(Baudr*256)
     * CUBR0 = (UARTCLK/Baudr)256*CUBR1
     */

    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK

    UCA0BR1 = clock_speed / (BAUDRATE*256);
    UCA0BR0 = ((clock_speed + (BAUDRATE/2))  / BAUDRATE) - (256*UCA0BR1);

//    UCA0BR0 = 9;                              // 1MHz 115200 (see User's Guide)
//    UCA0BR1 = 0;                              // 1MHz 115200

    UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

}

void uart_enable_interrupt()
{
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

#pragma NO_HOOKS(uart_transmit_data)
void uart_transmit_data(unsigned char data)
{
        while(!uart_tx_ready());
    UCA0TXBUF = data;
}

#pragma NO_HOOKS(uart_transmit_message)
void uart_transmit_message(unsigned char *data, unsigned char length)
{
    unsigned char i=0;
    for (; i<length; i++)
    {
        uart_transmit_data(data[i]);
    }

}

#pragma NO_HOOKS(uart_tx_ready)
unsigned char uart_tx_ready()
{
    return UCA0IFG&UCTXIFG;
}

unsigned char uart_receive_data()
{
    return UCA0RXBUF;
}
