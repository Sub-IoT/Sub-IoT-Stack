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
 *     maarten.weyn@uantwerpen.be
 *
 * 	Example of the notification system
 *
 * 	File System Example
 */

#ifndef __FILESYSTEM_C
#define __FILESYSTEM_C

#include "types.h"
#include "dae/fs.h"


uint8_t const filesystem_info_nr_files = 40; // first 32 - DA7, next 8 application dependent

#pragma DATA_SECTION(filesystem_info_bitmap, ".fs_fileinfo_bitmap")
#pragma RETAIN(filesystem_info_bitmap)

const uint8_t filesystem_info_bitmap[40] = {	// D7 Specific
												0,1,2,3,4,0xFF,5,0xFF,
												6,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,7,
												0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
												0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
												// Appication specific
												8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
												};

#pragma DATA_SECTION(filesystem_info_headers, ".fs_fileinfo")
#pragma RETAIN(filesystem_info_headers)
const uint8_t filesystem_info_headers[] = {
		// Notification File ID - Notify/Access/Qos/Storage Class - Permissions - Length(2Bytes) - Allocated Length (2Bytes) - File Offset (2Bytes)


		// 0x00: UID
		0x00,0x03,0x24, 0x00,0x08, 0x00,0x08, 0x00,0x00,
		// 0x01: Device Capacity
		0x00,0x03,0x24, 0x00,0x0C, 0x00,0x0C, 0x00,0x08,
		// 0X02: Device Status
		0x00,0x03,0x24, 0x00,0x05, 0x00,0x05, 0x00,0x14,
		// 0x03: PHY Configuration
		0x00,0x03,0x26, 0x00,0x04, 0x00,0x04, 0x00,0x19,
		// 0x04: Channel Configuration
		0x00,0x03,0x34, 0x00,0x06, 0x00,0x06, 0x00,0x1D,
		// 0x05: Autoscale
		// 0x06: DLL Configuration
		0x00,0x03,0x34, 0x00,0x04, 0x00,0x04, 0x00,0x23,
		// 0x07: Scan scheduler
		// 0x08: Scan series
		0x00,0x03,0x34, 0x00,0x07, 0x00,0x07, 0x00,0x27,
		// 0x09: Root Authentication Key (Optional)
		// 0x0A: User Authentication Key (Optional)
		// 0x0B: Routing Code (Optional)
		// 0x0C: Default Destination Access Template
		// 0x0E: Default Local Access Template
		// 0x0F-13: Transmission Notification Files
		// 0x0F: App Temperature Notification File
		0x00,0x02,0x34, 0x00,0x15, 0x00,0x15, 0x00,0x2E,
		// 0x14-18: Reception Notification Files
		// 0x19: Location Data List (Optional)
		// 0x1A: ISO 21451-7 Sensor List (Optional)
		// 0x1B: Application Extension
		// 0x1C: Key Table

		// 0x20: App Temperature
		0x0F,0x82,0x36, 0x00,0x06, 0x00,0x06, 0x00,0x42
};

#pragma DATA_SECTION(filesystem_files, ".fs_files")
#pragma RETAIN(filesystem_files)
const uint8_t filesystem_files[] = {
		// 0x00: UID
		0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
		// 0x01: Device Capacity
		0x40, 0x80, 0x00,0x00, 0x02,0x00, 0x02,0x00, 0x00,0x0A, 0x07,0x00,
		// 0x02: Device Status
		0x00, 0x02,0x00, 0x02,0x00,
		// 0x03: PHY Configuration
		0x04, 0x64, 0x00, 0x00,
		// 0x04: Channel Configuration - ecca -110 - 90
		0xFF,0xFF, 0x65, 0x00, 0x32, 0x1E,
		// 0x05: Autoscale
		// 0x06: DLL Configuration
		0xFF, 0xFF, 0x00,0x00,
		// 0x07: Scan scheduler
		// 0x08: Scan series
		0x04,0x00, 0x01, 0x03,0x38, 0x03,0x38,
		// 0x09: Root Authentication Key (Optional)
		// 0x0A: User Authentication Key (Optional)
		// 0x0B: Routing Code (Optional)
		// 0x0C: Default Destination Access Template
		// 0x0E: Default Local Access Template
		// 0x0F-13: Transmission Notification Files
		// 0x0F: App Temperature Notification File

			// TNF Flags
			0x01, // Query Defined, no target address

			// Query Template
			0x05, // Length of Query Template
			0x01, // 1 Query

			// Query 1
			0x20,0x02,0x02,0x50,// File ID 0x20, file offset 2 , compare length 2 , previous value - arithmetic comp - inequality

			// Channel ID
			0x04, 0x00, // Band 0: 433 / Class 1: Normal Rate / Coding 0: PN9, Index 0: 433.164
			0x64, // TX EIRP 2 x (10 dBm + 50)
			// Subnet
			0xFF,
			// No device id since broadcast

			// ALP
			0xD0, //record flag: not chucked, unsolictied,
			0x0B, // Record lenght =  3 + alp template length
			0x01, // ALP ID
				// ALP Template
				0x00, // OP Read File Data
				// Fie Data Template
				0x20, // FILE ID
				0x00, 0x02, // Start byte offset
				0x00, 0x04, // Bytes Accessing

		// 0x14-18: Reception Notification Files
		// 0x19: Location Data List (Optional)
		// 0x1A: ISO 21451-7 Sensor List (Optional)
		// 0x1B: Application Extension
		// 0x1C: Key Table
		// 0x20: App Temperature
		0x00,0x01, 0xAA, 0xBB, 0xAA, 0xBB
};

#endif
