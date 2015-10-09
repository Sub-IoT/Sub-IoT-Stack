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
#include "debug.h"
#include "platform.h"

#include <stdio.h>
#include <stdlib.h>

#include "hwlcd.h"
#include "d7ap_stack.h"
#include "dll.h"
#include "hwuart.h"
#include "fifo.h"

#define UART_RX_BUFFER_SIZE 50

static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = { 0 };
static fifo_t uart_rx_fifo;

static d7asp_init_args_t d7asp_init_args;

static void process_uart_rx_fifo()
{
    // expected: <0xCE> <Length byte> <0xD7> <D7ASP fifo config> <ALP command>
    // where length is the length of D7ASP fifo config and ALP command
    if(fifo_get_size(&uart_rx_fifo) > 3)
    {
        uint8_t alp_command[MODULE_D7AP_FIFO_COMMAND_BUFFER_SIZE] = { 0x00 };
        fifo_peek(&uart_rx_fifo, alp_command, 0, 3);
        if(alp_command[0] != 0xCE)
        {
            // unexpected data, pop and return
            fifo_pop(&uart_rx_fifo, alp_command, 1);
            return;
        }

        assert(alp_command[2] == ALP_ITF_ID_D7ASP);
        uint8_t length = alp_command[1];
        if(fifo_get_size(&uart_rx_fifo) >= 3 + length)
        {
            // complete command received
            fifo_pop(&uart_rx_fifo, alp_command, 3); // we don't need the header anymore

            // first pop D7ASP fifo config
            fifo_pop(&uart_rx_fifo, alp_command, D7ASP_FIFO_CONFIG_SIZE);
            d7asp_fifo_config_t fifo_config;
            fifo_config.fifo_ctrl = alp_command[0];
            memcpy(&(fifo_config.qos), alp_command + 1, 4);
            fifo_config.dormant_timeout = alp_command[5];
            fifo_config.start_id = alp_command[6];
            memcpy(&(fifo_config.addressee), alp_command + 7, 9);

            // and now ALP command
            uint8_t alp_command_length = length - D7ASP_FIFO_CONFIG_SIZE;
            fifo_pop(&uart_rx_fifo, alp_command, alp_command_length);
            d7asp_queue_alp_actions(&fifo_config, alp_command, alp_command_length);
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

static void on_unsollicited_response_received(d7asp_result_t d7asp_result, uint8_t *alp_command, uint8_t alp_command_size, hw_rx_metadata_t* rx_meta)
{
    //led_on(0);
	// TODO move this to log module so we can reuse this for other applications?
//    uart_transmit_data(ALP_ITF_ID_D7ASP);
//    uart_transmit_data(d7asp_result.status.raw);
//    uart_transmit_data(d7asp_result.fifo_token);
//    uart_transmit_data(d7asp_result.request_id);
//    uart_transmit_data(d7asp_result.response_to);
//    uart_transmit_data(d7asp_result.addressee.addressee_ctrl);
//    uint8_t address_len = d7asp_result.addressee.addressee_ctrl_virtual_id? 2 : 8; // TODO according to spec this can be 1 byte as well?
//    uart_transmit_message(d7asp_result.addressee.addressee_id, address_len);
//    uart_transmit_message(alp_command, alp_command_size);
}

void bootstrap()
{
    dae_access_profile_t access_classes[1] = {
        {
            .control_scan_type_is_foreground = true,
            .control_csma_ca_mode = CSMA_CA_MODE_UNC,
            .control_number_of_subbands = 1,
            .subnet = 0x05,
            .scan_automation_period = 0,
            .transmission_timeout_period = 0xFF, // TODO compressed ticks value
            .subbands[0] = (subband_t){
                .channel_header = {
                    .ch_coding = PHY_CODING_PN9,
                    .ch_class = PHY_CLASS_NORMAL_RATE,
                    .ch_freq_band = PHY_BAND_433
                },
                .channel_index_start = 16,
                .channel_index_end = 16,
                .eirp = 0,
                .ccao = 0
            }
        }
    };

    fs_init_args_t fs_init_args = (fs_init_args_t){
        .fs_user_files_init_cb = NULL,
        .access_profiles_count = 1,
        .access_profiles = access_classes
    };

    d7asp_init_args.d7asp_received_unsollicited_data_cb = &on_unsollicited_response_received;

    d7ap_stack_init(&fs_init_args, &d7asp_init_args);

    fifo_init(&uart_rx_fifo, uart_rx_buffer, sizeof(uart_rx_buffer));

    uart_set_rx_interrupt_callback(&uart_rx_cb);
    uart_rx_interrupt_enable(true);

    sched_register_task(&process_uart_rx_fifo);

    lcd_write_string("started");
}

