/*
 * queue.h
 *
 *  Created on: 25-nov.-2012
 *      Author: Maarten Weyn
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include "types.h"

typedef struct
{
	u16 size;
	u8 length;
	u8* front;
	u8* rear;
	u8* start;
} queue;


void queue_init(queue* q, u8* buffer, u16 size);
u8 queue_pop_u8(queue* q);
u16 queue_pop_u16(queue* q);
void* queue_pop_value(queue* q, u8 size);
u8 queue_read_u8(queue* q, u8 position);
u16 queue_read_u16(queue* q, u8 position);
bool queue_push_u8(queue* q, u8 data);
bool queue_push_u16(queue* q, u16 data);
bool queue_push_value(queue* q, void* data, u8 size);
void queue_set_u8(queue* q, u8 data, u8 position);
void queue_set_u16(queue* q, u16 data, u8 position);
void queue_insert_u8(queue* q, u8 data, u8 position);
void queue_insert_u16(queue* q, u16 data, u8 position);
bool queue_is_empty(queue* q);

#endif /* QUEUE_H_ */
