/*! \file fs.c
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
 * \brief The File System API implementation
 */*


#include "fs.h"


static const filesystem_info *fs_info;

void fs_init(const filesystem_info *file_system_info)
{
	fs_info =file_system_info;
}


uint8_t fs_open(file_handler *fh uint8_t file_id, file_system_user user, file_system_access_type access_type)
{
	if (file_id >= 32)
		// custom Files not supported yet
		return 255

	/*

	uint8_t file_id_map = file_id / 8;
	uint8_t file_id_map_id = file_id % 8;


	if ((fs_info->fs_bitmap->file_bitmap_D7A[file_id_map] >> file_id_map_id) == 0)
			return 1; // not present



	file_info *fi = (void*) (fs_address_info->file_info_start_address + sizeof(filesystem_info) + (sizeof(file_info) * file_id));

	uint8_t permission_mask = 0;
	// TODO: validate correct user
	if (user == file_system_user_user)
	{
		permission_mask |= PERMISSION_CODE_USER_MASK;
	} else if (user == file_system_user_guest)
	{
		permission_mask |= PERMISSION_CODE_GUEST_MASK;
	}

	if (access_type == file_system_access_type_read)
	{
		permission_mask &= PERMISSION_CODE_READ_MASK;
	} else if (access_type == file_system_access_type_write)
	{
		permission_mask &= PERMISSION_CODE_WRITE_MASK;
	} else if (access_type == file_system_access_type_run)
	{
		permission_mask &= PERMISSION_CODE_RUN_MASK;
	}

	if ((user != file_system_user_root) && !(fi->permissions & permission_mask))
		return 2;

	fh->file_info = fi;
	fh->file = (void*) (fs_address_info->files_start_address + fi->offset);

*/

	return 0;
}

uint8_t fs_close(file_handler *fh)
{
	/*
	if (fh != NULL)
	{
		fh->file = NULL;
		fh->file_info = NULL;
		return 0;
	}
	*/

	return 1;
}

uint8_t fs_read_byte(file_handler *fh, uint8_t offset)
{
	/*
	if (fh->file_info->length > offset)
	{
		uint8_t* data = fh->file;
		return data[offset];
	}
	else
	{
		return 0;
	}
	*/
}

uint16_t fs_read_short(file_handler *fh, uint8_t offset)
{
	/*
	if (fh->file_info->length > offset + 1)
	{
		uint8_t* ptr = fh->file;
		return *(uint16_t*) (&ptr[offset]);
	}
	else
	{
		return 0;
	}
	*/
}


uint8_t fs_read_data(file_handler *fh, uint8_t *data_array, uint8_t offset, uint8_t length)
{
	/*
	if (fh->file_info->length > offset + length - 1)
	{
		uint8_t* ptr = fh->file;
		memcpy(data_array, &ptr[offset], length);
		return 0;
	}
	*/
	return 1;
}
