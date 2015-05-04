/*!
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
