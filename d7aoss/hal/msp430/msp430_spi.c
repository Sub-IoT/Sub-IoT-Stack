/*
 * msp430_spi.c
 *
 *  Created on: Sep 12, 2013
 *      Author: jan.stevens@ieee.org
 */

#include "../spi.h"
#include "platforms/platform.h"
#include "../system.h"

#include <msp430.h>
#include "driverlib/5xx_6xx/gpio.h"
//#include "driverlib/5xx_6xx/spi.h"

void spi_init()
{
#ifdef PLATFORM_MSP430
	  P1DIR |= BIT3;
	  PMAPPWD = 0x02D52;
	  P1DIR |= BIT1 + BIT2 + BIT0;
	  P1MAP0 = PM_UCB0CLK;
	  P1MAP1 = PM_UCB0SIMO;
	  P1MAP2 = PM_UCB0SOMO;
	  PMAPPWD = 0;

	  P1OUT |= BIT3;
	  P1OUT |= 0x00; 			// Slave select - low
	  P1SEL |= 0x07;
#endif
	UCB0CTL1 |= UCSWRST;		// Put state machine in reset
	UCB0CTL0 |= UCMST + UCSYNC + UCCKPL + UCMSB; // 3-pin, 8-bit SPI Master
									// Clock polarity high, MSB
	UCB0CTL1 |= UCSSEL_2;
	//TODO: select a meaningful data rate for SPI
	//UCB0BR1 = 0;
	//UCB0BR0 = 9;
	UCB0BR1 = 0x02;
	UCB0BR0 = 0;

	UCB0CTL1 &= ~UCSWRST;		// Initialize the state machine
	P1OUT &= ~BIT3;             // Now with SPI signals initialized,
	P1OUT |= BIT3;				// reset slave
}

void spi_enable_interrupt()
{
	UCB0IE |= UCRXIE;
}

#pragma NO_HOOKS(spi_transmit_data)
void spi_transmit_data(unsigned char data)
{
	while(!spi_tx_ready());
	UCB0TXBUF = data;
}

#pragma NO_HOOKS(spi_transmit_message)
void spi_transmit_message(unsigned char *data, unsigned char length)
{
	unsigned char i = 0;
	for(; i < length; i++)
	{
		spi_transmit_data(data[i]);
	}
}
#pragma NO_HOOKS(spi_tx_ready)
unsigned char spi_tx_ready()
{
	return UCB0IFG&UCTXIFG;
}

unsigned char spi_receive_data()
{
	return UCB0RXBUF;
}
