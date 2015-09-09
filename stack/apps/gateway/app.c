/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * \author	maarten.weyn@uantwerpen.be
 */

#include "hwuart.h"
#include "hwleds.h"
#include "hwsystem.h"
#include "scheduler.h"
#include "timer.h"
#include "log.h"
#include <assert.h>
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>

#include "hwlcd.h"
#include "d7ap_stack.h"
#include "dll.h"

static void start_foreground_scan()
{
    // TODO we start FG scan manually now, later it should be started by access profile automatically
    dll_start_foreground_scan();
}

static void on_alp_unhandled_action(d7asp_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size)
{
	// TODO move this to log module so we can reuse this for other applications?
    uart_transmit_data(0xD7);
    uart_transmit_data(d7asp_result.status.raw);
    uart_transmit_data(d7asp_result.fifo_token);
    uart_transmit_data(d7asp_result.request_id);
    uart_transmit_data(d7asp_result.response_to);
    uart_transmit_data(d7asp_result.addressee.addressee_ctrl);
    uint8_t address_len = d7asp_result.addressee.addressee_ctrl_virtual_id? 2 : 8; // TODO according to spec this can be 1 byte as well?
    uart_transmit_message(d7asp_result.addressee.addressee_id, address_len);
    uart_transmit_message(alp_command, alp_command_size);
}

void bootstrap()
{
    d7ap_stack_init(&on_alp_unhandled_action);
    sched_register_task(&start_foreground_scan);
    sched_post_task(&start_foreground_scan);

    lcd_write_string("started");
}

