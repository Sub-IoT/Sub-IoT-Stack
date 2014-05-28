/*
 *  Created on: Dec 2, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 */

#include "project.h"

#include <hal/system.h>
#include <trans/trans.h>
#include <hal/rtc.h>

#include "driver/display.h"
#include "driver/ports.h"
#include "driver/buzzer.h"
//#include "driver/timer.h"
#include "driver/vti_ps.h"

#include "logic/clock.h"
#include "logic/date.h"
#include "logic/menu.h"
#include "logic/user.h"
#include "logic/altitude.h"
#include "logic/battery.h"
#include "logic/rf.h"

#define INTERRUPT_RTC 		(1 << 3)

//static uint8_t interrupt_flags = 0;



// *************************************************************************************************
// Global Variable section

// Variable holding system internal flags
volatile s_system_flags sys;

// Variable holding flags set by logic modules
volatile s_request_flags request;

// Variable holding message flags
volatile s_message_flags message;

int8_t txPower;
uint8_t beacon_channel;
uint16_t beacon_interval;

// Function pointers for LINE1 and LINE2 display function
void (*fptr_lcd_function_line1)(uint8_t line, uint8_t update);
void (*fptr_lcd_function_line2)(uint8_t line, uint8_t update);

extern void tx_callback(Dll_Tx_Result result);

// Prototypes section
void init_application(void);
void init_global_variables(void);
void wakeup_event(void);
void process_requests(void);
void display_update(void);
void idle_loop(void);
//void configure_ports(void);
//void read_calibration_values(void);

void menu_skip_next(line_t line);

void init_application(void)
{
	volatile unsigned char *ptr;


	// Initialize the OSS-7 Stack
	//system_init();
	system_get_unique_id(device_id);

	// ---------------------------------------------------------------------
	// Enable watchdog

	// Watchdog triggers after 16 seconds when not cleared
	#ifdef USE_WATCHDOG
		WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK;
	#else
		WDTCTL = WDTPW + WDTHOLD;
	#endif


	// Set global high power request enable
	PMMCTL0_H  = 0xA5;
	PMMCTL0_L |= PMMHPMRE;
	PMMCTL0_H  = 0x00;

	// ---------------------------------------------------------------------
	// Enable 32kHz ACLK
	P5SEL |= 0x03;                            // Select XIN, XOUT on P5.0 and P5.1
	UCSCTL6 &= ~XT1OFF;        				  // XT1 On, Highest drive strength
	UCSCTL6 |= XCAP_3;                        // Internal load cap

	UCSCTL3 = SELA__XT1CLK;                   // Select XT1 as FLL reference
	UCSCTL4 = SELA__XT1CLK | SELS__DCOCLKDIV | SELM__DCOCLKDIV;

	// ---------------------------------------------------------------------
	// Configure CPU clock for 12MHz
	_BIS_SR(SCG0);                  // Disable the FLL control loop
	UCSCTL0 = 0x0000;          // Set lowest possible DCOx, MODx
	UCSCTL1 = DCORSEL_5;       // Select suitable range
	UCSCTL2 = FLLD_1 + 0x16E;  // Set DCO Multiplier
	_BIC_SR(SCG0);                  // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
    __delay_cycles(250000);

	// Loop until XT1 & DCO stabilizes, use do-while to insure that
	// body is executed at least once
	do
	{
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
		SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	} while ((SFRIFG1 & OFIFG));


	// ---------------------------------------------------------------------
	// Configure port mapping

	// Disable all interrupts
	__disable_interrupt();
	// Get write-access to port mapping registers:
	PMAPPWD = 0x02D52;
	// Allow reconfiguration during runtime:
	PMAPCTL = PMAPRECFG;

	// P2.7 = TA0CCR1A or TA1CCR0A output (buzzer output)
	ptr  = &P2MAP0;
	*(ptr+7) = PM_TA1CCR0A;
	P2OUT &= ~BIT7;
	P2DIR |= BIT7;

	// P1.5 = SPI MISO input
	ptr  = &P1MAP0;
	*(ptr+5) = PM_UCA0SOMI;
	// P1.6 = SPI MOSI output
	*(ptr+6) = PM_UCA0SIMO;
	// P1.7 = SPI CLK output
	*(ptr+7) = PM_UCA0CLK;

	// Disable write-access to port mapping registers:
	PMAPPWD = 0;
	// Re-enable all interrupts
	__enable_interrupt();

	// ---------------------------------------------------------------------
	// Configure ports

	// ---------------------------------------------------------------------
	// Reset radio core
	//radio_reset();
	//radio_powerdown();

	//#ifdef FEATURE_PROVIDE_ACCEL
	// ---------------------------------------------------------------------
	// Init acceleration sensor
//	as_init();
//	#else
//	as_disconnect();
//	#endif

	// ---------------------------------------------------------------------
	// Init LCD
	lcd_init();

	// ---------------------------------------------------------------------
	// Init buttons
	init_buttons();

	// ---------------------------------------------------------------------
	// Configure Timer0 for use by the clock and delay functions
	//Timer0_Init();
	//Setup Current Time for Calendar
	rtc_init_counter_mode();
	rtc_enable_interrupt();
	rtc_start();

	// ---------------------------------------------------------------------
	// Init pressure sensor
	ps_init();
}

