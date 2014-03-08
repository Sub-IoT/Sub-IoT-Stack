/*! \file queue.c
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *
 */

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "queue.h"

static void shift_queue (queue_t* q, uint8_t places)
{
	if (q->front - places < q->start)
	{
		// cannot move before start
		return;
	}

	uint8_t* data_ptr = q->front;
	while (data_ptr <= q->rear)
	{
		*(data_ptr-places) = *(data_ptr++);
	}

	q->front-= places;
	q->rear -= places;
}

static bool check_for_space(queue_t* q, uint8_t size)
{
    if (q->rear + 1 >= q->start + q->size)
	{
		// no place at the end anymore!

		if (q->front > q->start)
			shift_queue(q, q->start - q->front);
		else
			return 0;
	}

	return 1;
}

void queue_init(queue_t* q, uint8_t* buffer, uint16_t size)
{
	q->size = size;
	q->front = NULL;
	q->start = buffer;
	q->rear = NULL;
	q->length = 0;
}

uint8_t queue_pop_u8(queue_t* q)
{
	if (q->front == NULL)
		return 0;

	q->length--;
	uint8_t value = *(uint16_t*)(q->front);
	if (q->length == 0)
	{
		q->front = NULL;
		q->rear = NULL;
	} else {
		q->front += 1;
	}
	return value;
}

uint16_t queue_pop_u16(queue_t* q)
{
	if (q->front == NULL)
			return 0;

	q->length--;
	uint16_t value = *(uint16_t*)(q->front);
	if (q->length == 0)
	{
		q->front = NULL;
		q->rear = NULL;
	} else {
		q->front += 2;
	}
	return value;
}

void* queue_pop_value(queue_t* q, uint8_t size)
{
	if (q->front == NULL)
		return 0;

	q->length--;
	uint8_t* value = q->front;
	if (q->length == 0)
	{
		q->front = NULL;
		q->rear = NULL;
	} else {
		q->front += size;
	}
	return (void*) value;
}

uint8_t queue_read_u8(queue_t* q, uint8_t position)
{
	if (q->front + position > q->rear)
		return 0;

	return *(q->front + position);
}

uint16_t queue_read_u16(queue_t* q, uint8_t position)
{
	uint16_t* value = (uint16_t*) q->front;
	if (value + position > (uint16_t*)q->rear)
			return 0;

		return *(value + position);
}

void* queue_read_value(queue_t* q, uint8_t position, uint8_t size)
{
	if (q->front + (position * size) > q->rear)
			return 0;

	return (void*) (q->front + (position * size));
}

bool queue_push_u8(queue_t* q, uint8_t data)
{
	return queue_push_value(q, &data, 1);
}

bool queue_push_u16(queue_t* q, uint16_t data)
{
	return queue_push_value(q, &data, 2);
}

bool queue_push_value(queue_t* q, void* data, uint8_t size)
{
	//log_print_stack_string(LOG_FWK, "push");
	if (q->front == NULL)
	{
		q->front = q->start;
		q->rear = q->start;
    }
    else
	{
		if (!check_for_space(q,size))
            return false;

		q->rear += size;
	}

	memcpy(q->rear, data, size);
	q->length++;

    return true;
}

void queue_insert_u8(queue_t* q, uint8_t data, uint8_t position)
{
	check_for_space(q,1);

	uint8_t* position_ptr = q->front + position;
	uint8_t* data_ptr = q->rear++;
	while (data_ptr >= position_ptr)
	{
		*(data_ptr+1) = *(data_ptr--);
	}

	*position_ptr = data;
	q->length++;
}

void queue_insert_u16(queue_t* q, uint16_t data, uint8_t position)
{
	check_for_space(q,2);

	uint16_t* position_ptr = (uint16_t*)(q->front) + position;
	uint16_t* data_ptr = (uint16_t*)(q->rear);
	q->rear += 2;
	while (data_ptr >= position_ptr)
	{
		uint16_t* new_data_ptr = data_ptr + 1;
		*(new_data_ptr) = *(data_ptr--);
	}

	*position_ptr = data;
	q->length++;
}

void queue_insert_value(queue_t* q, void* data, uint8_t position, uint8_t size)
{
	check_for_space(q,size);

	uint8_t* position_ptr = q->front + (position * size);
	uint8_t* data_ptr = q->rear;
	q->rear += size;
	while (data_ptr >= position_ptr)
	{
		uint8_t* new_data_ptr = data_ptr + size;
		memcpy(new_data_ptr, data_ptr, size);
		data_ptr -= size;
	}

	memcpy(position_ptr, data, size);
	q->length++;
}


void queue_set_u8(queue_t* q, uint8_t data, uint8_t position)
{
	uint8_t* data_ptr = (uint8_t*)(q->front) + position;
	*data_ptr = data;
}
void queue_set_u16(queue_t* q, uint16_t data, uint8_t position)
{
	uint16_t* data_ptr = (uint16_t*)(q->front) + position;
	*data_ptr = data;
}

bool queue_is_empty(queue_t* q)
{
	return q->front == NULL;
}
