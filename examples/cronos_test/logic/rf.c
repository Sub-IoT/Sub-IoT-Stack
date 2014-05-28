/*
 * rf.c
 *
 *  Created on: 7-dec.-2012
 *      Author: Maarten Weyn
 */

#include "rf.h"

#include "../driver/display.h"
#include "../driver/ports.h"

#include "altitude.h"
#include "temperature.h"
#include "battery.h"
#include "user.h"

#include <framework/timer.h>

static timer_event event;
static uint8_t tx = 0;
static uint8_t blinking = 0;
static uint8_t data[32];
static uint8_t dataLength = 29;

void send_beacon(void* ar)
{
	if (!tx && blinking)
	{
		tx = 1;

		display_symbol(LCD_ICON_BEEPER2, SEG_ON);
		display_symbol(LCD_ICON_BEEPER3, SEG_ON);

		data[1] = (uint8_t) (sBatt.voltage>> 8);
		data[2] = (uint8_t) sBatt.voltage;

		timer_add_event(&event);
		trans_tx_foreground_frame(data, dataLength, 0xFF, beacon_channel, txPower);
	}
}

void tx_callback(Trans_Tx_Result result)
{
	if(result == TransPacketSent)
	{
		display_symbol(LCD_ICON_BEEPER3, SEG_OFF);
	}

	display_symbol(LCD_ICON_BEEPER2, SEG_OFF);

	tx = 0;
}


void blink_init()
{
	event.next_event = beacon_interval;
	event.f = &send_beacon;

	data[0] = 0x65;
	data[1] = 0;
	data[2] = 0;
	dataLength = 3;
}


void blink_start()
{
	blinking = 1;
	display_symbol(LCD_ICON_BEEPER1, SEG_ON);
	timer_add_event(&event);

}
void blink_stop()
{
	blinking = 0;
	display_symbol(LCD_ICON_BEEPER1, SEG_OFF);
}


void sx_beacon(uint8_t line)
{
	// Exit if battery voltage is too low for radio operation
	if (sys.flag.low_battery) return;


	if (blinking)
		blink_stop();
	else
		blink_start();
}


void display_beacon(uint8_t line, uint8_t update)
{
	if (update == DISPLAY_LINE_UPDATE_FULL)
	{
		if (blinking)
			display_chars(LCD_SEG_L2_5_0, (uint8_t *)" D7 ON", SEG_ON);
		else
			display_chars(LCD_SEG_L2_5_0, (uint8_t *)" D7OFF", SEG_ON);

	}
}

/// Powerpoint

void sx_ppt(uint8_t line)
{
	// Exit if battery voltage is too low for radio operation
	if (sys.flag.low_battery) return;

  	// Start SimpliciTI in tx only mode
	//start_simpliciti_tx_only(SIMPLICITI_BUTTONS);

	display_symbol(LCD_ICON_BEEPER1, SEG_ON);
	display_symbol(LCD_ICON_BEEPER2, SEG_ON);
	display_symbol(LCD_ICON_BEEPER3, SEG_ON);
	//trans_tx_foreground_frame(data, 1, 0xFF, SEND_CHANNEL, TX_EIRP);
}

void display_ppt(uint8_t line, uint8_t update)
{
	if (update == DISPLAY_LINE_UPDATE_FULL)
	{
		display_chars(LCD_SEG_L2_5_0, (uint8_t *)"   PPT", SEG_ON);
	}
}


//Synchronization

