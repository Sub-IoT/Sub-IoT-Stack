/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */


#include <string.h>

#include <phy/phy.h>

#include <hal/system.h>
#include <hal/button.h>
#include <hal/leds.h>
#include <hal/rtc.h>
#include <log.h>

#define INTERRUPT_BUTTON1 	(1)
#define INTERRUPT_BUTTON2 	(1 << 1)
#define INTERRUPT_BUTTON3 	(1 << 2)
#define INTERRUPT_RTC 		(1 << 3)

#define PACKET_LEN 19

static u8 packet[PACKET_LEN] = { 0x13, 0x50, 0xF1, 0x20, 0x59, 0x40, 0x46, 0x93, 0x21, 0xAB, 0x00, 0x31, 0x00, 0x24, 0x00, 0x00, 0x00, 0x01, 0x01 };

static u8 interrupt_flags = 0;
static u8 rtcEnabled = 0;

phy_rx_cfg_t rx_cfg = {
		0, // timeout
		0, // multiple TODO
		0x10, // spectrum ID TODO
		0, // coding scheme TODO
		0, // RSSI min filter TODO
		0x01  // sync word class TODO
};

phy_tx_cfg_t tx_cfg = {
	    0x10, // spectrum ID
		0, // coding scheme
		0, // sync word class
	    packet,
	    PACKET_LEN
};

void start_rx()
{
	phy_rx_start(&rx_cfg); // TODO remove (use timeout/multiple)
	Led_On(2);
}

void stop_rx()
{
	phy_rx_stop();
	Led_Off(2);
}

void tx_callback()
{
	Led_Off(3);
	start_rx(); // go back to rx mode
}

void rx_callback(phy_rx_res_t* res)
{
	Log_Packet(res->data); // TODO other params
	if(memcmp(res->data, packet, res->len - 2) != 0) //exclude CRC bytes in check
	{
		__no_operation(); // TODO assert
		Log_PrintString("!!! unexpected packet data", 26);
		Led_Off(3);
		Led_Toggle(1);
	}
	else
	{
		char text[14] = "RX OK     dBm";
		u8 i = 6;
		if (res->rssi < 0)
		{
			text[i++] = '-';
			res->rssi  *= -1;
		}
		if (res->rssi  >= 100)
		{
			text[i++] = '1';
			res->rssi  -= 100;
		}

		text[i++] = 0x30 + (res->rssi / 10);
		text[i++] = 0x30 + (res->rssi % 10);
		Log_PrintString((char*)&text, 14);
		Led_Toggle(3);
	}

	start_rx();
}

void main(void) {
	System_Init();
	Buttons_EnableInterrupts();

	Rtc_InitCounterMode();
	Rtc_Start();

	phy_init();
	phy_set_tx_callback(&tx_callback);
	phy_set_rx_callback(&rx_callback);

	start_rx();

	while(1)
	{
        if(INTERRUPT_BUTTON1 & interrupt_flags)
        {
        	interrupt_flags &= ~INTERRUPT_BUTTON1;
        	Led_On(3);

        	if(phy_is_rx_in_progress() == true)
        		stop_rx();

        	phy_tx(&tx_cfg);

        	Buttons_ClearInterruptFlag();
        	Buttons_EnableInterrupts();
        }

        if (INTERRUPT_BUTTON3 & interrupt_flags)
        {
        	interrupt_flags &= ~INTERRUPT_BUTTON3;

        	if (rtcEnabled)
			{
				rtcEnabled = 0;
				Rtc_DisableInterrupt();
				start_rx();
			} else {
				rtcEnabled = 1;
				Rtc_EnableInterrupt();
				stop_rx();
			}

        	Buttons_ClearInterruptFlag();
        	Buttons_EnableInterrupts();
        }

        if (INTERRUPT_RTC & interrupt_flags)
		{
			Led_On(3);
			stop_rx();
			phy_tx(&tx_cfg);
			interrupt_flags &= ~INTERRUPT_RTC;
		}

		System_LowPowerMode(4,1);
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR (void)
{
	if(Button_IsActive(1))
		interrupt_flags |= INTERRUPT_BUTTON1;
	else
		interrupt_flags &= ~INTERRUPT_BUTTON1;

	if(Button_IsActive(3))
		interrupt_flags |= INTERRUPT_BUTTON3;
	else
		interrupt_flags &= ~INTERRUPT_BUTTON3;

	if(interrupt_flags != 0)
	{
		Buttons_DisableInterrupts();
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
