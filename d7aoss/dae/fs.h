/*! \file fs.h
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
 * \author maarten.weyn@uantwerpen.be
 *
 * \brief The File System API
 *
 *	Add following sections to the SECTIONS in .cmd linker file to use the filesystem
 *		.fs_fileinfo			: {} > FLASH_FS_FI
 *		.fs_files				: {} > FLASH_FS_FILES
 *
 *	Add FLASH_FS_FI and FLASH_FS_FILES to the MEMORY section
 *  eg.
 *  	FLASH_FS_FI	            : origin = 0xC000, length = 0x0064
 *  	FLASH_FS_FILES	        : origin = 0xC064, length = 0x019C
 */

#ifndef PRES_H_
#define PRES_H_

// must correspond with your linker file
//TODO: check how this could be in filesystem.c not in d7oos
//#define FILESYSTEM_FILE_INFO_START_ADDRESS			0x8000
//#define FILESYSTEM_FILES_START_ADDRESS				0x8064


/*
#define FILESYSTEM_NETWORK_CONFIGURATION 			0
#define FILESYSTEM_DEVICE_FEATURES 					FILESYSTEM_NETWORK_CONFIGURATION + FILESYSTEM_NETWORK_CONFIGURATION_SIZE
#define FILESYSTEM_CHANNEL_CONFIGURATION			FILESYSTEM_DEVICE_FEATURES + FILESYSTEM_DEVICE_FEATURES_SIZE
#define FILESYSTEM_REAL_TIME_SCHEDULER				FILESYSTEM_CHANNEL_CONFIGURATION + FILESYSTEM_CHANNEL_CONFIGURATION_SIZE
#define FILESYSTEM_SLEEP_SCAN_SCHEDULER				FILESYSTEM_REAL_TIME_SCHEDULER + FILESYSTEM_REAL_TIME_SCHEDULER_SIZE
#define FILESYSTEM_HOLD_SCAN_SCHEDULER				FILESYSTEM_SLEEP_SCAN_SCHEDULER + FILESYSTEM_SLEEP_SCAN_SCHEDULER_SIZE
#define FILESYSTEM_BEACON_TRANSMIT_SERIES			FILESYSTEM_HOLD_SCAN_SCHEDULER + FILESYSTEM_HOLD_SCAN_SCHEDULER_SIZE
*/



#include "../hal/system.h"
#include "data_elements.h"

//TODO: uint16_t is only for 16 bit address, should be HW depended


typedef struct
{
	Data_Element_File_Header header;
	uint16_t file_offset;
} file_info;

typedef struct
{
	file_info *info;
	uint8_t *file;
} file_handler;

typedef enum
{
	file_system_user_root,
	file_system_user_user,
	file_system_user_guest
} file_system_user;

typedef enum
{
	file_system_access_type_read,
	file_system_access_type_write,
	file_system_access_type_run
} file_system_access_type;

extern uint8_t filesystem_info_nr_files;
extern const uint8_t filesystem_info_bitmap[];
extern const uint8_t filesystem_info_headers[];
extern const uint8_t filesystem_files[];


void fs_init();

/** Opens a file and gives file_handler and return code
 * 	@param fh the returned file handler
 * 	@param file_id the id of the file for file series
 * 	@param user the user (root, user or guest)
 * 	@param access_type the access type (read, write, run)
 * 	@return status variable: 0: succes, 1 file not found, 2 incorrect user rights
 */
uint8_t fs_open(file_handler * file_handle, uint8_t file_id, file_system_user user, file_system_access_type access_type);

uint8_t fs_close(file_handler *fh);

uint8_t fs_read_byte(file_handler *fh, uint8_t offset);
uint16_t fs_read_short(file_handler *fh, uint8_t offset);
uint8_t fs_read_data(file_handler *fh, uint8_t *data_array, uint8_t offset, uint8_t length);
uint8_t* fs_get_data_pointer(file_handler *fh, uint8_t offset);





#endif /* PRES_H_ */
