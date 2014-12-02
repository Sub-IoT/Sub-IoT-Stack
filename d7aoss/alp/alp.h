/*! \file alp.h
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
 */


#ifndef ALP_H_
#define ALP_H_

#include "../types.h"

#define ALP_REC_FLG_CHUNK_CTRL_INTERMED	(0 << 6)
#define ALP_REC_FLG_CHUNK_CTRL_FIRST	(1 << 6)
#define ALP_REC_FLG_CHUNK_CTRL_LAST		(2 << 6)
#define ALP_REC_FLG_CHUNK_CTRL_SINGLE	(3 << 6)
#define ALP_REC_FLG_TYPE_RESPONSE		  (0 << 6)
#define ALP_REC_FLG_TYPE_UNSOLICITED	  (1 << 6)
#define ALP_REC_FLG_TYPE_COMMAND_NO_RES	  (2 << 6)
#define ALP_REC_FLG_TYPE_COMMAND_RESPONSE (3 << 6)

#define ALP_OP_READ_DATA	0
#define ALP_OP_READ_HEADER	1
#define ALP_OP_READ_ALL		2
#define ALP_OP_WRITE_DATA	4
#define ALP_OP_WRITE_FLUSH	5
#define ALP_OP_WRITE_PROP	6
#define ALP_OP_ACTION_EXIST	16
#define ALP_OP_ACTION_CREATE 17
#define ALP_OP_ACTION_DELETE 18
#define ALP_OP_ACTION_RESTORE 19
#define ALP_OP_ACTION_FLUSH	20
#define ALP_OP_ACTION_OPEN	21
#define ALP_OP_ACTION_CLOSE	22
#define ALP_OP_RESP_DATA	32
#define ALP_OP_RESP_HEADER	33
#define ALP_OP_RESP_ALL		34
#define ALP_OP_RESP_ERROR	255

#define ALP_FILE_ERROR_CODE_OK				0
#define ALP_FILE_ERROR_CODE_NOT_EXIST		1
#define ALP_FILE_ERROR_CODE_ALREADY_EXIST	2
#define ALP_FILE_ERROR_CODE_NOT_RESTORABLE	3
#define ALP_FILE_ERROR_CODE_NO_PERMISSION	4
#define ALP_FILE_ERROR_CODE_LENGTH_TO_BIG	5
#define ALP_FILE_ERROR_CODE_ALLOC_T0_BIG		6
#define ALP_FILE_ERROR_CODE_OFFSET_OFB		7
#define ALP_FILE_ERROR_CODE_DATA_TO_BIG		8
#define ALP_FILE_ERROR_CODE_UNKOWN			255


typedef struct
{
	uint8_t record_flags;
	uint8_t record_lenght;
	uint8_t alp_id;
	uint8_t* alp_templates;
} ALP_Record_Structure;

typedef struct
{
	uint8_t op;
	uint8_t* data;
} ALP_Template;

/** @struct ALP_File_Data_Template
 *  @brief 8.2.1 Contains the data template of an ALP File Data Template
 *  @var ALP_File_Data_Template::file_id
 *  Field 'file_id' contains the file id the of the file
 *  @var ALP_File_Data_Template::start_byte_offset
 *  Field 'start_byte_offset' contains the offset from which the file is transmitted.
 *  @var ALP_File_Data_Template::bytes_accessing
 *  Field 'bytes_accessing' contains the total length of the filedata which is transmitted.
 *  @var ALP_File_Data_Template::data
 *  Field 'data' contains the returned file data.
 *  **/

typedef struct {
	uint8_t file_id;
	uint16_t start_byte_offset;
	uint16_t bytes_accessing;
	uint8_t* data;
} ALP_File_Data_Template;

typedef struct {
	uint8_t file_id;
	uint8_t	start_byte_offset;
	uint8_t bytes_accessing;
	uint8_t* data;
} ALP_File_Header_Template;

typedef struct {
	uint8_t file_id;
} ALP_File_Id_Template;

typedef struct {
	uint8_t errorneous_file_id;
	uint8_t file_error_code;
} ALP_File_Error_Template;


/*! \brief Formats the ALP record structure and adds it to the TX Queue  (Application Layer)
 *
 *  \param uint8_t 			flags 		The Flags for the ALP record structre
 *  \param uint8_t 			id 			The ALP Id.
 *  \param uint8_t 			lenght 		The lenght of the array of ALP templates
 *  \paral ALP_Template*	templates	The array of ALP_Templates
 */
void alp_create_structure_for_tx(uint8_t flags, uint8_t id, uint8_t length, ALP_Template* templates);


#endif /* ALP_H_ */
