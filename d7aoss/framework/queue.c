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

static bool shift_queue_left (queue_t* q, uint8_t places)
{
	if (q->front - places < q->start)
	{
		// cannot move before start
		return 0;
	}

	uint8_t* data_ptr = q->front;
	while (data_ptr <= q->rear)
	{
		*(data_ptr-places) = *(data_ptr++);
	}

	q->front-= places;
	q->rear -= places;

	return 1;
}

static bool shift_queue_right (queue_t* q, uint8_t places)
{
	if (q->rear + places > q->start + q->size)
	{
		// cannot move behind allocated place
		return 0;
	}

	uint8_t* data_ptr = q->rear;
	while (data_ptr >= q->front)
	{
		*(data_ptr+places) = *(data_ptr--);
	}

	q->front+= places;
	q->rear += places;

	return 1;
}

static bool check_for_space(queue_t* q, uint8_t nr_of_elements)
{
    if (q->rear + (q->element_size * nr_of_elements) >= q->start + q->size)
	{
		// no place at the end anymore!

		if (q->front > q->start)
			return shift_queue_left(q, q->start - q->front);
		else
			return 0;
	}

	return 1;
}

void queue_init(queue_t* q, uint8_t* buffer, uint16_t size, uint8_t element_size)
{
	q->size = size;
	q->front = NULL;
	q->start = buffer;
	q->rear = NULL;
	q->length = 0;
	q->element_size = element_size;
	q->reserve_header = 0;
}

void queue_init_with_header(queue_t* q, uint8_t* buffer, uint16_t size, uint8_t element_size, uint8_t header_space)
{
	queue_init(q,buffer, size, element_size);
	if (header_space * element_size < size)
		q->reserve_header = header_space;
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

void* queue_pop_value(queue_t* q)
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
		q->front += q->element_size;
	}
	return (void*) value;
}

uint8_t queue_read_u8(queue_t* q, uint8_t position)
{
	if (q->front + position > q->rear)
		return 0;

	return *(q->front + position);
}

void* queue_read_value(queue_t* q, uint8_t position)
{
	if (q->front + (position * q->element_size) > q->rear)
			return 0;

	return (void*) (q->front + (position * q->element_size));
}

bool queue_push_u8(queue_t* q, uint8_t data)
{
	if (q->front == NULL)
	{
		q->front = q->start + q->reserve_header;
		q->rear = q->front;
	}
	else
	{
		if (!check_for_space(q,1))
			return false;

		q->rear ++;
	}

	*q->rear = data;
	q->length++;

	return true;
}


bool queue_push_u8_array(queue_t* q, uint8_t* data, uint8_t length)
{
	if (q->front == NULL)
	{
		q->front = q->start + q->reserve_header;
		q->rear = q->front;
	}
	else
	{
		if (!check_for_space(q, length))
			return false;

		q->rear ++;
	}

	memcpy(q->rear, data, length);
	q->rear += length - 1;
	q->length += length;

	return true;
}


bool queue_push_value(queue_t* q, void* data)
{
	//log_print_stack_string(LOG_FWK, "push");
	if (q->front == NULL)
	{
		q->front = q->start + (q->reserve_header * q->element_size);
		q->rear = q->front;
    }
    else
	{
		if (!check_for_space(q,1))
            return false;

		q->rear += q->element_size;
	}

	memcpy(q->rear, data, q->element_size);
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

void queue_insert_value(queue_t* q, void* data, uint8_t position)
{
	check_for_space(q,1);

	uint8_t* position_ptr = q->front + (position * q->element_size);
	uint8_t* data_ptr = q->rear;
	q->rear += q->element_size;
	while (data_ptr >= position_ptr)
	{
		uint8_t* new_data_ptr = data_ptr + q->element_size;
		memcpy(new_data_ptr, data_ptr, q->element_size);
		data_ptr -= q->element_size;
	}

	memcpy(position_ptr, data, q->element_size);
	q->length++;
}


void queue_set_u8(queue_t* q, uint8_t data, uint8_t position)
{
	uint8_t* data_ptr = (uint8_t*)(q->front) + position;
	*data_ptr = data;
}

bool queue_is_empty(queue_t* q)
{
	return q->front == NULL;
}


void queue_clear(queue_t* q)
{
	q->front = NULL;
	q->rear = NULL;
	q->length = 0;
}

bool queue_create_header_space(queue_t* q, uint8_t required_header_space)
{
	if (q->front == NULL)
		return 0;

	if (q->front - required_header_space < q->start)
		if (!(shift_queue_right(q, required_header_space)))
			return 0;

	q->front -= required_header_space;
	q->length += required_header_space;

	return 1;
}

bool queue_create_space(queue_t* q, uint8_t required_space)
{
	if (q->front == NULL)
	{
		q->front = q->start;
		q->rear = q->start;
	}


	if (q->rear + required_space > q->start + q->size)
		if (!(shift_queue_left(q, required_space)))
			return 0;

	q->rear += required_space;
	q->length += required_space;

	return 1;
}

