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
 * \author	glenn.ergeerts@uantwerpen.be
 */

#include "uart_alp_interface.h"

#define UART_RX_BUFFER_SIZE 256
#define UART_SYNC_BYTE 0xCE

#include "types.h"
#include "fifo.h"
#include "alp.h"
#include "hwuart.h"
#include "debug.h"
#include "MODULE_D7AP_defs.h"


static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = { 0 };
static fifo_t uart_rx_fifo;

static void process_uart_rx_fifo()
{
    // expected: <0xCE> <Length byte> <interface> <interface config> <ALP command>
    // where length is the length of interface config and ALP command
    // interface: 0xD7 = D7ASP, 0x00 = own filesystem
    // interface config: D7ASP fifo config in case of interface 0xD7, void for interface 0x00
    if(fifo_get_size(&uart_rx_fifo) > 3)
    {
        uint8_t alp_command[MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE] = { 0x00 };
        fifo_peek(&uart_rx_fifo, alp_command, 0, 3);
        if(alp_command[0] != UART_SYNC_BYTE)
        {
            // unexpected data, pop and return
            fifo_pop(&uart_rx_fifo, alp_command, 1);
            sched_post_task(&process_uart_rx_fifo);
            return;
        }

        uint8_t interface_type = alp_command[2];
        //assert(interface_type == ALP_ITF_ID_D7ASP || interface_type ==  ALP_ITF_ID_FS);
        if(interface_type != ALP_ITF_ID_D7ASP && interface_type !=  ALP_ITF_ID_FS)
        {
            // unexpected data, pop and return
            fifo_pop(&uart_rx_fifo, alp_command, 1);
            sched_post_task(&process_uart_rx_fifo);
            return;
        }

        uint8_t length = alp_command[1];
        if(fifo_get_size(&uart_rx_fifo) >= 3 + length)
        {
            // complete command received
            fifo_pop(&uart_rx_fifo, alp_command, 3); // we don't need the header anymore

            switch(interface_type)
            {
            case ALP_ITF_ID_D7ASP:
                // first pop D7ASP fifo config
                fifo_pop(&uart_rx_fifo, alp_command, D7ASP_FIFO_CONFIG_SIZE);
                d7asp_fifo_config_t fifo_config;
                fifo_config.fifo_ctrl = alp_command[0];
                memcpy(&(fifo_config.qos), alp_command + 1, 4);
                fifo_config.dormant_timeout = alp_command[5];
                fifo_config.start_id = alp_command[6];
                memcpy(&(fifo_config.addressee), alp_command + 7, 9);
                uint8_t alp_command_length = length - D7ASP_FIFO_CONFIG_SIZE;
                fifo_pop(&uart_rx_fifo, alp_command, alp_command_length);
                d7asp_queue_alp_actions(&fifo_config, alp_command, alp_command_length);
                break;
            case ALP_ITF_ID_FS:
                fifo_pop(&uart_rx_fifo, alp_command, length);
                uart_alp_interface_process_command(alp_command, length);
            }
        }

        sched_post_task(&process_uart_rx_fifo);
    }
}

static void uart_rx_cb(char data)
{
    error_t err;
    err = fifo_put(&uart_rx_fifo, &data, 1); assert(err == SUCCESS);
    if(!sched_is_scheduled(&process_uart_rx_fifo))
        sched_post_task(&process_uart_rx_fifo);
}

void uart_alp_interface_init()
{
    fifo_init(&uart_rx_fifo, uart_rx_buffer, sizeof(uart_rx_buffer));

    uart_set_rx_interrupt_callback(&uart_rx_cb);
    uart_rx_interrupt_enable(true);

    sched_register_task(&process_uart_rx_fifo);
}

void uart_alp_interface_process_command(uint8_t* alp_command, uint8_t alp_command_length)
{
    uint8_t alp_response[128] = { 0 };
    uint8_t alp_reponse_length = 0;
    alp_process_command(alp_command, alp_command_length, alp_response, &alp_reponse_length);
    uart_transmit_data(UART_SYNC_BYTE);
    uart_transmit_data(alp_reponse_length + 1);
    uart_transmit_data(ALP_ITF_ID_FS);
    if(alp_reponse_length > 0)
        uart_transmit_message(alp_response, alp_reponse_length);
}

