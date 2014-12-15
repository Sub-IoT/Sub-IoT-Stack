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
#include "../alp/alp.h"
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


/*! \brief Opens a file (File System)
 *
 *  Checks the correct file id, user and access type and sets the file_handler to the correct file.
 *
 *  \param file_handle A pointer to the File Handle which will be set.
 *  \param file_id The ID of the file to be opened.
 *  \param user The user (root, user, guest).
 *  \param access_type The access type (read, write, run)
 *  \return Returs success or fail (0: OK, 1: File not present, 2: wrong permissions)
 */
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
	file_handle->file = (uint8_t*) &filesystem_files[MERGEUINT16(fi->file_offset[0], fi->file_offset[1])];

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

	if (MERGEUINT16(fh->info->header.length[0],fh->info->header.length[1]) > offset)
	{
		return ((uint8_t*)fh->file)[offset];
	}

	return 0;
}

uint16_t fs_read_short(file_handler *fh, uint8_t offset)
{
	if (fh == NULL)
			return 0;

	if (MERGEUINT16(fh->info->header.length[0],fh->info->header.length[1]) > (uint16_t) (offset + 1))
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

	if (MERGEUINT16(fh->info->header.length[0],fh->info->header.length[1]) > (uint16_t) (offset + length - 1))
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

	if (MERGEUINT16(fh->info->header.length[0],fh->info->header.length[1]) > (uint16_t) (offset + 1))
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

	if ((fh->permission_mask != 0)  && !(fh->permission_mask & DA_PERMISSION_CODE_WRITE_MASK))
		return 2;

	if (MERGEUINT16(fh->info->header.length[0],fh->info->header.length[1]) < offset + length)
	{
		return 3;
	}

	bool do_notify = false;

	if (fh->info->header.properties_flags & DA_NOTIFICATION_NOTIFY)
	{
		// Notification flag is set for this file
		if (fh->info->header.properties_flags & DA_NOTIFICATION_ACCESS_WRITE)
		{
			do_notify = fs_check_notification_query(fh->info->header.properties_file_id, fh, data, length);
		}
	}

	if (store)
	{
		// Check if storage class is Restorable
		if ((fh->permission_mask != 0)  && ((fh->info->header.properties_flags & 0x03) != (uint8_t) DataElementStorageClassRestorable))
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
		uint8_t nr_of_unitary_queries = fs_read_byte(&nf, nf_pointer++);
		if (nr_of_unitary_queries > 1)
		{
			uint8_t logical_code = fs_read_byte(&nf, nf_pointer++);
			//TODO: implement multiple queries in notification
			return false;
		}

		D7AQP_Unitary_Query_Template *unitary_queries  = (D7AQP_Unitary_Query_Template*) fs_get_data_pointer(&nf, nf_pointer);

//		if (query.unitary_queries[0].file_id != file_id)
//		{
//			return false;
//			//TODO: check and implement notification where query uses other file then writing file
//		}

		nf_pointer += 4;

		if ((unitary_queries[0].compare_code & D7AQP_QUERY_COMP_CODE_MASKED)) // masked not implemented
		{
			return false;
			//TODO: implement mask in query
		}

		uint8_t* compare_value;

		if ((unitary_queries[0].compare_code & D7AQP_QUERY_COMP_CODE_COMPTYPE_NONNULL))
		{
			uint8_t i = 0;
			while (i < length)
			{
				if (data[i++] != 0)
					return true;
			}
		} else if ((unitary_queries[0].compare_code & D7AQP_QUERY_COMP_CODE_COMPTYPE_ARITHM))
		{
			if ((unitary_queries[0].compare_code & D7AQP_QUERY_COMP_CODE_VALTYPE)) // compare with previous file value
			{
				compare_value = fs_get_data_pointer(fh, unitary_queries[0].file_offset);
			}
			else
			{
				compare_value = fs_get_data_pointer(&nf, nf_pointer);
				nf_pointer += unitary_queries[0].compare_length;
			}

			switch (unitary_queries[0].compare_code & D7AQP_QUERY_COMP_CODE_PARAMS(0x0F))
			{
				case 0: // inequality
				{
					if (memcmp(compare_value, data, unitary_queries[0].compare_length) != 0)
						return true;

					break;
				}
				case 1:
				{
					if (memcmp(compare_value, data, unitary_queries[0].compare_length) == 0)
						return true;

					break;
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

	uint8_t spectrum_id[2];
	spectrum_id[0] = fs_read_byte(&nf, nf_pointer++);
	spectrum_id[1] = fs_read_byte(&nf, nf_pointer++);
	int8_t tx_eirp = ((int8_t) ((fs_read_byte(&nf, nf_pointer++) & 0x7F) / 2) - 20) ;
	int8_t subnet = fs_read_byte(&nf, nf_pointer++);

	uint8_t* target_id = NULL;
	uint8_t target_id_length = 0;
	if (tnf_flags & 0x02)
	{
		target_id = fs_get_data_pointer(&nf, nf_pointer);
		if (tnf_flags & 0x04)
			target_id_length= 2;
		else
			target_id_length= 8;

		nf_pointer += target_id_length;
	}

	//TODO: use target_id in query

	trans_execute_query(fs_get_data_pointer(&nf, nf_pointer), ALP_REC_FLG_TYPE_UNSOLICITED, file_system_user_root, subnet, spectrum_id, tx_eirp, target_id_length, target_id);

/*
	uint8_t alp_rec_flags =  fs_read_byte(&nf, nf_pointer++);
	uint8_t alp_rec_length =  fs_read_byte(&nf, nf_pointer++);
	uint8_t alp_alp_id =  fs_read_byte(&nf, nf_pointer++);
	uint8_t alp_op =  fs_read_byte(&nf, nf_pointer++);
	uint8_t alp_id =  fs_read_byte(&nf, nf_pointer++);
	uint8_t alp_offset=  SWITCH_BYTES(fs_read_short(&nf, nf_pointer));
	nf_pointer += 2;
	uint8_t alp_length =  SWITCH_BYTES(fs_read_short(&nf, nf_pointer));
	nf_pointer += 2;

	// TODO: only singular templates are currently supported
	uint8_t alp_nr_of_templates = 1;

	ALP_File_Data_Template data_template;
	ALP_Template alp_template;

	alp_template.op = ALP_OP_RESP_DATA;
	alp_template.data = (uint8_t*) &data_template;

	if (alp_op == ALP_OP_READ_DATA)
	{
		data_template.file_id = alp_id;
		data_template.start_byte_offset = alp_offset;
		data_template.bytes_accessing = alp_length;

		file_handler fh;
		fs_open(&fh, alp_id, file_system_user_root, file_system_access_type_read);

		data_template.data = fs_get_data_pointer(&fh, (uint8_t) alp_offset);
	}

	alp_create_structure_for_tx(ALP_REC_FLG_TYPE_UNSOLICITED, alp_alp_id, alp_nr_of_templates, &alp_template);
	trans_tx_query(NULL, subnet, spectrum_id, tx_eirp);
*/
}
