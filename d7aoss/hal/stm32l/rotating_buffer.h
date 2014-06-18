/*
 * rotating_buffer.h
 *
 *  Created on: Jan 30, 2014
 *      Author: armin
 */

#ifndef ROTATING_BUFFER_H_
#define ROTATING_BUFFER_H_

#define ROTATING_BUFFER_SIZE 200
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	volatile unsigned int pos;
	volatile unsigned int top;
	uint8_t data[ROTATING_BUFFER_SIZE];
} rotating_buffer;


bool rotating_buffer_store_char(unsigned char c, rotating_buffer *buffer);
uint8_t rotating_buffer_read_char(rotating_buffer *buffer);
static inline bool rotating_buffer_is_empty(rotating_buffer *buffer) {
	return buffer->pos == buffer->top;
}

#endif /* ROTATING_BUFFER_H_ */
