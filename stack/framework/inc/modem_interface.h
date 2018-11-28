#ifndef MODEM_INTERFACE_H
#define MODEM_INTERFACE_H

//#define MODEM_INTERFACE_ENABLED 1
#include "fifo.h"
#include "hwuart.h"
#include "hwgpio.h"
//#ifdef MODEM_INTERFACE_ENABLED

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

void modem_interface_init(uint8_t idx, uint32_t baudrate, uint8_t pins, pin_id_t mcu2modem, pin_id_t modem2mcu);
void modem_interface_enable();
void modem_interface_disable();
void modem_interface_print_bytes(uint8_t* bytes, uint8_t length, serial_message_type_t type);
void modem_interface_print(char* string);
void modem_interface_rx_interrupt_enable();
void modem_interface_set_rx_interrupt_callback(uart_rx_inthandler_t uart_rx_cb);
void modem_interface_register_handler(cmd_handler_t cmd_handler, uint8_t type);

/*
#else

#define modem_interface_init(....)              ((void)0)
#define modem_interface_enable()            ((void)0)
#define shell_echo_disable()                ((void)0)
#define modem_interface_disable()           ((void)0)
#define modem_interface_print_bytes(...)    ((void)0)
#define modem_interface_print(...)          ((void)0)

#endif
*/

#endif //MODEM_INTERFACE_H