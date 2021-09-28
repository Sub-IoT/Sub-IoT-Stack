/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
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
#include "ng.h"
#include <stdarg.h>
#include <stdio.h>

#define BUFFER_SIZE 12
static char NGDEF(buffer)[BUFFER_SIZE];

void __lcd_init()
{
	/* Initialize segment LCD. */
	SegmentLCD_Init(false);
	/* Turn all LCD segments off. */
	SegmentLCD_AllOff();
}

void lcd_enable(bool enable)
{
	if (enable)
		__lcd_init();
	else
		SegmentLCD_Disable();
}

void lcd_clear()
{
	SegmentLCD_AllOff();
}

void lcd_write_string(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	uint8_t len = vsnprintf(NG(buffer), BUFFER_SIZE, format, args);
	va_end(args);
	SegmentLCD_Write(NG(buffer));
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
	if (segments > 7) segments = 7;
	for (;i<segments;i++)
	{
		SegmentLCD_ARing(i, true);
	}

	for(i=segments;i<8;i++)
	{
		SegmentLCD_ARing(i, false);
	}
}

void lcd_show_ring_segments(uint8_t segments)
{
	int i = 0;
	for (;i<8;i++)
	{
		if (segments & 1 << i)
			SegmentLCD_ARing(i, true);
		else
			SegmentLCD_ARing(i, false);
	}
}

