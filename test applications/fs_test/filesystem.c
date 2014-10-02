/*
 * filesystem.c
 *
 *  Created on: 31-jul.-2013
 *      Author: Maarten Weyn
 */

#ifndef __FILESYSTEM_C
#define __FILESYSTEM_C

#include "types.h"
#include "dae/fs.h"


uint8_t filesystem_info_nr_files = 32;

#pragma DATA_SECTION(filesystem_info_bitmap, ".fs_fileinfo_bitmap")
#pragma RETAIN(filesystem_info_bitmap)
const uint8_t filesystem_info_bitmap[32] = {	0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
												0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
												0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
												0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

#pragma DATA_SECTION(filesystem_info_headers, ".fs_fileinfo")
#pragma RETAIN(filesystem_info_headers)
const uint8_t filesystem_info_headers[] = {
		// 0x00: UID
		0x00,0x00,0x03,0x24, 0x00,0x08, 0x00,0x08, 0x00,0x00

};

#pragma DATA_SECTION(filesystem_files, ".fs_files")
#pragma RETAIN(filesystem_files)
const uint8_t filesystem_files[] = {
		// 0x00: UID
		0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08
};

//#define FILESYSTEM_NETWORK_CONFIGURATION_SIZE 		10
//#define FILESYSTEM_DEVICE_FEATURES_SIZE 			32
//#define FILESYSTEM_CHANNEL_CONFIGURATION_SIZE		64
//#define FILESYSTEM_REAL_TIME_SCHEDULER_SIZE			0
//#define FILESYSTEM_SLEEP_SCAN_SCHEDULER_SIZE		32
//#define FILESYSTEM_HOLD_SCAN_SCHEDULER_SIZE			32
//#define FILESYSTEM_BEACON_TRANSMIT_SERIES_SIZE		24
//
//#define FILESYSTEM_NETWORK_CONFIGURATION_LENGTH		10
//#define FILESYSTEM_DEVICE_FEATURES_LENGTH			32
//#define FILESYSTEM_CHANNEL_CONFIGURATION_LENGTH		8
//#define FILESYSTEM_REAL_TIME_SCHEDULER_LENGTH		0
//#define FILESYSTEM_SLEEP_SCAN_SCHEDULER_LENGTH		32
//#define FILESYSTEM_HOLD_SCAN_SCHEDULER_LENGTH		32
//#define FILESYSTEM_BEACON_TRANSMIT_SERIES_LENGTH	24
//
//#define FILESYSTEM_NR_OF_ISFB_FILES					7
//
//// must correspond with your linker file
//#define FILESYSTEM_FILE_INFO_START_ADDRESS			0x8000
//#define FILESYSTEM_FILES_START_ADDRESS				0x8064
//
//const filesystem_address_info fs_info = { FILESYSTEM_FILE_INFO_START_ADDRESS, FILESYSTEM_FILES_START_ADDRESS };
//
//
//// id, address_offset, length, allocation, permissions
//#pragma DATA_SECTION(filesystem_info_headers, ".fs_fileinfo")
//#pragma RETAIN(filesystem_info_headers)
//const uint8_t filesystem_info_headers[] = {
//		/* Number of files */
//		FILESYSTEM_NR_OF_ISFB_FILES, // Number of ISFB files
//
//
//		/* ID=0x00: network configuration - length = 10 - allocation = 10 */
//		ISFB_ID_NETWORK_CONFIGURATION,
//		FILESYSTEM_NETWORK_CONFIGURATION,
//		FILESYSTEM_NETWORK_CONFIGURATION_LENGTH,
//		FILESYSTEM_NETWORK_CONFIGURATION_SIZE,
//		B00111100,
//
//		/* ID=0x01: device features - length = 48 - allocation = 48 */
//		ISFB_ID_DEVICE_FEATURES,
//		FILESYSTEM_DEVICE_FEATURES,
//		FILESYSTEM_DEVICE_FEATURES_LENGTH,
//		FILESYSTEM_DEVICE_FEATURES_SIZE,
//		B00111100,
//
//		/* ID=0x02: Channel Configuration - Length = 8 bytes per channel - Allocation = minimum 64 bytes */
//		ISFB_ID_CHANNEL_CONFIGURATION,
//		FILESYSTEM_CHANNEL_CONFIGURATION,
//		FILESYSTEM_CHANNEL_CONFIGURATION_LENGTH,
//		FILESYSTEM_CHANNEL_CONFIGURATION_SIZE,
//		B00111100,
//
//	    /* ID=0x03: Real Time Scheduler (Optional) - Length = 12 bytes - Allocation = 12 bytes  - currently not used --> 0*/
//		ISFB_ID_REAL_TIME_SCHEDULER,
//		FILESYSTEM_REAL_TIME_SCHEDULER,
//		FILESYSTEM_REAL_TIME_SCHEDULER_LENGTH,
//		FILESYSTEM_REAL_TIME_SCHEDULER_SIZE,
//		B00111100,
//
//		/* ID=0x04:  Sleep Channel Scan Series - Length 4 bytes / channel scan datum - Allocation = min 32 bytes */
//		ISFB_ID_SLEEP_SCAN_SCHEDULER,
//		FILESYSTEM_SLEEP_SCAN_SCHEDULER,
//		FILESYSTEM_SLEEP_SCAN_SCHEDULER_LENGTH,
//		FILESYSTEM_SLEEP_SCAN_SCHEDULER_SIZE,
//		B00111100,
//
//		/* ID=0x05:  Hold Channel Scan Series - Length 4 bytes / channel scan datum - Allocation = min 32 bytes */
//		ISFB_ID_HOLD_SCAN_SCHEDULER,
//		FILESYSTEM_HOLD_SCAN_SCHEDULER,
//		FILESYSTEM_HOLD_SCAN_SCHEDULER_LENGTH,
//		FILESYSTEM_HOLD_SCAN_SCHEDULER_SIZE,
//		B00111100,
//
//		/* ID=0x06: Beacon Transmit Period List - Length = 8 bytes - Allocation = 24 bytes */
//		ISFB_ID_BEACON_TRANSMIT_SERIES,
//		FILESYSTEM_BEACON_TRANSMIT_SERIES,
//		FILESYSTEM_BEACON_TRANSMIT_SERIES_LENGTH,
//		FILESYSTEM_BEACON_TRANSMIT_SERIES_SIZE,
//		B00111100,
//};
//
//#pragma DATA_SECTION(filesystem_files, ".fs_files")
//#pragma RETAIN(filesystem_files)
//const uint8_t filesystem_files[] = {
//		/* ID=0x00: network configuration - length = 10 - allocation = 10 */
//		0x00,0x00,             						// Virtual ID
//	    0xFF,               						// Device Subnet
//	    0xFF,       								// Beacon Subnet
//	    B00100010,B00000110,  						// Active Setting
//	    0x00,               						// Default Device Flags
//	    2,                  						// Beacon Redundancy (attempts)
//	    SPLITUINT16(0x0002),             			// Hold Scan Sequence Cycles
//
//	    /* ID=0x01: device features - length = 32 - allocation = 32 */
//	    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// Device ID
//	    B00100010,B00000110,  						// Supported Setting
//	    127,										// Max Frame Length
//	    1,											// Max Frames Per Packet
//	    SPLITUINT16(0),								// DLLS Available Methods
//	    SPLITUINT16(0),								// NLS Available Methods
//	    SPLITUINT16(0x0600),						// Total ISFB Memory TODO: check this value
//	    SPLITUINT16(0),								// Available ISFB Memory TODO: check this value
//	    SPLITUINT16(0),								// Total ISFSB File Memory TODO: check this value
//	    SPLITUINT16(0),								// Available ISFSB File Memory TODO: check this value
//	    SPLITUINT16(0),								// Total GFB File Memory TODO: check this value
//	    SPLITUINT16(0),								// Available GFB File Memory TODO: check this value
//	    0,											// RFU
//	    1,											// Session Stack Depth TODO: check this
//	    SPLITUINT16(0x0701),						// Firmware & Version - 0x07: OSS-7 / 0x01: v. 01
//
//	    /* ID=0x02: Channel Configuration - Length = 8 bytes per channel - Allocation = minimum 64 bytes */
//	    // Channel 1
//	    0x10,                                       // Channel Spectrum ID
//	    0x00,                                       // Channel Parameters
//	    (uint8_t)(( (-15) + 40 )*2),                // Channel TX Power Limit
//	    100,                                        // Channel Link Quality Filter Level
//	    (uint8_t)( (-85) + 140 ),                   // CS RSSI Threshold
//	    (uint8_t)( (-92) + 140 ),                   // CCA RSSI Threshold
//	    0x00,                                       // Regulatory Code
//	    0x01,                                       // TX Duty Cycle
//
//	    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Channel 2 - dummy data
//	    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Channel 3 - dummy data
//	    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Channel 4 - dummy data
//	    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Channel 5 - dummy data
//	    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Channel 6 - dummy data
//	    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Channel 7 - dummy data
//	    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Channel 8 - dummy data
//
//	    /* ID=0x03: Real Time Scheduler (Optional) - Length = 12 bytes - Allocation = 12 bytes */
//	    // Not used
//
//	    /* ID=0x04:  Sleep Channel Scan Series - Length 4 bytes / channel scan datum - Allocation = min 32 bytes */
//	    // Not used in this example
//	    0xFF, 0xFF, 0xFF, 0xFF,                      // Channel ID, Scan Code, Next Scan ticks
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//
//		/* ID=0x05:  Hold Channel Scan Series - Length 4 bytes / channel scan datum - Allocation = min 32 bytes */
//		// Not used in this example
//		0xFF, 0xFF, 0xFF, 0xFF,                      // Channel ID, Scan Code, Next Scan ticks
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//		0xFF, 0xFF, 0xFF, 0xFF,
//
//		/* ID=0x06: Beacon Transmit Period List - Length = 8 bytes - Allocation = 24 bytes */
//		// Period 1
//		0x10, 										// Channel ID
//		B00000010, 									// Beacon Command Params -> No Response
//		0x02, 0x00, 0x00, 0x00, 					// D7AQP Call Template
//		SPLITUINT16(0x0400),						// Next Beacon -> 1024 ticks = 1 s
//
//		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Period 2 - Not Used
//		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Period 3 - Not Used
//};

#endif
