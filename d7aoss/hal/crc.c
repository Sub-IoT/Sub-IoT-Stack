/*
 * crc.c
 *
 *  Created on: 19-dec.-2012
 *      Author: Maarten Weyn
 */

#include "system.h"

u16 crc_calculate(u8* data, u8 length)
{
	CRCINIRES = 0xFFFF;
	u8 i = 0;
	for(; i<length; i++)
	{
		CRCDIRB_L = data[i];
	}
	u16 crc = CRCINIRES;
	return crc;
}