void init_global_variables(void)
{
	// --------------------------------------------
	// Apply default settings

	menu_L1_position=0;
	menu_L2_position=0;
	// set menu pointers to default menu items
	ptrMenu_L1 = menu_L1[menu_L1_position];
	ptrMenu_L2 = menu_L2[menu_L2_position];

	// Assign LINE1 and LINE2 display functions
	fptr_lcd_function_line1 = ptrMenu_L1->display_function;
	fptr_lcd_function_line2 = ptrMenu_L2->display_function;

	// Init system flags
	button.all_flags 	= 0;
	sys.all_flags 		= 0;
	request.all_flags 	= 0;
	display.all_flags 	= 0;
	message.all_flags	= 0;

	txPower = TX_EIRP;
	beacon_channel = SEND_CHANNEL;
	beacon_interval = SEND_INTERVAL_MS;

	// Force full display update when starting up
	display.flag.full_update = 1;

//#ifndef ISM_US
//	// Use metric units when displaying values
//    sys.flag.use_metric_units = 1;
//#else
//#ifdef CONFIG_METRIC_ONLY
	sys.flag.use_metric_units = 1;
//#endif
//#endif

	// Read calibration values from info memory
	//read_calibration_values();
//#ifdef CONFIG_ALTI_ACCUMULATOR
//	// By default, don't have the altitude accumulator running
//	alt_accum_enable = 0;
//#endif


//	#ifdef CONFIG_INFOMEM
//	if(infomem_ready()==-2)
//	{
//		infomem_init(INFOMEM_C, INFOMEM_C+2*INFOMEM_SEGMENT_SIZE);
//	}
//	#endif

	// Set system time to default value
	reset_clock();

	// Set date to default value
	reset_date();

//	#ifdef CONFIG_SIDEREAL
//	reset_sidereal_clock();
//	#endif
//
//	#ifdef CONFIG_ALARM
//	// Set alarm time to default value
//	reset_alarm();
//	#endif

	// Set buzzer to default value
	reset_buzzer();

//#ifdef CONFIG_STOP_WATCH
//	// Reset stopwatch
//	reset_stopwatch();
//#endif

	// Reset altitude measurement
#ifdef CONFIG_ALTITUDE
	reset_altitude_measurement();
#endif
//
//	#ifdef FEATURE_PROVIDE_ACCEL
//	// Reset acceleration measurement
//	reset_acceleration();
//	#endif

	// Reset BlueRobin stack
	//pfs
//	#ifndef ELIMINATE_BLUEROBIN
//	reset_bluerobin();
//	#endif

//#ifdef CONFIG_EGGTIMER
//	init_eggtimer(); // Initialize eggtimer
//#endif
//
//#ifdef CONFIG_PROUT
//        reset_prout();
//#endif
//
//#ifdef CONFIG_PHASE_CLOCK
//	// default program
//	sPhase.program = 0;
//#endif

	// Reset SimpliciTI stack
//	reset_rf();

	// Reset temperature measurement
//	reset_temp_measurement();

	#ifdef CONFIG_BATTERY
	// Reset battery measurement
	reset_batt_measurement();
	battery_measurement();
	#endif
}

