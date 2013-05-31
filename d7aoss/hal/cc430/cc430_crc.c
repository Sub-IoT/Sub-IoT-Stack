/*
 * crc.c
 *
 *  Created on: 19-dec.-2012
 *      Author: Maarten Weyn
 */

#include "../system.h"
#include "cc430_addresses.h"

uint16_t crc_calculate(uint8_t* data, uint8_t length)
{
	CRCINIRES = 0xFFFF;
	u8 i = 0;
	for(; i<length; i++)
	{
		CRCDIRB_L = data[i];
	}
	u16 crc = CRCINIRES;
	u16 crcMSB = (crc << 8) | (crc >> 8);
	return crcMSB;
}
