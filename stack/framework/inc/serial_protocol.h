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
#ifndef SERIAL_PROTOCOL_H
#define SERIAL_PROTOCOL_H

#include "types.h"
#include "fifo.h"
#include "framework_defs.h"
#include "hwuart.h"
#include "hwgpio.h"
#include "hwsystem.h"


typedef enum
{
    SERIAL_MESSAGE_TYPE_ALP_DATA=0X01,
    SERIAL_MESSAGE_TYPE_PING_REQUEST=0X02,
    SERIAL_MESSAGE_TYPE_PING_RESPONSE=0X03,
    SERIAL_MESSAGE_TYPE_LOGGING=0X04,
    SERIAL_MESSAGE_TYPE_REBOOTED=0X05,
} serial_message_type_t;

typedef struct serial_protocol_handle serial_protocol_handle_t;
typedef void (*cmd_handler_t)(serial_protocol_handle_t* handle, serial_message_type_t type, fifo_t* cmd_fifo);
typedef void (*target_rebooted_callback_t)(serial_protocol_handle_t* handle, system_reboot_reason_t reboot_reason);

/*
---------------HEADER(bytes)---------------------
|sync|sync|counter|message type|length|crc1|crc2|
-------------------------------------------------
*/

typedef struct
{
    /** @brief  Adds header to bytes containing sync bytes, counter, length and crc and puts it in UART fifo
     *  @param handle handle of the modem interface instance
     *  @param bytes Bytes that need to be transmitted
     *  @param length Length of bytes
     *  @param type type of message (SERIAL_MESSAGE_TYPE_ALP, SERIAL_MESSAGE_TYPE_PING_REQUEST, SERIAL_MESSAGE_TYPE_LOGGING, ...)
     *  @return Void.
     */
    error_t (*serial_protocol_transfer_bytes)(serial_protocol_handle_t* handle, uint8_t* bytes, uint8_t length, serial_message_type_t type);
    /** @brief Transmits a string by adding a header and putting it in the UART fifo
     *  @param handle handle of the modem interface instance
     *  @param string Bytes that need to be transmitted
     *  @return error_t 	SUCCESS if the data is put into the Tx buffer.
     *                      ENOMEM if the data doesn't fit in the Tx buffer.
     */
    error_t (*serial_protocol_transfer)(serial_protocol_handle_t* handle, char* string);
    /** @brief Registers callback to process a certain type of received UART data
     *  @param handle handle of the modem interface instance
     *  @param cmd_handler Pointer to function that processes the data
     *  @param type The type of data that needs to be processed by the given callback function
     *  @return error_t 	SUCCESS if the data is put into the Tx buffer.
     *                      ENOMEM if the data doesn't fit in the Tx buffer.
     */
    void (*serial_protocol_register_handler)(serial_protocol_handle_t* handle, cmd_handler_t cmd_handler, serial_message_type_t type);
    void (*serial_protocol_unregister_handler)(serial_protocol_handle_t* handle, serial_message_type_t type);
    /** @brief Registers callback to be executed when the remote target reboots
     *  @param handle handle of the modem interface instance
     *  @param cb Callback function pointer
     */
    void (*serial_protocol_set_target_rebooted_callback)(serial_protocol_handle_t* handle, target_rebooted_callback_t cb);
    void (*serial_protocol_clear_handler)(serial_protocol_handle_t* handle);
} serial_protocol_driver_t;

#if(__LONG_WIDTH__ == 64)
#define PRIV_BUF_BASE_SIZE (146 + (sizeof(void *)/4) + 9)
#elif(__LONG_WIDTH__ == 32)
#define PRIV_BUF_BASE_SIZE (146 + (sizeof(void *)/4))
#else
#error Unsupported word size
#endif
#ifdef FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_DMA
#define PRIV_BUF_SIZE (PRIV_BUF_BASE_SIZE + (2*sizeof(void *)/4 + 5))
#elif defined(FRAMEWORK_SERIAL_PROTOCOL_SUPPORT_INTERRUPT_LINES)
#define PRIV_BUF_SIZE (PRIV_BUF_BASE_SIZE + 4)
#else
#define PRIV_BUF_SIZE PRIV_BUF_BASE_SIZE
#endif

struct serial_protocol_handle
{
    serial_protocol_driver_t* driver;
    uint32_t priv_data[PRIV_BUF_SIZE];
};

/** @brief Initialize the modem interface by registering
 *  tasks, initialising fifos/UART and registering callbacks/interrupts
 *  @param handle handle of the modem interface instance
 *  @param idx The UART port id.
 *  @param baudrate The used baud rate for UART communication
 *  @param uart_state_pin_id The GPIO pin used to signal to the target we are ready for transmit or receive
 *  @param target_uart_state_pin_id The GPIO pin used by the target to signal that the target is ready for transmit or receive
 *  @return Void.
 */
void serial_protocol_init(serial_protocol_handle_t* handle, uint8_t idx, uint32_t baudrate,
                           bool use_interrupt_lines, pin_id_t uart_state_pin_id, pin_id_t target_uart_state_pin_id,
                           bool use_dma, uint8_t dma_rx_channel, uint8_t dma_tx_channel);

#endif //SERIAL_PROTOCOL_H