void idle_loop(void)
{
	#ifdef CONFIG_CW_TIME
	// what I'd like to do here is set a morsepos variable
	// if non-zero it is how many digits we have left to go
	// on sending the time.
	// we also would have a morse var that would only get set
	// the first send and reset when not in view so we'd only
	// send the time once

#define CW_DIT_LEN CONV_MS_TO_TICKS(50)    // basic element size (100mS)

	static int morse=0;       // should send morse == 1
	static int morsepos=0; // position in morse time (10 hour =1, hour=2, etc.)
	static int morsehr; // cached hour for morse code
	static int morsemin;  // cached minute for morse code
	static int morsedig=-1; // current digit
	static int morseel; // current element in digit (always 5 elements max)
	static unsigned int morseinitdelay; // start up delay

  // We only send the time in morse code if the seconds display is active, and then only
  // once per activation

	if (sTime.line1ViewStyle == DISPLAY_ALTERNATIVE_VIEW)
	  {
	    if (!morse)   // this means its the first time (we reset this to zero in the else)
		{

			morse=1;  // mark that we are sending
			morsepos=1;  // initialize pointer

			// Jim pointed out it is remotely possible that a button
			// int could wake up this routine and then the hour could
			// flip over after reading so I added this "do" loop

			do {
				morsehr=sTime.hour;  // and cache
				morsemin=sTime.minute;
			} while (morsehr!=sTime.hour);

			morsedig=-1;  // not currently sending digit
			morseinitdelay=45000;  // delay for a bit before starting so the key beep can quiet down

		}

		if (morseinitdelay)   // this handles the initial delay
		{
			morseinitdelay--;
			return;  // do not sleep yet or no event will get scheduled and we'll hang for a very long time
		}

	    if (!is_buzzer() && morsedig==-1)  // if not sending anything
		{

			morseel=0;                     // start a new character
			switch (morsepos++)            // get the right digit
			{
				case 1:
					morsedig=morsehr/10;
					break;
				case 2:
					morsedig=morsehr%10;
					break;
				case 3:
					morsedig=morsemin/10;
					break;
				case 4:
					morsedig=morsemin%10;
					break;
				default:
					morsepos=5;  // done for now
			}
			if (morsedig==0)
				morsedig=10;  // treat zero as 10 for code algorithm
		}

	    // now we have a digit and we need to send element
		if (!is_buzzer()&&morsedig!=-1)
		{

			int digit=morsedig;
			// assume we are sending dit for 1-5 or dah for 6-10 (zero is 10)
			int ditdah=(morsedig>5)?1:0;
			int dit=CW_DIT_LEN;
			if (digit>=6)
				digit-=5;   // fold digits 6-10 to 1-5
			if (digit>=++morseel)
				ditdah=ditdah?0:1;  // flip dits and dahs at the right point

			// send the code
			start_buzzer(1,ditdah?dit:(3*dit),(morseel>=5)?10*dit:dit);

			// all digits have 5 elements
			if (morseel==5)
				morsedig=-1;

		}

	} else {
	  morse=0;  // no morse code right now
	}

#endif
	// To low power mode
	//to_lpm();
	system_lowpower_mode(3,1);

#ifdef USE_WATCHDOG
	// Service watchdog (reset counter)
	WDTCTL = (WDTCTL &0xff) | WDTPW | WDTCNTCL;
#endif
}

void menu_skip_next(line_t line)
{
	if(line==LINE1)
	{
		// Clean up display before activating next menu item
		fptr_lcd_function_line1(LINE1, DISPLAY_LINE_CLEAR);

		if(++menu_L1_position>=menu_L1_size)
		{
			menu_L1_position=0;
		}

		// Go to next menu entry
		ptrMenu_L1 = menu_L1[menu_L1_position];

		// Assign new display function
		fptr_lcd_function_line1 = ptrMenu_L1->display_function;
	}
	else if(line==LINE2)
	{
	// Clean up display before activating next menu item
		fptr_lcd_function_line2(LINE2, DISPLAY_LINE_CLEAR);

		if(++menu_L2_position>=menu_L2_size)
			menu_L2_position=0;

		// Go to next menu entry
		ptrMenu_L2 = menu_L2[menu_L2_position];

		// Assign new display function
		fptr_lcd_function_line2 = ptrMenu_L2->display_function;
	}

}

