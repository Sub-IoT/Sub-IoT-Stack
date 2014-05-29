/*
 * crc.c
 *
 *  Created on: 19-dec.-2012
 *      Author: Maarten Weyn
 */

#include "crc.h"

static uint16_t crc;

// TODO refactor: software implementation of CRC is not plaform specific and can be reused

void crc_ccitt_update(uint8_t x)
{
     uint16_t crc_new = (uint8_t)(crc >> 8) | (crc << 8);
     crc_new ^= x;
     crc_new ^= (uint8_t)(crc_new & 0xff) >> 4;
     crc_new ^= crc_new << 12;
     crc_new ^= (crc_new & 0xff) << 5;
     crc = crc_new;
}

uint16_t crc_calculate(uint8_t* data, uint8_t length)
{
	crc = 0xffff;
	uint8_t i = 0;

	for(; i<length; i++)
	{
		crc_ccitt_update(data[i]);
	}
	return crc;
}