#include <msp430.h>

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 			// Stop WDT
	P1DIR |= BIT7;                            			// P1.7 output

	TA1CCTL0 = CCIE;                          			// Timer_A1 Capture/Compare Control n Register - Capture/compare interrupt enable

	TA1CCR0 = 0xFFFF;									//Timer_A Capture/Compare n Register (Count from 0 to value in CCR0)
	TA1CTL = TASSEL_2 + ID__8 + MC_2 + TACLR;         	// SMCLK, Input divider /8, Up mode, Clear Timer A Register

	__bis_SR_register(LPM0_bits + GIE);       			// Enter LPM0, enable interrupts
	__no_operation();                         			// For debugger
}

// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	P1OUT ^= BIT7;                            			// Toggle P1.7
}

