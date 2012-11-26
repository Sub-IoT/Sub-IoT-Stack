/*
 * queue.c
 *
 *  Created on: 25-nov.-2012
 *      Author: Maarten Weyn
 */

#include "queue.h"

static void shift_queue (queue* q, u8 places)
{
	if (q->front - places < q->start)
	{
		// cannot move before start
		return;
	}

	u8* data_ptr = q->front;
	while (data_ptr <= q->rear)
	{
		*(data_ptr-places) = *(data_ptr++);
	}

	q->front-= places;
	q->rear -= places;
}

static void check_for_space(queue* q, u8 size)
{
	if (q->rear + 1 > q->start + q->size)
	{
		// no place at the end anymore!

		if (q->front > q->start)
			shift_queue(q, q->start - q->front);
	}
}

void queue_init(queue* q, u8* buffer, u16 size)
{
	q->size = size;
	q->front = NULL;
	q->start = buffer;
	q->rear = NULL;
	q->length = 0;
}

u8 queue_pop_u8(queue* q)
{
	if (q->front == NULL)
		return 0;

	q->length--;

	return (*q->front++);
}

u16 queue_pop_u16(queue* q)
{
	if (q->front == NULL)
			return 0;

	q->length--;
	u16 value = *(u16*)(q->front);
	q->front += 2;
	return value;
}

void* queue_pop_value(queue* q, u8 size)
{
	if (q->front == NULL)
		return 0;

	q->length--;
	u8* value = q->front;
	q->front += size;
	return (void*) value;
}

u8 queue_read_u8(queue* q, u8 position)
{
	if (q->front + position > q->rear)
		return 0;

	return *(q->front + position);
}

u16 queue_read_u16(queue* q, u8 position)
{
	u16* value = (u16*) q->front;
	if (value + position > (u16*)q->rear)
			return 0;

		return *(value + position);
}

void queue_push_u8(queue* q, u8 data)
{
	queue_push_value(q, &data, 1);
}

void queue_push_u16(queue* q, u16 data)
{
	queue_push_value(q, &data, 2);
}

void queue_push_value(queue* q, void* data, u8 size)
{
	if (q->front == NULL)
	{
		q->front = q->start;
		q->rear = q->start;
	} else
	{
		q->rear += size;
	}

	memcpy(q->rear, data, size);
	q->length++;
}

void queue_insert_u8(queue* q, u8 data, u8 position)
{
	check_for_space(q,1);

	u8* position_ptr = q->front + position;
	u8* data_ptr = q->rear++;
	while (data_ptr >= position_ptr)
	{
		*(data_ptr+1) = *(data_ptr--);
	}

	*position_ptr = data;
	q->length++;
}

void queue_insert_u16(queue* q, u16 data, u8 position)
{
	check_for_space(q,2);

	u16* position_ptr = (u16*)(q->front) + position;
	u16* data_ptr = (u16*)(q->rear);
	q->rear += 2;
	while (data_ptr >= position_ptr)
	{
		u16* new_data_ptr = data_ptr + 1;
		*(new_data_ptr) = *(data_ptr--);
	}

	*position_ptr = data;
	q->length++;
}


void queue_set_u8(queue* q, u8 data, u8 position)
{
	u8* data_ptr = (u8*)(q->front) + position;
	*data_ptr = data;
}
void queue_set_u16(queue* q, u16 data, u8 position)
{
	u16* data_ptr = (u16*)(q->front) + position;
	*data_ptr = data;
}

bool queue_is_empty(queue* q)
{
	return q->front == NULL;
}
