/*
 *  Created on: May 9, 2013
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */
#include <stdbool.h>

#include "stm32l1xx.h"
#include "stm32l1xx_usart.h"
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_gpio.h"
#include "misc.h"


/* USARTx configured as follow:
 - BaudRate defined below
 - Word Length = 8 Bits
 - One Stop Bit
 - No parity
 - Hardware flow control disabled (RTS and CTS signals)
 - Receive and transmit enabled
 */
/* Which usart to use: */
#define PRINTF_USART_NUMBER			1
#define PRINTF_USART_PCLK			APB2
#define PRINTF_USART_BAUDRATE		115200
#define PRINTF_USART_GPIO_PORT		A
#define PRINTF_USART_TX_PIN			9
#define PRINTF_USART_RX_PIN			10
#define PRINTF_SERIAL_BUFFER_SIZE	200

#define GLUE(a, b) a##b
#define GLUE2(x,y)		GLUE(x, y)
#define GPIO_RCC(x)	 GLUE(RCC_AHBPeriph_GPIO, x)
#define GPIO(x) GLUE(GPIO, x)
#define GPIO_PIN(x) GLUE(GPIO_Pin_, x)
#define GPIO_PINSOURCE(x) GLUE(GPIO_PinSource, x)
#define GPIO_AF(x) GLUE(GPIO_AF_USART, x)
#define USART(x) GLUE(USART, x)
#define PCLK_REG(x) GLUE(x, ENR)

#define USART_IRQ_HANDLER(x) USART(GLUE(x, _IRQHandler))
#define PRINTF_USART_IRQ_HANDLER USART_IRQ_HANDLER(PRINTF_USART_NUMBER)

#define USART_IRQ_N(x) USART(GLUE(x, _IRQn))

#define RCC_PERIPH1(x)	GLUE(RCC_, x)
#define RCC_PERIPH2(x)	GLUE(Periph_USART, x)

#define PRINTF_RCC_PERIPH	GLUE2(RCC_PERIPH1(PRINTF_USART_PCLK), RCC_PERIPH2(PRINTF_USART_NUMBER))

#define PRINTF_PCLK_REG PCLK_REG(PRINTF_USART_PCLK)
#define PRINTF_USART USART(PRINTF_USART_NUMBER)


typedef struct {
	volatile unsigned int pos;
	volatile unsigned int top;
	uint8_t data[PRINTF_SERIAL_BUFFER_SIZE];
} buffer;

static buffer serial_tx = { .pos = 0, .top = 0 };
static buffer serial_rx = { .pos = 0, .top = 0 };

void uart_disable_interrupt() {
	NVIC ->ICER[USART_IRQ_N(PRINTF_USART_NUMBER) >> 0x05] =
			(uint32_t) 0x01 << (USART_IRQ_N(PRINTF_USART_NUMBER) & (uint8_t) 0x1F);
}

void uart_enable_interrupt() {
	NVIC ->ISER[USART_IRQ_N(PRINTF_USART_NUMBER) >> 0x05] =
			(uint32_t) 0x01 << (USART_IRQ_N(PRINTF_USART_NUMBER) & (uint8_t) 0x1F);
}

static inline bool store_char(unsigned char c, buffer *buffer) {
	uart_disable_interrupt();
	int i = (unsigned int) (buffer->pos + 1) % PRINTF_SERIAL_BUFFER_SIZE;
	bool canAdd = i != buffer->top;

	if (canAdd) {
		buffer->data[buffer->pos] = c;
		buffer->pos = i;
	}
	uart_enable_interrupt();
	return canAdd;
}

void uart_init()
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable UART & nGPIO clock */
	RCC ->PRINTF_PCLK_REG |= PRINTF_RCC_PERIPH;
	RCC ->AHBENR |= GPIO_RCC(PRINTF_USART_GPIO_PORT);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN(PRINTF_USART_TX_PIN);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(GPIO(PRINTF_USART_GPIO_PORT), &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN(PRINTF_USART_RX_PIN);
	GPIO_Init(GPIO(PRINTF_USART_GPIO_PORT), &GPIO_InitStructure);

	/* Connect USART TX and RX */
	GPIO_PinAFConfig(GPIO(PRINTF_USART_GPIO_PORT), GPIO_PINSOURCE(PRINTF_USART_TX_PIN), GPIO_AF(PRINTF_USART_NUMBER) );
	GPIO_PinAFConfig(GPIO(PRINTF_USART_GPIO_PORT), GPIO_PINSOURCE(PRINTF_USART_RX_PIN), GPIO_AF(PRINTF_USART_NUMBER) );

	/* USART configuration */
	/* USARTx configured as follow:
	 - Word Length = 8 Bits
	 - One Stop Bit
	 - No parity
	 - Hardware flow control disabled (RTS and CTS signals)
	 - Receive and transmit enabled
	 */
	USART_InitStructure.USART_BaudRate = PRINTF_USART_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(PRINTF_USART, &USART_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure the Priority Group to 2 bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2 );

	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART_IRQ_N(PRINTF_USART_NUMBER);
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xf;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0xf;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable USART */
	USART_Cmd(PRINTF_USART, ENABLE);
	/* Enable RXNE interrupt */
	PRINTF_USART ->CR1 |= USART_CR1_RXNEIE;

}


void uart_transmit_data(unsigned char data)
{
	// Enable transmit buffer empty interrupt
	PRINTF_USART ->CR1 |= USART_CR1_TXEIE;
	// Wait until buffer has space
	while(!store_char(data, &serial_tx)) {
	}

}

void uart_transmit_message(unsigned char *data, unsigned char length)
{
    unsigned char i=0;
    for (; i<length; i++)
    {
        uart_transmit_data(data[i]);
    }

}


unsigned char uart_tx_ready()
{
	return (serial_rx.pos != serial_rx.top);
}

unsigned char uart_receive_data()
{
	uint8_t c;
	uart_disable_interrupt();
	if (uart_tx_ready()) {
		c = serial_rx.data[serial_rx.top];
		serial_rx.top = (unsigned int) (serial_rx.top + 1) % PRINTF_SERIAL_BUFFER_SIZE;
	}
	else {
		c = 0;
	}
	uart_enable_interrupt();
	return c;
}



void PRINTF_USART_IRQ_HANDLER(void) {
	if (PRINTF_USART ->SR & USART_SR_TXE ) {
		if (serial_tx.pos == serial_tx.top) {
			// Disable transmit buffer empty interrupt
			PRINTF_USART ->CR1 &= ~USART_CR1_TXEIE;
		}
		else {
			// send one byte
			PRINTF_USART ->DR = serial_tx.data[serial_tx.top];
			serial_tx.top = (unsigned int) (serial_tx.top + 1) % PRINTF_SERIAL_BUFFER_SIZE;
		}
	}
	if (PRINTF_USART ->SR & USART_SR_RXNE ) {
		if (!(PRINTF_USART ->SR & USART_SR_PE )) {
			unsigned char c = PRINTF_USART ->DR;
			store_char(c, &serial_rx);
		} else {
			unsigned char c = PRINTF_USART ->DR;
		};
	}
}
