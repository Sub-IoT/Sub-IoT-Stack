/*
 * Created on: September 6, 2013
 * Authors:
 * 		jan.stevens@ieee.org
 *
 * 	Testing out the MSP430F5172 without the RF part.
 *
 */
#include <string.h>
#include <trans/trans.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/spi.h>
#include <framework/log.h>
#include <framework/timer.h>
#include <msp430.h> 

/*
 * main.c
 */
unsigned char MST_Data,SLV_Data;
unsigned char test_data;
int main(void) {
    // Initialize the OSS-7 Stack
	system_init();
	spi_enable_interrupt();
	trans_init();
	log_print_clean("We have started");

	//system_watchdog_init(WDTSSEL0, 0x03);
	//system_watchdog_timer_start();
	P1DIR |= BIT4;
	MST_Data = 0x01;                          // Initialize data values
	SLV_Data = 0x00;
	spi_transmit_data(MST_Data);
	while(1)
	{
		//led_on(1);

		log_print_clean("We have started");
		//system_lowpower_mode(4,1);
	}
}

#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
  volatile unsigned int i;

  switch(__even_in_range(UCB0IV,4))
  {
    case 0: break;                          // Vector 0 - no interrupt
    case 2:                                 // Vector 2 - RXIFG
      while (!(UCB0IFG&UCTXIFG));           // USCI_A0 TX buffer ready?
      test_data = UCB0RXBUF;
      if (UCB0RXBUF==SLV_Data)              // Test for correct character RX'd
        P1OUT |= BIT4;                      // If correct, P1.4 high
      else
        P1OUT &= ~BIT4;                     // If incorrect, P1.4 low

      MST_Data++;                           // Increment data
      SLV_Data++;
      UCB0TXBUF = MST_Data;                 // Send next value

      for(i = 20; i>0; i--);                // Add time between transmissions to
                                            // make sure slave can process information
      break;
    case 4: break;                          // Vector 4 - TXIFG
    default: break;
  }
}
