/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file platform_lcd.c
 *
 *  \author maarten.weyn@uantwerpen.be
 *
 */

#include "hwlcd.h"
#include "platform.h"
#include "platform_lcd.h"
#include "display.h"
#include "textdisplay.h"
#include "retargettextdisplay.h"
#include <assert.h>

static TEXTDISPLAY_Handle_t h;

void __lcd_init()
{
	DISPLAY_Init();

	TEXTDISPLAY_Config_t config  = {0, false, true};
	EMSTATUS status = TEXTDISPLAY_New(&config, &h);


}

void lcd_all_off()
{
	 //SegmentLCD_AllOff();
}

void lcd_all_on()
{
	//SegmentLCD_AllOn();
}

void lcd_write_string(char* text)
{
	TEXTDISPLAY_WriteString(h, text);
}

void lcd_write_number(int value)
{
	//SegmentLCD_Number(value);
}


