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

#include "cc430_addresses.h"
#include "../flash.h"
#include <string.h>


#define SEGPTR(x) (uint8_t*) ((uint16_t)(x) & 0xFE00)

void write_bytes_to_flash(uint8_t *address, uint8_t* data, uint16_t length)
{
	__disable_interrupt();

	uint8_t buffer[512];
	uint8_t* start = SEGPTR(address);
	uint16_t offset = (uint16_t) address - (uint16_t)start;

	memcpy(buffer, start, 512); // Copy FLASH segment to RAM
	if (memcmp(&buffer[offset], data, length) != 0)
	{
		memcpy(&buffer[offset], data, length); // Adapt segment in RAM

		while (FCTL3 & BUSY);
		FCTL3 = FWKEY; // Clear Lock bit
		FCTL1 = FWKEY + ERASE;  // ERASE
		*start = 0; // Erase FLASH segment

		while (FCTL3 & BUSY);
		FCTL1 = FWKEY + WRT; // WRITE

		memcpy(start, &buffer, 512); // Write RAM to FLASH

		while (FCTL3 & BUSY);
		FCTL1 = FWKEY; // Clear Write/Erase bit
		FCTL3 = FWKEY + LOCK; // Reset LOCK bit
	}

	__enable_interrupt();
}
