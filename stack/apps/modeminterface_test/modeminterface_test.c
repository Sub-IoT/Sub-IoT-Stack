/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
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
 */

#include <stdio.h>
#include <stdlib.h>

#include "app_defs.h"
#include "modem_interface.h"
#include "log.h"
#include "platform.h"
#include "platform_defs.h"
#include "scheduler.h"
#include "timer.h"
#include "random.h"
#include "string.h"
#include "debug.h"

#ifdef APP_MODEMINTERFACE_TEST_SEND_PING
uint32_t ping_counter = 0;
static void ping_handler(fifo_t* cmd_fifo);
#endif

#ifdef APP_MODEMINTERFACE_TEST_SEND_DATA
uint32_t data_counter = 0;
static void data_handler(fifo_t* cmd_fifo);

#define DATA1_SIZE 200
uint8_t data1[DATA1_SIZE];

#define DATA2_SIZE 50
uint8_t data2[DATA2_SIZE];

#define DATA3_SIZE 100
uint8_t data3[DATA3_SIZE];

uint8_t* datas[] = {data1, data2, data3};
uint8_t datas_size[] = {DATA1_SIZE, DATA2_SIZE, DATA3_SIZE};
uint8_t datas_index = 0;

uint8_t comp_buffer[255];
#else
uint8_t data_buffer[255];
#endif //APP_MODEMINTERFACE_TEST_SEND_DATA

#ifdef APP_MODEMINTERFACE_TEST_SEND_PING
static void send_ping()
{
    uint8_t ping_request[1]={0x01};
    modem_interface_transfer_bytes(ping_request, 1, SERIAL_MESSAGE_TYPE_PING_REQUEST);
    timer_post_task_delay((void (*)(void*)) & ping_handler, TIMER_TICKS_PER_SEC);
}

static void ping_handler(fifo_t* cmd_fifo)
{
    timer_cancel_task((void (*)(void*)) & ping_handler);
    sched_cancel_task((void (*)(void*)) & ping_handler);
    if(cmd_fifo == NULL)
    { // timeout exceeded, modem not responsive
        log_print_error_string("ping did not return, Stopping after %d iterations", ping_counter);
        assert(false);
    }
    ping_counter++;
    if(ping_counter%1000 == 0)
    {
        log_print_string("ping returned %d", ping_counter);
    }
    fifo_skip(cmd_fifo, fifo_get_size(cmd_fifo));
    sched_post_task(&send_ping);
}
#endif //APP_MODEMINTERFACE_TEST_SEND_PING

#ifdef APP_MODEMINTERFACE_TEST_SEND_DATA
static void init_data(uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size;)
    {
        uint32_t random_value = get_rnd();
        for(size_t j = 0; i < size && j < 4; i++, j++)
        {
            data[i] = ((uint8_t *)&random_value)[j];
        }
    }
}

static void send_data()
{
    modem_interface_transfer_bytes(datas[datas_index], datas_size[datas_index], SERIAL_MESSAGE_TYPE_ALP_DATA);
    timer_post_task_delay((void (*)(void*)) & data_handler, TIMER_TICKS_PER_SEC);    
}
static void data_handler(fifo_t* cmd_fifo)
{
    timer_cancel_task((void (*)(void*)) & data_handler);
    sched_cancel_task((void (*)(void*)) & data_handler);
    if(cmd_fifo == NULL)
    { // timeout exceeded, modem not responsive
        log_print_error_string("data%d did not return, Stopping after %d iterations", datas_index + 1, data_counter);
        assert(false);
    }
    assert(fifo_get_size(cmd_fifo) == datas_size[datas_index]);
    fifo_pop(cmd_fifo, comp_buffer, fifo_get_size(cmd_fifo));
    if(memcmp(comp_buffer, datas[datas_index], datas_size[datas_index])!= 0)
    {
        log_print_error_string("Return data doesn't match, Stopping");
        assert(false);
    }
    data_counter++;
    if(data_counter%1000 == 0)
    {
        log_print_string("data returned %d", data_counter);
    }
    datas_index++;
    if (datas_index == sizeof(datas)/sizeof(uint8_t*))
    {
        datas_index = 0;
    }
    fifo_skip(cmd_fifo, fifo_get_size(cmd_fifo));
    sched_post_task(&send_data);
}
#else
static void data_handler(fifo_t* cmd_fifo)
{
    uint16_t size = fifo_get_size(cmd_fifo);
    fifo_pop(cmd_fifo, data_buffer, size);
    modem_interface_transfer_bytes(data_buffer, size, SERIAL_MESSAGE_TYPE_ALP_DATA);
}
#endif //APP_MODEMINTERFACE_TEST_SEND_DATA

void bootstrap()
{
    log_print_string("Device booted\n");

#ifdef FRAMEWORK_MODEM_INTERFACE_USE_INTERRUPT_LINES
    modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, PLATFORM_MODEM_INTERFACE_UART_STATE_PIN, PLATFORM_MODEM_INTERFACE_TARGET_UART_STATE_PIN);
#else
    modem_interface_init(PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, (pin_id_t) 0, (pin_id_t) 0);
#endif
    modem_interface_register_handler(&data_handler, SERIAL_MESSAGE_TYPE_ALP_DATA);

#ifdef APP_MODEMINTERFACE_TEST_SEND_DATA
    init_data(data1, DATA1_SIZE);
    init_data(data2, DATA2_SIZE);
    init_data(data3, DATA3_SIZE);
    sched_register_task(&send_data);
    sched_register_task((void (*)(void*)) & data_handler);
    timer_post_task_delay(&send_data, 5 * TIMER_TICKS_PER_SEC);
#endif

#ifdef APP_MODEMINTERFACE_TEST_SEND_PING
    modem_interface_register_handler(&ping_handler, SERIAL_MESSAGE_TYPE_PING_RESPONSE);
    sched_register_task(&send_ping);
    sched_register_task((void (*)(void*)) & ping_handler);
    timer_post_task_delay(&send_ping, 5 * TIMER_TICKS_PER_SEC);
#endif
}

//Stubs to get the application compiled without a bunch of external dependencies
uint8_t d7ap_volatile_files_data[FRAMEWORK_FS_VOLATILE_STORAGE_SIZE];