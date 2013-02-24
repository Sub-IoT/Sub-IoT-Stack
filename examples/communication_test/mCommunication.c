/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *
 */

#include <trans/trans.h>
#include <string.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <log.h>

u8 dataSend = 0;

#define INTERRUPT_BUTTON1 	(1)
static u8 interrupt_flags = 0;



void start_tx()
{
		trans_tx_foreground_frame(&dataSend, sizeof(dataSend), 0x18, 0x00);
		led_toggle(3);
}

void tx_callback(Trans_Tx_Result result)
{
	switch(result){
			case TransPacketSent:
				log_print_string("TX successful");
				led_toggle(1);
				break;
			case TransTCAFail:
				log_print_string("T_CA is less than the T_G");
				led_toggle(2);
				break;
			case TransPacketFail:
				log_print_string("Problem with the CCA");

				break;
		}

}


void main(void) {
	system_init();
	button_enable_interrupts();

	trans_init();
	trans_set_tx_callback(&tx_callback);

	while(1)
		{
			if(INTERRUPT_BUTTON1 & interrupt_flags)
			{
				interrupt_flags &= ~INTERRUPT_BUTTON1;

		        start_tx();

		        button_clear_interrupt_flag();
		        button_enable_interrupts();
		     }
			system_lowpower_mode(4, 1);
		}
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR (void)
{
	if(button_is_active(1))
		interrupt_flags |= INTERRUPT_BUTTON1;
	else
		interrupt_flags &= ~INTERRUPT_BUTTON1;

	if(interrupt_flags != 0)
	{
		button_disable_interrupts();
		// TODO: debouncing
		LPM4_EXIT;
	}
}

