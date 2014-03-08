/*
 * rotating_buffer.c
 *
 *  Created on: Jan 30, 2014
 *      Author: armin
 */

#include "rotating_buffer.h"

bool rotating_buffer_store_char(unsigned char c, rotating_buffer *buffer) {
	unsigned int i = (unsigned int) (buffer->pos + 1) % ROTATING_BUFFER_SIZE;
	bool canAdd = (i != buffer->top);

	if (canAdd) {
		buffer->data[buffer->pos] = c;
		buffer->pos = i;
	}
	return canAdd;
}

uint8_t rotating_buffer_read_char(rotating_buffer *buffer) {
	uint8_t c;
	if (!rotating_buffer_is_empty(buffer)) {
		c = buffer->data[buffer->top];
		buffer->top = (unsigned int) (buffer->top + 1) % ROTATING_BUFFER_SIZE;
	}
	else {
		c = 0;
	}
	return c;
}
