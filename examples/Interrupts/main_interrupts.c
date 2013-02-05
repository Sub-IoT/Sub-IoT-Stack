#include <msp430.h>

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

  P1DIR |= BIT7;                            // Set P1.7 to output direction
  P1REN |= BIT6;                            // Enable P1.6 internal resistance
  P1OUT |= BIT6;                            // Set P1.6 as pull-Up resistance
  P1IE |= BIT6;                             // P1.6 interrupt enabled
  P1IES |= BIT6;                            // P1.6 Hi/Lo edge
  P1IFG &= ~BIT6;                           // P1.6 IFG cleared

  __bis_SR_register(LPM4_bits + GIE);       // Enter LPM4 w/interrupt
  __no_operation();                         // For debugger
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  P1OUT ^= BIT7;                            // P1.0 = toggle
  P1IFG &= ~BIT6;                          // P1.4 IFG cleared
}

