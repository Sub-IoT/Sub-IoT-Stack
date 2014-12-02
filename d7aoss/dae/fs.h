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
 *		.fs_fileinfo_bitmap : 	{} > FLASH_FS1
 *  	.fs_fileinfo: 			{} > FLASH_FS1
 *		.fs_files	: 			{} > FLASH_FS2
 *
 *	Add FLASH_FS_FI and FLASH_FS_FILES to the MEMORY section
 *  eg.
 *  	FLASH_FS1               : origin = 0xC000, length = 0x0200 // The file headers
 *	    FLASH_FS2               : origin = 0xC200, length = 0x0400 // The file contents
 */

#ifndef FS_H_
#define FS_H_


#include "../hal/system.h"
#include "data_elements.h"

//TODO: uint16_t is only for 16 bit address, should be HW depended


typedef struct
{
	Data_Element_File_Header header;
	uint8_t file_offset[2];
} file_info;

typedef struct
{
	file_info *info;
	uint8_t *file;
	uint8_t permission_mask;
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

extern const uint8_t filesystem_info_nr_files;
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


uint8_t fs_write_byte(file_handler *fh, uint8_t offset, uint8_t value, bool store);
uint8_t fs_write_data(file_handler *fh, uint8_t offset, uint8_t* data, uint8_t length, bool store);

static bool fs_check_notification_query(uint8_t tnf_id, file_handler* fh, uint8_t* data, uint8_t length);
static void fs_send_notification(uint8_t tnf_id);



#endif /* FS_H_ */
