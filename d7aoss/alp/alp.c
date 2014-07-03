/*! \file nwl.c
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


#include "alp.h"
#include "../hal/system.h"

void alp_create_structure_for_tx(uint8_t flags, uint8_t id, uint8_t nr_of_templates, ALP_Template* templates)
{
	queue_clear(&tx_queue);
	queue_push_u8(&tx_queue, flags);
	queue_push_u8(&tx_queue, 3);  // Length change after calculation of the total lenght
	queue_push_u8(&tx_queue, id);

	uint8_t i = 0;
	for (; i<nr_of_templates; i++)
	{

		switch (templates[i].op)
		{
			case ALP_OP_READ_DATA: // File Data Template
			case ALP_OP_READ_ALL:
			case ALP_OP_WRITE_DATA:
			case ALP_OP_WRITE_FLUSH:
			case ALP_OP_RESP_DATA:
				break;
			case ALP_OP_READ_HEADER: // File ID Template
			case ALP_OP_ACTION_EXIST:
			case ALP_OP_ACTION_DELETE:
			case ALP_OP_ACTION_RESTORE:
			case ALP_OP_ACTION_FLUSH:
			case ALP_OP_ACTION_OPEN:
			case ALP_OP_ACTION_CLOSE:
				break;
			case ALP_OP_WRITE_PROP: // File Header Template
			case ALP_OP_ACTION_CREATE:
			case ALP_OP_RESP_HEADER:
				break;
			case ALP_OP_RESP_ALL: // FILE Header + Data Template
				break;
			case ALP_OP_RESP_ERROR: // File Error Template
				break;
		}
	}

	//queue_push_u8_array(&tx_queue, alp_data, alp_length);

}
