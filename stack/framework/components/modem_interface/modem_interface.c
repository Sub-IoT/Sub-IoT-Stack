/*
 * Copyright (c) 2015-2022 University of Antwerp, Aloxy NV.
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

#include "framework_defs.h"
#include "modem_interface.h"
#include "platform.h"
#include "platform_defs.h"
#include "serial_protocol.h"

static serial_protocol_handle_t sp_handle;
static modem_interface_cmd_handler_t cmd_handlers[SERIAL_MESSAGE_TYPE_REBOOTED + 1];
static modem_interface_target_rebooted_callback_t modem_interface_target_rebooted_callback;

static void ping_response_handler(serial_protocol_handle_t* handle, serial_message_type_t type, fifo_t* cmd_fifo)
{
#ifdef MODULE_ALP
    // free all alp commands
    alp_layer_free_commands();
#endif
}

void modem_interface_init(void)
{
#ifdef FRAMEWORK_MODEM_INTERFACE_USE_DMA
    serial_protocol_init(&sp_handle, PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, true, PLATFORM_MODEM_INTERFACE_UART_STATE_PIN, PLATFORM_MODEM_INTERFACE_TARGET_UART_STATE_PIN, true, PLATFORM_MODEM_INTERFACE_DMA_RX ,PLATFORM_MODEM_INTERFACE_DMA_TX);
#elif defined(FRAMEWORK_MODEM_INTERFACE_USE_INTERRUPT_LINES)
    serial_protocol_init(&sp_handle, PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, true, PLATFORM_MODEM_INTERFACE_UART_STATE_PIN, PLATFORM_MODEM_INTERFACE_TARGET_UART_STATE_PIN, false, 0 ,0);
#else
    serial_protocol_init(&sp_handle, PLATFORM_MODEM_INTERFACE_UART, PLATFORM_MODEM_INTERFACE_BAUDRATE, false, (pin_id_t) 0, (pin_id_t) 0, false, 0, 0);
#endif
    sp_handle.driver->serial_protocol_register_handler(&sp_handle, &ping_response_handler, SERIAL_MESSAGE_TYPE_PING_REQUEST);
}


error_t modem_interface_transfer_bytes(uint8_t* bytes, uint8_t length, serial_message_type_t type)
{
    return sp_handle.driver->serial_protocol_transfer_bytes(&sp_handle, bytes, length, type);
}

error_t modem_interface_transfer(char* string)
{
    return sp_handle.driver->serial_protocol_transfer(&sp_handle, string);
}

static void modem_interface_cmd_handler(serial_protocol_handle_t* handle, serial_message_type_t type, fifo_t* cmd_fifo)
{
    if(cmd_handlers[type] != NULL)
    {
        cmd_handlers[type](cmd_fifo);
    }
}

error_t modem_interface_register_handler(modem_interface_cmd_handler_t cmd_handler, serial_message_type_t type)
{
    if(cmd_handlers[type] != NULL)
    {
        return -EEXIST;
    }
    cmd_handlers[type] = cmd_handler;
    sp_handle.driver->serial_protocol_register_handler(&sp_handle, &modem_interface_cmd_handler, type);
    return SUCCESS;
}

void modem_interface_unregister_handler(serial_message_type_t type)
{
    cmd_handlers[type] = NULL;
    sp_handle.driver->serial_protocol_unregister_handler(&sp_handle, type);
}

static void target_rebooted_callback(serial_protocol_handle_t* handle, system_reboot_reason_t reboot_reason)
{
    if(modem_interface_target_rebooted_callback != NULL)
    {
        modem_interface_target_rebooted_callback(reboot_reason);
    }
}

error_t modem_interface_set_target_rebooted_callback(modem_interface_target_rebooted_callback_t cb)
{
    if(modem_interface_target_rebooted_callback != NULL)
    {
        return -EEXIST;
    }
    modem_interface_target_rebooted_callback = cb;
    sp_handle.driver->serial_protocol_set_target_rebooted_callback(&sp_handle, &target_rebooted_callback);
    return SUCCESS;
}

void modem_interface_clear_handler()
{
    sp_handle.driver->serial_protocol_clear_handler(&sp_handle);
}