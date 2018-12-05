#ifndef MODEM_INTERFACE_H
#define MODEM_INTERFACE_H

#include "fifo.h"
#include "hwuart.h"
#include "hwgpio.h"

#define SERIAL_FRAME_SYNC_BYTE 0xC0
#define SERIAL_FRAME_VERSION   0x00
#define SERIAL_FRAME_HEADER_SIZE 7
#define SERIAL_FRAME_SIZE 4
#define SERIAL_FRAME_COUNTER 2
#define SERIAL_FRAME_TYPE 3
#define SERIAL_FRAME_CRC1   5
#define SERIAL_FRAME_CRC2   6

typedef enum
{
    ALP_DATA=0X01,
    PING_REQUEST=0X02,
    PING_RESPONSE=0X03,
    LOGGING=0X04
} serial_message_type_t;

typedef void (*cmd_handler_t)(fifo_t* cmd_fifo);

/*
---------------HEADER(bytes)---------------------
|sync|sync|counter|message type|length|crc1|crc2|
-------------------------------------------------
*/

/** @brief Initialize the modem interface by registering
 *  tasks, initialising fifos/UART and registering callbacks/interrupts
 *  @param idx The UART port id.
 *  @param baudrate The used baud rate for UART communication
 *  @param mcu2modem The GPIO pin id of interrupt line indication request transmission/ready to receive
 *  @param modem2mcu The GPIO pin id of interrupt line indication request transmission/ready to receive
 *  @return Void.
 */
void modem_interface_init(uint8_t idx, uint32_t baudrate, pin_id_t mcu2modem, pin_id_t modem2mcu);

/** @brief  Adds header to bytes containing sync bytes, counter, length and crc and puts it in UART fifo
 *  @param bytes Bytes that need to be transmitted
 *  @param length Length of bytes
 *  @param type type of message (ALP, PING_REQUEST, LOGGING, ...)
 *  @return Void.
 */
void modem_interface_print_bytes(uint8_t* bytes, uint8_t length, serial_message_type_t type);
/** @brief Transmits a string by adding a header and putting it in the UART fifo
 *  @param string Bytes that need to be transmitted
 *  @return Void.
 */
void modem_interface_print(char* string);
/** @brief Registers callback to process a certain type op received UART data
 *  @param cmd_handler Pointer to function that processes the data
 *  @param type The type of data that needs to be processed by the given callback function
 *  @return Void.
 */
void modem_interface_register_handler(cmd_handler_t cmd_handler, uint8_t type);

#endif //MODEM_INTERFACE_H