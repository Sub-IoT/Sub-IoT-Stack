/*
 * project.h
 *
 *  Created on: 2-dec.-2012
 *      Author: Maarten Weyn
 */

#ifndef PROJECT_H_
#define PROJECT_H_

#define FEATURE_ALTITUDE

#define CONFIG_USEPPT
#define CONFIG_BATTERY
#define CONFIG_ALTITUDE
//#define CONFIG_USE_SYNC_TOSET_TIME

#define SEND_INTERVAL_MS	10000
#define SEND_CHANNEL 0x1E
#define TX_EIRP 10

#include <hal/system.h>
#include <msp430.h>

// Use/not use filter when measuring physical values
#define FILTER_OFF						(0u)
#define FILTER_ON						(1u)

extern void sx_ppt(uint8_t line);

extern void display_ppt(uint8_t line, uint8_t update);

// *************************************************************************************************
// Macro section

// Conversion from usec to ACLK timer ticks
#define CONV_US_TO_TICKS(usec)         			(((usec) * 32768) / 1000000)

// Conversion from msec to ACLK timer ticks
#define CONV_MS_TO_TICKS(msec)         			(((msec) * 32768) / 1000)


// *************************************************************************************************
// Typedef section

typedef uint8_t line_t;
typedef uint8_t update_t;

typedef enum
{
  MENU_ITEM_NOT_VISIBLE = 0,   	// Menu item is not visible
  MENU_ITEM_VISIBLE      		// Menu item is visible
} menu_t;


// Set of system flags
typedef union
{
  struct
  {
    uint16_t idle_timeout      		: 1;    // Timeout after inactivity
    uint16_t idle_timeout_enabled    : 1;    // When in set mode, timeout after a given period
    uint16_t lock_buttons			: 1;    // Lock buttons
    uint16_t mask_buzzer		 		: 1;	// Do not output buzz for next button event
    uint16_t up_down_repeat_enabled  : 1;    // While in set_value(), create virtual UP/DOWN button events
    uint16_t low_battery      		: 1;    // 1 = Battery is low
    uint16_t use_metric_units		: 1;    // 1 = Use metric units, 0 = use English units
    uint16_t am_pm_time          : 1;    // 1 = Display times as AM/PM else 24Hr
    uint16_t delay_over     			: 1;    // 1 = Timer delay over
    uint16_t no_beep                 : 1;    // Don't beep on key press
  } flag;
  uint16_t all_flags;            // Shortcut to all display flags (for reset)
} s_system_flags;
extern volatile s_system_flags sys;


// Set of request flags
typedef union
{
  struct
  {
    uint16_t temperature_measurement 	: 1;    // 1 = Measure temperature
    uint16_t voltage_measurement    		: 1;    // 1 = Measure voltage
    uint16_t altitude_measurement    	: 1;    // 1 = Measure air pressure
#ifdef CONFIG_ALTI_ACCUMULATOR
    uint16_t altitude_accumulator            : 1;	// 1 = Measure altitude & accumulate it
#endif
    uint16_t	acceleration_measurement	: 1; 	// 1 = Measure acceleration
    uint16_t alarm_buzzer			: 1;	// 1 = Output buzzer for alarm
#ifdef CONFIG_EGGTIMER
    uint16_t eggtimer_buzzer : 1; // 1 = Output buzzer for eggtimer
#endif
#ifdef CONFIG_STRENGTH
    uint16_t strength_buzzer 		: 1;    // 1 = Output buzzer from strength_data
#endif
  } flag;
  uint16_t all_flags;            // Shortcut to all display flags (for reset)
} s_request_flags;
extern volatile s_request_flags request;


// Set of message flags
typedef union
{
  struct
  {
    uint16_t prepare							: 1;	// 1 = Wait for clock tick, then set display.flag.show_message flag
    uint16_t show							: 1;	// 1 = Display message now
    uint16_t erase							: 1;	// 1 = Erase message
    uint16_t type_locked						: 1;	// 1 = Show "buttons are locked" in Line2
    uint16_t type_unlocked					: 1;	// 1 = Show "buttons are unlocked" in Line2
    uint16_t type_lobatt						: 1;	// 1 = Show "lobatt" text in Line2
    uint16_t type_alarm_off_chime_off		: 1;	// 1 = Show " off" text in Line1
    uint16_t type_alarm_off_chime_on			: 1;	// 1 = Show " offh" text in Line1
    uint16_t type_alarm_on_chime_off			: 1;	// 1 = Show "  on" text in Line1
    uint16_t type_alarm_on_chime_on			: 1;	// 1 = Show " onh" text in Line1
    uint16_t type_no_beep_on					: 1;	// 1 = Show " beep" text in Line2
    uint16_t type_no_beep_off				: 1;	// 1 = Show "nobeep" text in Line2
    uint16_t block_line1						: 1;	// 1 = block Line1 from updating until message erase
    uint16_t block_line2						: 1;	// 1 = block Line2 from updating until message erase
 } flag;
  uint16_t all_flags;            // Shortcut to all message flags (for reset)
} s_message_flags;
extern volatile s_message_flags message;

#endif /* PROJECT_H_ */
