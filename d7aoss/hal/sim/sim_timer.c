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

#include "../timer.h"
#include "../framework/timer.h"

#include "../../simulation/Castalia/src/node/application/oss7Test/SimHalInterface.h"

void hal_timer_init()
{
	
}

void hal_timer_enable_interrupt()
{

}

void hal_timer_disable_interrupt()
{

}

void hal_timer_clear_interrupt( void )
{

}

uint16_t hal_timer_getvalue()
{
    return 0; // TODO
}

void hal_timer_setvalue(uint16_t next_event)
{
    set_timer(next_event);
}

void hal_timer_counter_reset()
{

}

void hal_benchmarking_timer_init()
{

}

void hal_benchmarking_timer_start()
{

}

void hal_benchmarking_timer_stop()
{

}

uint32_t hal_benchmarking_timer_getvalue()
{

}

void sim_timer_completed()
{
    timer_completed();
}
