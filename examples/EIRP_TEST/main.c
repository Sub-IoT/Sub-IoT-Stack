/*
 * Program to do Packet Delivery Ratio testing
 * To execute a test:
 * 	- program this firmware to 2 tags
 * 	- attach one tag to a serial port and run the pdr_logger.py script
 * 	- put this tag in rx mode by pressing button 1
 * 	- put the other tag in tx mode by pressing button 3
 *
 *  Created on: Feb 8, 2013
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */


#include <string.h>

#include <dll/dll.h>
#include <trans/trans.h>
#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>

#define SYNC_WORD 0xCE

#define CHANNEL_ID	0x25

#define INTERRUPT_BUTTON1 	(1)
#define INTERRUPT_BUTTON2 	(1 << 1)
#define INTERRUPT_BUTTON3 	(1 << 2)
#define INTERRUPT_RTC 		(1 << 3)

typedef enum {
	mode_idle,
	mode_tx,
	mode_rx
} mode_t;

static u16 counter = 0;
static u8 interrupt_flags = 0;
static mode_t mode = mode_idle;
static s8 tx_eirp[40] = {9.9, 8.8, 7.8, 6.8, 5.8, 4.8, 3.6, 2.0, 0.1, -1.1, -2.1, -3.1, -4.1,
						-5.3, -6.0, -7.1, -8.3, -10.1, -11.4, -12.3, -13.3, -14.3, -15.5, -16.2
						-17, -18.8, -20.4, -21, -22.5, -23.3, -24.3, -25.3, -26.5, -27.9, -29.5,
						-31.4, -33.8, -36.5, -38.3, -62.7};
static s8 used_eirp = 10;
int i = 0;

void start_tx()
{
	led_on(3);
	trans_tx_foreground_frame((u8*)&counter, sizeof(counter), CHANNEL_ID, used_eirp);
}

void tx_callback(Trans_Tx_Result result)
{
	switch(result){
			case TransPacketSent:
				log_print_string("TX successful");
				counter++; // increment even if CCA failed
				led_off(3);
				break;
			case TransTCAFail:
				log_print_string("T_CA is less than the T_G");
				break;
			case TransPacketFail:
				log_print_string("Problem with the CCA");
				counter++; // increment even if CCA failed
				led_toggle(2);
				break;
		}

}

void main(void) {
	system_init();

	button_enable_interrupts();

	rtc_init_counter_mode();
	rtc_start();

	trans_init();
	trans_set_tx_callback(&tx_callback);


	while(1)
	{
		if(INTERRUPT_BUTTON1 & interrupt_flags)
		{
		  	interrupt_flags &= ~INTERRUPT_BUTTON1;
		  	if (i > 40)
		  		i = 0;

		  	used_eirp = tx_eirp[i++];

		    button_clear_interrupt_flag();
		    button_enable_interrupts();
		}


        if (INTERRUPT_BUTTON3 & interrupt_flags)
        {
        	interrupt_flags &= ~INTERRUPT_BUTTON3;

        	if (mode == mode_tx)
			{
        		mode = mode_idle;
				rtc_disable_interrupt();
			} else {
				mode = mode_tx;
				rtc_enable_interrupt();
			}

        	button_clear_interrupt_flag();
        	button_enable_interrupts();
        }

        if (INTERRUPT_RTC & interrupt_flags)
		{
        	start_tx();

			interrupt_flags &= ~INTERRUPT_RTC;
		}

		system_lowpower_mode(4,1);
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR (void)
{
	if(button_is_active(1))
		interrupt_flags |= INTERRUPT_BUTTON1;
	else
		interrupt_flags &= ~INTERRUPT_BUTTON1;

	if(button_is_active(3))
		interrupt_flags |= INTERRUPT_BUTTON3;
	else
		interrupt_flags &= ~INTERRUPT_BUTTON3;

	if(interrupt_flags != 0)
	{
		button_disable_interrupts();
		// TODO: debouncing
		LPM4_EXIT;
	}
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR (void)
{
	if(button_is_active(1))
		interrupt_flags |= INTERRUPT_BUTTON1;
	else
		interrupt_flags &= ~INTERRUPT_BUTTON1;

	if(button_is_active(3))
		interrupt_flags |= INTERRUPT_BUTTON3;
	else
		interrupt_flags &= ~INTERRUPT_BUTTON3;

	if(interrupt_flags != 0)
	{
		button_disable_interrupts();
		LPM4_EXIT;
	}
}

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR (void)
{
    switch (RTCIV){
        case 0: break;  //No interrupts
        case 2: break;  //RTCRDYIFG
        case 4:         //RTCEVIFG
        	interrupt_flags |= INTERRUPT_RTC;
        	LPM4_EXIT;
            break;
        case 6: break;  //RTCAIFG
        case 8: break;  //RT0PSIFG
        case 10: break; //RT1PSIFG
        default: break;
    }
}
