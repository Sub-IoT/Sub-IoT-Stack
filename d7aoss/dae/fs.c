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
 */


#include "fs.h"


void fs_init()
{
//	uint8_t i = 0;
//	file_handler* fh = NULL;
//	for (;i<filesystem_info_nr_files;i++)
//	{
//		fh = fs_open(i, file_system_user_root, file_system_user_root, file_system_access_type_read);
//	}
}


uint8_t fs_open(file_handler * file_handle, uint8_t file_id, file_system_user user, file_system_access_type access_type)
{
	if (file_id >= filesystem_info_nr_files)
		// custom Files not supported yet
		return 1;


	if (file_id > 0)
	{
		if (filesystem_info_bitmap[file_id] == 0)
			return 1; // not present
	} else {
		if (filesystem_info_bitmap[file_id] == 0)
		{
			Data_Element_File_Header* header = (Data_Element_File_Header*)(filesystem_info_headers);
			if ((header->properties_3_file_id != 0) || (SWITCH_BYTES(header->length) == 0))
							return 1; // no present
		}

	}


	file_info* fi = (file_info*) (&filesystem_info_headers[6*filesystem_info_bitmap[file_id]]);

	uint8_t permission_mask = 0;
	// TODO: validate correct user
	if (user == file_system_user_user)
	{
		permission_mask |= DA_PERMISSION_CODE_USER_MASK;
	} else if (user == file_system_user_guest)
	{
		permission_mask |= DA_PERMISSION_CODE_GUEST_MASK;
	}

	if (access_type == file_system_access_type_read)
	{
		permission_mask &= DA_PERMISSION_CODE_READ_MASK;
	} else if (access_type == file_system_access_type_write)
	{
		permission_mask &= DA_PERMISSION_CODE_WRITE_MASK;
	} else if (access_type == file_system_access_type_run)
	{
		permission_mask &= DA_PERMISSION_CODE_RUN_MASK;
	}

	if ((user != file_system_user_root) && !(fi->header.properties_0_permissions & permission_mask))
	{
		return 2;
	}

	file_handle->info = fi;
	file_handle->file = (uint8_t*) &filesystem_files[SWITCH_BYTES(fi->file_offset)];

	return 0;
}

uint8_t fs_close(file_handler *fh)
{

	if (fh != NULL)
	{
		fh->file = NULL;
		fh->info = NULL;
		return 0;
	}


	return 1;
}

uint8_t fs_read_byte(file_handler *fh, uint8_t offset)
{
	if (fh == NULL)
		return 0;

	if (SWITCH_BYTES(fh->info->header.length) > offset)
	{
		return ((uint8_t*)fh->file)[offset];
	}

	return 0;
}

uint16_t fs_read_short(file_handler *fh, uint8_t offset)
{
	if (fh == NULL)
			return 0;

	if (SWITCH_BYTES(fh->info->header.length) > offset + 1)
	{
		uint8_t* ptr = fh->file;
		return *(uint16_t*) (&ptr[offset]);
	}

	return 0;
}


uint8_t fs_read_data(file_handler *fh, uint8_t *data_array, uint8_t offset, uint8_t length)
{
	if (fh == NULL)
			return 0;

	if (SWITCH_BYTES(fh->info->header.length) > offset + length - 1)
	{
		uint8_t* ptr = fh->file;
		memcpy(data_array, &ptr[offset], length);
		return length;
	}

	return 0;
}


uint8_t* fs_get_data_pointer(file_handler *fh, uint8_t offset)
{
	if (fh == NULL)
		return 0;

	if (SWITCH_BYTES(fh->info->header.length) > offset + 1)
	{
		return (uint8_t*) (&fh->file[offset]);
	}

	return 0;
}