void wakeup_event(void)
{
	// Enable idle timeout
	sys.flag.idle_timeout_enabled = 1;

	// If buttons are locked, only display "buttons are locked" message
	if (button.all_flags && sys.flag.lock_buttons)
	{
		// Show "buttons are locked" message synchronously with next second tick
		if (!((BUTTON_NUM_IS_PRESSED && BUTTON_DOWN_IS_PRESSED) || BUTTON_BACKLIGHT_IS_PRESSED))
		{
			message.flag.prepare     = 1;
			message.flag.type_locked = 1;
		}

		// Clear buttons
		button.all_flags = 0;
	}
	// Process long button press event (while button is held)
	else if (button.flag.star_long)
	{
		// Clear button event
		button.flag.star_long = 0;

		// Call sub menu function
		ptrMenu_L1->mx_function(LINE1);

		// Set display update flag
		display.flag.full_update = 1;
	}
	else if (button.flag.num_long)
	{
		// Clear button event
		button.flag.num_long = 0;

		// Call sub menu function
		ptrMenu_L2->mx_function(LINE2);

		// Set display update flag
		display.flag.full_update = 1;
	}
	// Process single button press event (after button was released)
	else if (button.all_flags)
	{
		// M1 button event ---------------------------------------------------------------------
		// (Short) Advance to next menu item
		if(button.flag.star)
		{
			//skip to next menu item
			ptrMenu_L1->nx_function(LINE1);

			// Set Line1 display update flag
			display.flag.line1_full_update = 1;

			// Clear button flag
			button.flag.star = 0;
		}
		// NUM button event ---------------------------------------------------------------------
		// (Short) Advance to next menu item
		else if(button.flag.num)
		{
			//skip to next menu item
			ptrMenu_L2->nx_function(LINE2);

			// Set Line2 display update flag
			display.flag.line2_full_update = 1;

			// Clear button flag
			button.flag.num = 0;
		}
		// UP button event ---------------------------------------------------------------------
		// Activate user function for Line1 menu item
		else if(button.flag.up)
		{
			// Call direct function
			ptrMenu_L1->sx_function(LINE1);

			// Set Line1 display update flag
			display.flag.line1_full_update = 1;

			// Clear button flag
			button.flag.up = 0;
		}
		// DOWN button event ---------------------------------------------------------------------
		// Activate user function for Line2 menu item
		else if(button.flag.down)
		{
			// Call direct function
			ptrMenu_L2->sx_function(LINE2);

			// Set Line1 display update flag
			display.flag.line2_full_update = 1;

			// Clear button flag
			button.flag.down = 0;
		}
	}

	// Process internal events
	if (sys.all_flags)
	{
		// Idle timeout ---------------------------------------------------------------------
		if (sys.flag.idle_timeout)
		{
			// Clear timeout flag
			sys.flag.idle_timeout = 0;

			// Clear display
			clear_display();

			// Set display update flags
			display.flag.full_update = 1;
		}
	}

	// Disable idle timeout
	sys.flag.idle_timeout_enabled = 0;
}

void process_requests(void)
{
	// Do temperature measurement
//	if (request.flag.temperature_measurement) temperature_measurement(FILTER_ON);

	// Do pressure measurement
#ifdef CONFIG_ALTITUDE
  	if (request.flag.altitude_measurement) do_altitude_measurement(FILTER_ON);
#endif
//#ifdef CONFIG_ALTI_ACCUMULATOR
//	if (request.flag.altitude_accumulator) altitude_accumulator_periodic();
//#endif

//	#ifdef FEATURE_PROVIDE_ACCEL
//	// Do acceleration measurement
//	if (request.flag.acceleration_measurement) do_acceleration_measurement();
//	#endif

	#ifdef CONFIG_BATTERY
	// Do voltage measurement
	if (request.flag.voltage_measurement) battery_measurement();
	#endif

//	#ifdef CONFIG_ALARM
//	// Generate alarm (two signals every second)
//	if (request.flag.alarm_buzzer) start_buzzer(2, BUZZER_ON_TICKS, BUZZER_OFF_TICKS);
//	#endif
//
//#ifdef CONFIG_EGGTIMER
//	// Generate alarm (two signals every second)
//	if (request.flag.eggtimer_buzzer) start_buzzer(2, BUZZER_ON_TICKS, BUZZER_OFF_TICKS);
//#endif
//
//
//#ifdef CONFIG_STRENGTH
//	if (request.flag.strength_buzzer && strength_data.num_beeps != 0)
//	{
//		start_buzzer(strength_data.num_beeps,
//			     STRENGTH_BUZZER_ON_TICKS,
//			     STRENGTH_BUZZER_OFF_TICKS);
//		strength_data.num_beeps = 0;
//	}
//#endif

	// Reset request flag
	request.all_flags = 0;
}

