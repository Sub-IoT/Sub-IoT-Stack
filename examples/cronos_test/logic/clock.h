// *************************************************************************************************
//
//	Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/ 
//	 
//	 
//	  Redistribution and use in source and binary forms, with or without 
//	  modification, are permitted provided that the following conditions 
//	  are met:
//	
//	    Redistributions of source code must retain the above copyright 
//	    notice, this list of conditions and the following disclaimer.
//	 
//	    Redistributions in binary form must reproduce the above copyright
//	    notice, this list of conditions and the following disclaimer in the 
//	    documentation and/or other materials provided with the   
//	    distribution.
//	 
//	    Neither the name of Texas Instruments Incorporated nor the names of
//	    its contributors may be used to endorse or promote products derived
//	    from this software without specific prior written permission.
//	
//	  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//	  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//	  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//	  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//	  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//	  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//	  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//	  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//	  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//	  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************

#ifndef CLOCKTIMER_H_
#define CLOCKTIMER_H_

// *************************************************************************************************
// Defines section

// Definitions for time format
#define TIMEFORMAT_24H					(0u)
#define TIMEFORMAT_12H					(1u)

// *************************************************************************************************
// Prototypes section
extern void reset_clock(void);
extern void sx_time(uint8_t line);
extern void mx_time(uint8_t line);
extern void clock_tick(void);
extern void display_selection_Timeformat1(uint8_t segments, uint32_t  index, uint8_t digits, uint8_t blanks, uint8_t dummy);
extern void display_time(uint8_t line, uint8_t update);

// English units support
extern uint8_t convert_hour_to_12H_format(uint8_t hour);
extern uint8_t is_hour_am(uint8_t hour);


// *************************************************************************************************
// Global Variable section
struct time
{
	uint32_t  	system_time;

	// Flag to minimize display updates
	uint8_t 		drawFlag;

	// Viewing style
	uint8_t		line1ViewStyle;
	uint8_t		line2ViewStyle;
	
	// Time data
	uint8_t		hour;
	uint8_t		minute;
	uint8_t 		second;
	
	// Inactivity detection (exits set_value() function)
	uint32_t  	last_activity;
	#ifdef CONFIG_SIDEREAL
	// offset of local time from UTC (=1: set time is UTC+1 =CET)
	int8_t 		UTCoffset;
	#endif
};
extern volatile struct time sTime;


#endif /*CLOCKTIMER_H_*/
