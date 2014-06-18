/*
 * stm32l_virtual_uart.c
 *
 *  Created on: Jan 30, 2014
 *      Author: armin
 */

#include "uart.h"

#include "hw_config.h"
#include "rotating_buffer.h"
#include "usb_init.h"

rotating_buffer usb_rx;

void uart_init() {
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();

    USB_Init();

}

void uart_enable_interrupt() {
    NVIC->ISER[0] = (uint32_t) 0x01 << (USB_LP_IRQn & (uint8_t) 0x1F);
}

void uart_disable_interrupt() {
    NVIC->ICER[0] = (uint32_t) 0x01 << (USB_LP_IRQn & (uint8_t) 0x1F);
}

void uart_transmit_data(unsigned char data) {
    USB_Send_Character(data);

}
void uart_transmit_message(unsigned char *data, unsigned char length) {
    USB_Send_Data(data, length);
}

unsigned char uart_tx_ready() {
	// FIXME How is uart_tx_ready defined??
	return 1;
}

unsigned char uart_receive_data() {
    return rotating_buffer_read_char(&usb_rx);
}

