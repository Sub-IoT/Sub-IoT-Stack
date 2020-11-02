
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include <string.h>
#include <termios.h>

#include "platform_defs.h"
#include "hwuart.h"

struct uart_handle
{
	int fd;
	char dev[256];
	volatile bool is_irq_enabled;
	volatile bool is_enabled;
	volatile uart_rx_inthandler_t rx_handler;
	uint8_t stack[8192];
};

static struct uart_handle dummy_uarts[] =
{
#ifdef PLATFORM_UART_DEV0
		{
				.dev = PLATFORM_UART_DEV0
		},
#endif
#ifdef PLATFORM_UART_DEV1
		{
				.dev = PLATFORM_UART_DEV1
		},
#endif
#ifdef PLATFORM_UART_DEV2
		{
				.dev = PLATFORM_UART_DEV2
		},
#endif
};

int uart_handler(void *arg)
{
	uart_handle_t *uart = (uart_handle_t *)arg;
	while (1)
	{
		char c;
		int ret;
		ret = read(uart->fd, &c, 1);
		if (ret == 1 && uart->is_enabled && uart->is_irq_enabled && uart->rx_handler)
		{
			//printf("\nRX byte(%s): %02x\n", uart->dev, (uint8_t)c);
			uart->rx_handler((uint8_t)c);
		}
	}

	return 0;
}


uart_handle_t* uart_init(uint8_t port_idx, uint32_t baudrate, uint8_t pins)
{
	uart_handle_t *uart = &dummy_uarts[port_idx];
	uart->fd = open(uart->dev, O_RDWR | O_NOCTTY);

	struct termios raw;
	tcgetattr(uart->fd, &raw);

	if (baudrate == 1200)
		baudrate = B1200;
	else if (baudrate == 2400)
		baudrate = B2400;
	else if (baudrate == 4800)
		baudrate = B4800;
	else if (baudrate == 9600)
		baudrate = B9600;
	else if (baudrate == 19200)
		baudrate = B19200;
	else if (baudrate == 38400)
		baudrate = B38400;
	else if (baudrate == 57600)
		baudrate = B57600;
	else if (baudrate == 57600)
		baudrate = B57600;
	else if (baudrate == 115200)
		baudrate = B115200;
	else
		assert(0);

	cfsetispeed(&raw, baudrate);
	cfsetospeed(&raw, baudrate);

	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON | CSTOPB);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	tcsetattr(uart->fd, TCSAFLUSH, &raw);

	clone(uart_handler, &uart->stack[8192], CLONE_VM, uart);

    return uart;
}

bool           uart_disable(uart_handle_t* uart)
{
	uart->is_enabled = false;
}

bool           uart_get_rx_port_state(uart_handle_t* uart)
{
	return false;
}

bool           uart_enable(uart_handle_t* uart)
{
	uart->is_enabled = true;
}

void           uart_send_byte(uart_handle_t* uart, uint8_t data)
{
	if (uart->is_enabled)
		write(uart->fd, &data, 1);
}

void           uart_send_bytes(uart_handle_t* uart, void const *data, size_t length)
{
	if (uart->is_enabled)
		write(uart->fd, data, length);
}

void           uart_send_string(uart_handle_t* uart, const char *string)
{
	if (uart->is_enabled)
		write(uart->fd, string, strlen(string));
}

error_t        uart_rx_interrupt_enable(uart_handle_t* uart)
{
	uart->is_irq_enabled = true;
	return 0;
}

void           uart_rx_interrupt_disable(uart_handle_t* uart)
{
	uart->is_irq_enabled = false;
}

void           uart_set_rx_interrupt_callback(uart_handle_t* uart,
                                                       uart_rx_inthandler_t rx_handler)
{
	uart->rx_handler = rx_handler;
}

void           cdc_set_rx_interrupt_callback(uart_rx_inthandler_t rx_handler)
{
}

void           uart_set_error_callback(uart_handle_t* uart, uart_error_handler_t error_handler)
{
}
