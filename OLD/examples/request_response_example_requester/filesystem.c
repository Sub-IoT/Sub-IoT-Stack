/*

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
												6,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
												0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
												0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
												// Application specific
												0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
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
		// 0x14-18: Reception Notification Files
		// 0x19: Location Data List (Optional)
		// 0x1A: ISO 21451-7 Sensor List (Optional)
		// 0x1B: Application Extension
		// 0x1C: Key Table

};

#pragma DATA_SECTION(filesystem_files, ".fs_files")
#pragma RETAIN(filesystem_files)
const uint8_t filesystem_files[] = {
		// 0x00: UID
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // if UID = 000000000000 -> OSS-7 will create UID based on hardware (is not compliant to spec!!!)
		// 0x01: Device Capacity
		// - 1B: Supported Settings (Device Type: GW, Coding scheme: 0, hi rate: disabled)
		// - 1B: Max Frame Length (128)
		// - 2B: NLS: Not Used Yet
		// - 2B: Total Permanent Memory: Not Used Yet
		// - 2B: Total Volatile Memory: Not Used Yet
		// - 2B: Session Stack Depth: 1 - Not Used Yet
		// - 2B: Firmware & Version: 07 - 0
		0xC0, 0x80, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x01, 0x07,0x00,

		// 0x02: Device Status
		0x00, 0x00,0x00, 0x00,0x00,
		// 0x03: PHY Configuration
		// - 1B: Preamble size: 4 bytes
		// - 1B: Max TX EIRP: 0x64 = 10 dBm
		// - 1B: ISM Band: 0: 433 MHz
		// - 1B: Regulatoins: 0: Not Used Yet
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
		// 0x14-18: Reception Notification Files
		// 0x19: Location Data List (Optional)
		// 0x1A: ISO 21451-7 Sensor List (Optional)
		// 0x1B: Application Extension
		// 0x1C: Key Table
};

#endif
