/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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