void display_update(void)
{
	uint8_t line;
	uint8_t string[8];

	// ---------------------------------------------------------------------
	// Call Line1 display function
	if (display.flag.full_update ||	display.flag.line1_full_update)
	{
		clear_line(LINE1);
		fptr_lcd_function_line1(LINE1, DISPLAY_LINE_UPDATE_FULL);
	}
	else if (ptrMenu_L1->display_update() &&!message.flag.block_line1)
	{
		// Update line1 only when new data is available
		fptr_lcd_function_line1(LINE1, DISPLAY_LINE_UPDATE_PARTIAL);
	}

	// ---------------------------------------------------------------------
	// Call Line2 display function
	if (display.flag.full_update || display.flag.line2_full_update)
	{
		clear_line(LINE2);
		fptr_lcd_function_line2(LINE2, DISPLAY_LINE_UPDATE_FULL);
	}
	else if (ptrMenu_L2->display_update() && !message.flag.block_line2)
	{
		// Update line2 only when new data is available
		fptr_lcd_function_line2(LINE2, DISPLAY_LINE_UPDATE_PARTIAL);
	}

	// ---------------------------------------------------------------------
	// If message text should be displayed
	if (message.flag.show)
	{
		line = LINE2;

		// Select message to display
		if (message.flag.type_locked)			memcpy(string, "  LOCT", 6);
		else if (message.flag.type_unlocked)	memcpy(string, "  OPEN", 6);
		else if (message.flag.type_lobatt)		memcpy(string, "LOBATT", 6);
		else if (message.flag.type_no_beep_on)  memcpy(string, " SILNT", 6);
		else if (message.flag.type_no_beep_off) memcpy(string, "  BEEP", 6);
		#ifdef CONFIG_ALARM
		else if (message.flag.type_alarm_off_chime_off)
		{
			memcpy(string, " OFF", 4);
			line = LINE1;
		}
		else if (message.flag.type_alarm_off_chime_on)
		{
			memcpy(string, "OFFH", 4);
			line = LINE1;
		}
		else if (message.flag.type_alarm_on_chime_off)
		{
			memcpy(string, "  ON", 4);
			line = LINE1;
		}
		else if (message.flag.type_alarm_on_chime_on)
		{
			memcpy(string, " ONH", 4);
			line = LINE1;
		}
		#endif


		// Clear previous content
		clear_line(line);
		if(line == LINE2) 	fptr_lcd_function_line2(line, DISPLAY_LINE_CLEAR);
		else				fptr_lcd_function_line1(line, DISPLAY_LINE_CLEAR);

		if (line == LINE2) 	display_chars(LCD_SEG_L2_5_0, string, SEG_ON);
		else 				display_chars(LCD_SEG_L1_3_0, string, SEG_ON);

		// Next second tick erases message and repaints original screen content (full_update)
		message.all_flags = 0;
		if(line == LINE2) 	message.flag.block_line2 = 1;
		else				message.flag.block_line1 = 1;
		message.flag.erase = 1;
	}

	// ---------------------------------------------------------------------
	// Restore blinking icons (blinking memory is cleared when calling set_value)
	if (display.flag.full_update)
	{
	//pfs
//	#ifndef ELIMINATE_BLUEROBIN
//		if (is_bluerobin() == BLUEROBIN_CONNECTED)
//		{
//			// Turn on beeper icon to show activity
//			display_symbol(LCD_ICON_BEEPER1, SEG_ON_BLINK_OFF);
//			display_symbol(LCD_ICON_BEEPER2, SEG_ON_BLINK_OFF);
//			display_symbol(LCD_ICON_BEEPER3, SEG_ON_BLINK_OFF);
//		}
//	#endif
	}

	// Clear display flag
	display.all_flags = 0;
}

