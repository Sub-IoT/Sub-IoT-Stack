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
#include "segmentlcd.h"
#include "segmentlcdconfig.h"
#include <debug.h>

void __lcd_init()
{
	/* Initialize segment LCD. */
	SegmentLCD_Init(false);
	/* Turn all LCD segments off. */
	SegmentLCD_AllOff();
}

void lcd_all_off()
{
	 SegmentLCD_AllOff();
}

void lcd_all_on()
{
	SegmentLCD_AllOn();
}

void lcd_write_string(const char* text)
{
	SegmentLCD_Write(text);
}

void lcd_write_number(int value)
{
	SegmentLCD_Symbol(LCD_SYMBOL_DP10, 0);
	SegmentLCD_Symbol(LCD_SYMBOL_DEGC, 0);
	SegmentLCD_Symbol(LCD_SYMBOL_DEGF, 0);
	SegmentLCD_Number(value);
}

void lcd_write_temperature(int temperature, bool celcius)
{
	SegmentLCD_Number(temperature);
	SegmentLCD_Symbol(LCD_SYMBOL_DP10, 1);

	if (celcius)
		SegmentLCD_Symbol(LCD_SYMBOL_DEGC, 1);
	else
		SegmentLCD_Symbol(LCD_SYMBOL_DEGF, 1);
}

void lcd_show_battery_indication(int batteryLevel)
{
	SegmentLCD_Battery(batteryLevel);
}

void lcd_show_antenna(int show)
{
	SegmentLCD_Symbol(LCD_SYMBOL_ANT, show);
}

void lcd_show_ring(int segments)
{
	int i = 0;
	for (;i<segments;i++)
	{
		SegmentLCD_ARing(i, true);
	}

	for(i=segments;i<8;i++)
	{
		SegmentLCD_ARing(i, false);
	}
}

