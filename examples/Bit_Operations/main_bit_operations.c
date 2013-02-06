#include <msp430.h> 

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
								//0x80 = 0b10000000
    P1DIR = 0x80;				//Select P1.7 as output
    P1OUT = 0x80;				//Drive P1.7 high
    P1OUT &= ~0x80;				//Bitwise AND operation of one's complement of 0x80
    							//0x80 = BIT7
    P1OUT |= BIT7;				//Bitwise OR

    P1OUT ^= BIT7;				//Bitwise EX-OR --> Toggle

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
    __no_operation();                         // For debugger
}
