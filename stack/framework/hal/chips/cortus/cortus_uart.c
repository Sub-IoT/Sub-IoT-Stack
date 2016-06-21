
#include "hwuart.h"


bool uart_enable(uart_handle_t* uart) {

}

bool uart_disable(uart_handle_t* uart) {

}

void uart_set_rx_interrupt_callback(uart_handle_t* uart,
                                    uart_rx_inthandler_t rx_handler)
{

}

void uart_send_byte(uart_handle_t* uart, uint8_t data) {

}

void uart_send_bytes(uart_handle_t* uart, void const *data, size_t length) {

}

void uart_send_string(uart_handle_t* uart, const char *string) {

}

error_t uart_rx_interrupt_enable(uart_handle_t* uart) {

}

void uart_rx_interrupt_disable(uart_handle_t* uart) {

}