void main(void) {
	// Init MCU
	init_application();

	// Assign initial value to global variables
	init_global_variables();

	display_all_off();

	trans_init();
	trans_set_tx_callback(&tx_callback);
	trans_set_initial_t_ca(200);

	blink_init();
	blink_start();

	// Main control loop: wait in low power mode until some event needs to be processed
	while(1)
	{
		// When idle go to LPM3
		idle_loop();

		// Process wake-up events
		if (button.all_flags || sys.all_flags) wakeup_event();

		// Process actions requested by logic modules
		if (request.all_flags) process_requests();

		// Before going to LPM3, update display
		if (display.all_flags) display_update();
	}
}


#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR (void)
{
	static uint8_t button_lock_counter = 0;
	static uint8_t button_beep_counter = 0;

    switch (RTCIV){
        case 0: break;  //No interrupts
        case 2: break;  //RTCRDYIFG
        case 4:         //RTCEVIFG

        	clock_tick();

        	// -------------------------------------------------------------------
			// Turn the Backlight off after timeout
			if (sButton.backlight_status == 1)
			{
				if (sButton.backlight_timeout > BACKLIGHT_TIME_ON)
				{
					//turn off Backlight
					P2OUT &= ~BUTTON_BACKLIGHT_PIN;
					P2DIR &= ~BUTTON_BACKLIGHT_PIN;
					sButton.backlight_timeout = 0;
					sButton.backlight_status = 0;
				}
				else
				{
					sButton.backlight_timeout++;
				}
			}
			// -------------------------------------------------------------------
			// Detect continuous button high states

			if (BUTTON_STAR_IS_PRESSED && BUTTON_UP_IS_PRESSED)
			{
				if (button_beep_counter++ > LEFT_BUTTON_LONG_TIME)
				{
					// Toggle no_beep buttons flag
					sys.flag.no_beep = ~sys.flag.no_beep;

					// Show "beep / nobeep" message synchronously with next second tick
					message.flag.prepare = 1;
					if (sys.flag.no_beep)	message.flag.type_no_beep_on   = 1;
					else					message.flag.type_no_beep_off  = 1;

					// Reset button beep counter
					button_beep_counter = 0;
				}
			} else if (BUTTON_NUM_IS_PRESSED && BUTTON_DOWN_IS_PRESSED) // Trying to lock/unlock buttons?
			{
				if (button_lock_counter++ > LEFT_BUTTON_LONG_TIME)
				{
					// Toggle lock / unlock buttons flag
					sys.flag.lock_buttons = ~sys.flag.lock_buttons;

					// Show "buttons are locked/unlocked" message synchronously with next second tick
					message.flag.prepare = 1;
					if (sys.flag.lock_buttons)	message.flag.type_locked   = 1;
					else						message.flag.type_unlocked = 1;

					// Reset button lock counter
					button_lock_counter = 0;
				}
			}
			else // Trying to create a long button press?
			{
				// Reset button lock counter
				button_lock_counter = 0;

				if (BUTTON_STAR_IS_PRESSED)
				{
					sButton.star_timeout++;

					// Check if button was held low for some seconds
					if (sButton.star_timeout > LEFT_BUTTON_LONG_TIME)
					{
						button.flag.star_long = 1;
						sButton.star_timeout = 0;
					}
				}
				else
				{
					sButton.star_timeout = 0;
				}

				if (BUTTON_NUM_IS_PRESSED)
				{
					sButton.num_timeout++;

					// Check if button was held low for some seconds
					if (sButton.num_timeout > LEFT_BUTTON_LONG_TIME)
					{
						button.flag.num_long = 1;
						sButton.num_timeout = 0;
					}
				}
				else
				{
					sButton.num_timeout = 0;
				}
			}




        	LPM3_EXIT;
            break;
        case 6: break;  //RTCAIFG
        case 8: break;  //RT0PSIFG
        case 10: break; //RT1PSIFG
        default: break;
    }
}
