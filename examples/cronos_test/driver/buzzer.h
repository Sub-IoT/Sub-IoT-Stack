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

#ifndef BUZZER_H_
#define BUZZER_H_

#include "..\project.h"

// *************************************************************************************************
// Include section


// *************************************************************************************************
// Prototypes section
extern void reset_buzzer(void);
extern void start_buzzer(uint8_t cycles, uint16_t on_time, uint16_t off_time);
extern void start_buzzer_steps(uint8_t cycles, uint16_t on_time, uint16_t off_time, uint8_t steps);
extern void stop_buzzer(void);
extern void toggle_buzzer(void* arg);
extern uint8_t is_buzzer(void);
extern void countdown_buzzer(void);


// *************************************************************************************************
// Defines section

// Buzzer states
#define BUZZER_OFF							(0u)
#define BUZZER_ON_OUTPUT_DISABLED			(1u)
#define BUZZER_ON_OUTPUT_ENABLED			(2u)

// Buzzer modes
#define BUZZER_MODE_SINGLE					(0u)
#define BUZZER_MODE_SINGLE_CONTINUOUS		(1u)
#define BUZZER_MODE_DOUBLE_CONTINUOUS		(2u)

// Buzzer output signal frequency = 32,768kHz/(BUZZER_TIMER_STEPS+1)/2 = 2.7kHz
#define BUZZER_TIMER_STEPS					(5u)	

// Buzzer on time
#define BUZZER_ON_TICKS						(CONV_MS_TO_TICKS(20))

// Buzzer off time
#define BUZZER_OFF_TICKS					(CONV_MS_TO_TICKS(200))


// *************************************************************************************************
// Global Variable section
struct buzzer
{
	// Keep output for "time" seconds
	uint8_t time;
	
	// On/off duty 
	uint16_t on_time;
	uint16_t off_time;
	
	// Current buzzer output state
	uint8_t state;

    // Current steps (~freq)
    uint8_t steps;

};
extern struct buzzer sBuzzer;


// *************************************************************************************************
// Extern section




#endif /*BUZZER_H_*/
