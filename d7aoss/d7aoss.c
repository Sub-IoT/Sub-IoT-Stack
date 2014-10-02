/*! \file d7aoss.c
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
 *	\brief High Level API for OSS-7
 *
 *  The high level API to be used by applications which use the dash7 stack
 */


#include "d7aoss.h"

/** \copydoc d7aoss_init */

void d7aoss_init(uint8_t* tx_buffer, uint16_t tx_buffer_size, uint8_t* rx_buffer, uint16_t rx_buffer_size)
{
	system_init(tx_buffer, tx_buffer_size, rx_buffer, rx_buffer_size);
	fs_init();

	trans_init();
}
