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

#include "../system.h"
#include "../framework/queue.h"
#include "../dae/fs.h"

void PMM_SetVCore (uint8_t level)
{

}

void system_init(uint8_t* tx_buffer, uint16_t tx_buffer_size, uint8_t* rx_buffer, uint16_t rx_buffer_size)
{
    // TODO not hardware specific
    queue_init_with_header(&tx_queue, tx_buffer, tx_buffer_size, 1, 30);
    queue_init(&rx_queue, rx_buffer, rx_buffer_size, 1);

    file_handler fh;
    fs_open(&fh, DA_FILE_ID_UID, file_system_user_root, file_system_access_type_read);
    device_id = fs_get_data_pointer(&fh, 0);
}

void system_watchdog_timer_stop()
{

}

void system_watchdog_timer_start()
{

}

void system_watchdog_timer_reset()
{

}

void system_watchdog_timer_enable_interrupt()
{

}

void system_watchdog_timer_init(unsigned char clockSelect, unsigned char clockDivider)
{

}

void system_watchdog_init(unsigned char clockSelect, unsigned char clockDivider)
{

}

void system_lowpower_mode(unsigned char mode, unsigned char enableInterrupts)
{

}

void system_get_unique_id(unsigned char *tagId)
{

}