void display_sync(uint8_t line, uint8_t update)
{
	if (update == DISPLAY_LINE_UPDATE_FULL)
	{
		display_chars(LCD_SEG_L2_5_0, (uint8_t *)"  SYNC", SEG_ON);
	}
}
void mx_beacon(uint8_t line)
{
  // Clear display
  clear_display_all();

  uint8_t select;
  int32_t  channel =  beacon_channel;
  int32_t  interval = beacon_interval / 1000;
  int32_t power = txPower;
  uint8_t * str;

  // Init value index
  select = 0;

  // Loop values until all are set or user breaks	set
  while(1)
  {
    // Idle timeout: exit without saving
    if (sys.flag.idle_timeout)
    {
      break;
    }

    // Button STAR (short): save, then exit
    if (button.flag.star)
    {
    	// Store local variables in global
    	beacon_channel 	 = channel;
    	beacon_interval = interval * 1000;
    	txPower = power;

    	break;
    }

    switch (select)
    {
    	case 0:		// Display channel
    		//clear_display_all();
			str = itoa(channel, 2, 0);
			display_chars(LCD_SEG_L1_1_0, str, SEG_ON);
			display_chars(LCD_SEG_L2_5_0, (uint8_t *)"  CHAN", SEG_ON);

			// Set hours
			set_value(&channel, 2, 0, 16, 45, SETVALUE_ROLLOVER_VALUE + SETVALUE_DISPLAY_VALUE + SETVALUE_NEXT_VALUE, LCD_SEG_L1_1_0, display_hex_value);
			select = 1;
			break;

    	case 1:		// Set interval
    		//clear_display_all();
			str = itoa(interval, 2, 0);
			display_chars(LCD_SEG_L1_1_0, str, SEG_ON);
			display_chars(LCD_SEG_L2_5_0, (uint8_t *)"  SEC", SEG_ON);

			set_value(&interval, 2, 0, 1, 65, SETVALUE_ROLLOVER_VALUE + SETVALUE_DISPLAY_VALUE + SETVALUE_NEXT_VALUE, LCD_SEG_L1_1_0, display_value1);
			select = 2;
			break;

    	case 2:		// Set power
    		//clear_display_all();
			str = itoa(power, 2, 0);
			display_chars(LCD_SEG_L1_1_0, str, SEG_ON);
			display_chars(LCD_SEG_L2_5_0, (uint8_t *)"TxDbm", SEG_ON);

			set_value(&power, 2, 0, 0, 10, SETVALUE_ROLLOVER_VALUE + SETVALUE_DISPLAY_VALUE + SETVALUE_NEXT_VALUE, LCD_SEG_L1_1_0, display_value1);
			select = 0;
			break;
    }
  }

  // Clear button flags
  button.all_flags = 0;
}


#ifndef CONFIG_USE_SYNC_TOSET_TIME
// *************************************************************************************************
// @fn          sx_sync
// @brief       Start SimpliciTI. Button DOWN connects/disconnects to access point.
// @param       u8 line		LINE2
// @return      none
// *************************************************************************************************
void sx_sync(uint8_t line)
{
	// Exit if battery voltage is too low for radio operation
	if (sys.flag.low_battery) return;

	// Exit if BlueRobin stack is active
	//pfs
	start_d7_sync();
}
#endif

void start_d7_sync(void)
{
  	// Clear LINE1
	//clear_line(LINE1);
	//fptr_lcd_function_line1(LINE1, DISPLAY_LINE_CLEAR);

	#ifdef FEATURE_PROVIDE_ACCEL
	// Stop acceleration sensor
	as_stop();
	#endif

	// Get updated altitude
#ifdef CONFIG_ALTITUTDE
	start_altitude_measurement();
	stop_altitude_measurement();
#endif

	// Get updated temperature
	temperature_measurement(FILTER_OFF);

	// Turn on beeper icon to show activity
	display_symbol(LCD_ICON_BEEPER1, SEG_ON_BLINK_ON);
	display_symbol(LCD_ICON_BEEPER2, SEG_ON_BLINK_ON);
	display_symbol(LCD_ICON_BEEPER3, SEG_ON_BLINK_ON);

	// Debounce button event
	//Timer0_A4_Delay(CONV_MS_TO_TICKS(BUTTONS_DEBOUNCE_TIME_OUT));

	// Prepare radio for RF communication
	//open_radio();

	// Set SimpliciTI mode
	//sRFsmpl.mode = SIMPLICITI_SYNC;

	// Set SimpliciTI timeout to save battery power
	//sRFsmpl.timeout = SIMPLICITI_TIMEOUT;

	// Start SimpliciTI stack. Try to link to access point.
	// Exit with timeout or by a button DOWN press.
//	if (simpliciti_link())
//	{
//		// Enter sync routine. This will send ready-to-receive packets at regular intervals to the access point.
//		// The access point replies with a command (NOP if no other command is set)
//		simpliciti_main_sync();
//	}
//
//	// Set SimpliciTI state to OFF
//	sRFsmpl.mode = SIMPLICITI_OFF;
//
//	// Powerdown radio
//	close_radio();
//
//	// Clear last button events
//	Timer0_A4_Delay(CONV_MS_TO_TICKS(BUTTONS_DEBOUNCE_TIME_OUT));
//	BUTTONS_IFG = 0x00;
//	button.all_flags = 0;
//
//	// Clear icons
//	display_symbol(LCD_ICON_BEEPER1, SEG_OFF_BLINK_OFF);
//	display_symbol(LCD_ICON_BEEPER2, SEG_OFF_BLINK_OFF);
//	display_symbol(LCD_ICON_BEEPER3, SEG_OFF_BLINK_OFF);
//
//	// Force full display update
//	display.flag.full_update = 1;
}


