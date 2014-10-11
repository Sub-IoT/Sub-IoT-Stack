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
#include "../hal/flash.h"
#include "../trans/trans.h"
#include <string.h>


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


//	if (file_id > 0)
//	{
		if (filesystem_info_bitmap[file_id] == 0xFF)
			return 1; // not present
//	} else {
//		if (filesystem_info_bitmap[file_id] == 0)
//		{
//			Data_Element_File_Header* header = (Data_Element_File_Header*)(filesystem_info_headers);
//			if ((header->properties_3_file_id != 0) || (SWITCH_BYTES(header->length) == 0))
//							return 1; // no present
//		}
//
//	}


	file_info* fi = (file_info*) (&filesystem_info_headers[9*filesystem_info_bitmap[file_id]]);

	file_handle->permission_mask = 0;
	// TODO: validate correct user
	if (user == file_system_user_user)
	{
		file_handle->permission_mask |= DA_PERMISSION_CODE_USER_MASK;
	} else if (user == file_system_user_guest)
	{
		file_handle->permission_mask |= DA_PERMISSION_CODE_GUEST_MASK;
	}

	if (access_type == file_system_access_type_read)
	{
		file_handle->permission_mask &= DA_PERMISSION_CODE_READ_MASK;
	} else if (access_type == file_system_access_type_write)
	{
		file_handle->permission_mask &= DA_PERMISSION_CODE_WRITE_MASK;
	} else if (access_type == file_system_access_type_run)
	{
		file_handle->permission_mask &= DA_PERMISSION_CODE_RUN_MASK;
	}

	if ((user != file_system_user_root) && !(fi->header.properties_permissions & file_handle->permission_mask))
	{
		file_handle->permission_mask = 0;
		return 2;
	}

	file_handle->permission_mask &= fi->header.properties_permissions;

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

uint8_t fs_write_byte(file_handler *fh, uint8_t offset, uint8_t value, bool store)
{
	return fs_write_data(fh, offset, (uint8_t*) &value, 1, store);
}


uint8_t fs_write_data(file_handler *fh, uint8_t offset, uint8_t* data, uint8_t length, bool store)
{
	if (fh == NULL)
		return 1;

	if (!(fh->permission_mask & DA_PERMISSION_CODE_WRITE_MASK))
		return 2;

	if (SWITCH_BYTES(fh->info->header.length) < offset + length)
	{
		return 3;
	}

	bool do_notify = false;

	if (fh->info->header.properties_flags & DA_NOTIFICATION_NOTIFY)
	{
		// Notification flag is set for this file
		if (fh->info->header.properties_flags & DA_NOTIFICATION_ACCESS_WRITE)
		{
			fs_check_notification_query(fh->info->header.properties_file_id, fh, data, length);
		}
	}

	if (store)
	{
		// Check if storage class is Restorable
		if ((fh->info->header.properties_flags & 0x03) != (uint8_t) DataElementStorageClassRestorable)
			return 4;

		write_bytes_to_flash(&fh->file[offset], data, length);
	}

	if (do_notify)
	{
		fs_send_notification(fh->info->header.properties_file_id);
	}



	return 0;
}

bool fs_check_notification_query(uint8_t tnf_id, file_handler* fh, uint8_t* data, uint8_t length)
{
	file_handler nf;
	fs_open(&nf, tnf_id, file_system_user_root, file_system_access_type_read);
	uint8_t nf_pointer = 0;

	uint8_t tnf_flags = fs_read_byte(&nf, nf_pointer++);
	if (tnf_flags & DA_TNF_FLAGS_QUERY) // Query Present
	{
		nf_pointer++; // Lenght of query template
		D7AQP_Query_Template query;
		query.nr_of_unitary_queries = fs_read_byte(&nf, nf_pointer++);
		if (query.nr_of_unitary_queries > 1)
		{
			query.logical_code = fs_read_byte(&nf, nf_pointer++);
		}
		else
		{
			return 255;
			//TODO: implement multiple queries in notification
		}
		query.unitary_queries = (D7AQP_Unitary_Query_Template*) fs_get_data_pointer(&nf, nf_pointer);

//		if (query.unitary_queries[0].file_id != file_id)
//		{
//			return false;
//			//TODO: check and implement notification where query uses other file then writing file
//		}

		nf_pointer += 4;

		if ((query.unitary_queries[0].compare_code & D7AQP_QUERY_COMP_CODE_MASKED)) // masked not implemented
		{
			return 255;
			//TODO: implement mask in query
		}

		uint8_t* compare_value;

		if ((query.unitary_queries[0].compare_code & D7AQP_QUERY_COMP_CODE_COMPTYPE_NONNULL))
		{
			uint8_t i = 0;
			while (i < length)
			{
				if (data[i++] != 0)
					return true;
			}
		} else if ((query.unitary_queries[0].compare_code & D7AQP_QUERY_COMP_CODE_COMPTYPE_ARITHM))
		{
			if ((query.unitary_queries[0].compare_code & D7AQP_QUERY_COMP_CODE_VALTYPE)) // compare with previous file value
			{
				compare_value = fs_get_data_pointer(fh, query.unitary_queries[0].file_offset);
			}
			else
			{
				compare_value = fs_get_data_pointer(&nf, nf_pointer);
				nf_pointer += query.unitary_queries[0].compare_length;
			}

			switch (query.unitary_queries[0].compare_code & D7AQP_QUERY_COMP_CODE_PARAMS(0x0F))
			{
				case 0: // inequality
				{
					if (memcmp(compare_value, data, query.unitary_queries[0].compare_length) != 0)
						return true;
				}
				case 1:
				{
					if (memcmp(compare_value, data, query.unitary_queries[0].compare_length) == 0)
						return true;
				}
				default:
					return false;
					//TODO implemnet other options
			}
		}
	}

	return false;
}

void fs_send_notification(uint8_t tnf_id)
{
	file_handler nf;
	fs_open(&nf, tnf_id, file_system_user_root, file_system_access_type_read);

	uint8_t nf_pointer = 0;

	uint8_t tnf_flags = fs_read_byte(&nf, nf_pointer++);
	if (tnf_flags & DA_TNF_FLAGS_QUERY) // Query Present
	{
		nf_pointer += 1 + fs_read_byte(&nf, nf_pointer); // Skip the query
	}



	//trans_tx_query(D7AQP_Query_Template* query, uint8_t subnet, uint8_t spectrum_id, int8_t tx_eirp);
}
