#ifndef MODEM_INTERFACE_H
#define MODEM_INTERFACE_H

#include "fifo.h"
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

typedef void (*cmd_handler_t)(fifo_t* cmd_fifo);
typedef void (*target_rebooted_callback_t)(system_reboot_reason_t reboot_reason);

/*
---------------HEADER(bytes)---------------------
|sync|sync|counter|message type|length|crc1|crc2|
-------------------------------------------------
*/

/** @brief Initialize the modem interface by registering
 *  tasks, initialising fifos/UART and registering callbacks/interrupts
 *  @param idx The UART port id.
 *  @param baudrate The used baud rate for UART communication
 *  @param uart_state_pin_id The GPIO pin used to signal to the target we are ready for transmit or receive
 *  @param target_uart_state_pin_id The GPIO pin used by the target to signal that the target is ready for transmit or receive
 *  @return Void.
 */
void modem_interface_init(uint8_t idx, uint32_t baudrate, pin_id_t uart_state_pin_id, pin_id_t target_uart_state_pin_id);

/** @brief  Adds header to bytes containing sync bytes, counter, length and crc and puts it in UART fifo
 *  @param bytes Bytes that need to be transmitted
 *  @param length Length of bytes
 *  @param type type of message (SERIAL_MESSAGE_TYPE_ALP, SERIAL_MESSAGE_TYPE_PING_REQUEST, SERIAL_MESSAGE_TYPE_LOGGING, ...)
 *  @return Void.
 */
void modem_interface_transfer_bytes(uint8_t* bytes, uint8_t length, serial_message_type_t type);
/** @brief Transmits a string by adding a header and putting it in the UART fifo
 *  @param string Bytes that need to be transmitted
 *  @return Void.
 */
void modem_interface_transfer(char* string);
/** @brief Registers callback to process a certain type of received UART data
 *  @param cmd_handler Pointer to function that processes the data
 *  @param type The type of data that needs to be processed by the given callback function
 *  @return Void.
 */
void modem_interface_register_handler(cmd_handler_t cmd_handler, serial_message_type_t type);

/** @brief Registers callback to be executed when the remote target reboots
 *  @param cb Callback function pointer
 */
void modem_interface_set_target_rebooted_callback(target_rebooted_callback_t cb);

#endif //MODEM_INTERFACE_H
