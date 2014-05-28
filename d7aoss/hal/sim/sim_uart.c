/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 *     	glenn.ergeerts@uantwerpen.be
 *
 */

#include "../../simulation/Castalia/src/node/application/oss7Test/SimHalInterface.h"

void uart_init()
{

}

void uart_enable_interrupt()
{

}

void uart_transmit_data(unsigned char data)
{

}

void uart_transmit_message(unsigned char *data, unsigned char length)
{
    log_msg(data, length);
}

unsigned char uart_tx_ready()
{
	return 0;
}

unsigned char uart_receive_data()
{
	return 0;
}
