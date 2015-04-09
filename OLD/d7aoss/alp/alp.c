/*! \file nwl.c
 *

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
 * \author maarten.weyn@uantwerpen.be
 *
 */


#include "alp.h"
#include "../hal/system.h"

/** \copydoc alp_create_structure_for_tx */

void alp_create_structure_for_tx(uint8_t flags, uint8_t id, uint8_t nr_of_templates, ALP_Template* templates)
{
	queue_clear(&tx_queue);
	// TODO: check if push was ok
	queue_push_u8(&tx_queue, flags);
	queue_push_u8(&tx_queue, 3);  // Length changes after calculation of the total lenght
	queue_push_u8(&tx_queue, id);

	uint8_t i = 0;
	uint8_t total_length = 3;
	for (; i<nr_of_templates; i++)
	{
		queue_push_u8(&tx_queue, templates[i].op);
		total_length++;

		//uint8_t length = 0;
		switch (templates[i].op)
		{
			case ALP_OP_READ_DATA: // File Data Template (no data)
			case ALP_OP_READ_ALL:
			{
				ALP_File_Data_Template* templ = (ALP_File_Data_Template*) templates[i].data;

				queue_push_u8(&tx_queue, templ->file_id);
				queue_push_u8(&tx_queue, templ->start_byte_offset >> 8);
				queue_push_u8(&tx_queue, templ->start_byte_offset & 0xFF);
				queue_push_u8(&tx_queue, templ->bytes_accessing >> 8);
				queue_push_u8(&tx_queue, templ->bytes_accessing & 0xFF);

				total_length += 5;
				break;
			}
			case ALP_OP_WRITE_DATA: // File Data Template
			case ALP_OP_WRITE_FLUSH:
			case ALP_OP_RESP_DATA:
			case ALP_OP_RESP_ALL: // FILE Header + Data Template
			{
				ALP_File_Data_Template* templ = (ALP_File_Data_Template*) templates[i].data;

				queue_push_u8(&tx_queue, templ->file_id);
				queue_push_u8(&tx_queue, templ->start_byte_offset >> 8);
				queue_push_u8(&tx_queue, templ->start_byte_offset & 0xFF);
				queue_push_u8(&tx_queue, templ->bytes_accessing >> 8);
				queue_push_u8(&tx_queue, templ->bytes_accessing & 0xFF);
				queue_push_u8_array(&tx_queue, templ->data, templ->bytes_accessing);

				total_length += 5 + templ->bytes_accessing;
				break;
			}
			case ALP_OP_READ_HEADER: // File ID Template
			case ALP_OP_ACTION_EXIST:
			case ALP_OP_ACTION_DELETE:
			case ALP_OP_ACTION_RESTORE:
			case ALP_OP_ACTION_FLUSH:
			case ALP_OP_ACTION_OPEN:
			case ALP_OP_ACTION_CLOSE:
				total_length += 1;
				break;
			case ALP_OP_WRITE_PROP: // File Header Template
			case ALP_OP_ACTION_CREATE:
			case ALP_OP_RESP_HEADER:
				total_length += templates[i].data[2] + 3;
				break;
			case ALP_OP_RESP_ERROR: // File Error Template
				total_length += 2;
				break;
		}

	}

	tx_queue.front[1] = total_length;
}
