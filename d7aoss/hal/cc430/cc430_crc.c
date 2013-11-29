/*!
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *
 */

#include "../system.h"
#include "cc430_addresses.h"

uint16_t crc_calculate(uint8_t* data, uint8_t length)
{
	CRCINIRES = 0xFFFF;
	uint8_t i = 0;
	for(; i<length; i++)
	{
		CRCDIRB_L = data[i];
	}
	uint16_t crc = CRCINIRES;
	uint16_t crcMSB = (crc << 8) | (crc >> 8);
	return crcMSB;
}
