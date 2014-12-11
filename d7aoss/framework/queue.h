/*! \file queue.h
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

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdbool.h>

#include "../types.h"


/** @struct Queue_t
 *  @brief A queue holds a data array but dynamically can change front and rear
 *  @var queue_t::size
 *  Field 'size' contains the size in bytes of the allocated memory.
 *  @var queue_t::length'
 *  Field 'length' contains the number of elements which are used in the queue.
 *  @var queue_t::front
 *  Field 'front' is  pointer to the first element.
 *  @var queue_t::rear
 *  Field 'rear' is a pointer to the last element.
 *  @var queue_t::start
 *  Field 'start' is a pointer to allocated memory
 *  @var queue_t::element_size
 *  Field 'element_size' is the size in bytes of one element
 **/
typedef struct
{
	uint16_t size;
	uint16_t length;
	uint8_t* front;
	uint8_t* rear;
	uint8_t* start;
	uint8_t element_size;
	uint8_t reserve_header;
} queue_t;


void queue_init(queue_t* q, uint8_t* buffer, uint16_t buffer_size, uint8_t element_size);
void queue_init_with_header(queue_t* q, uint8_t* buffer, uint16_t size, uint8_t element_size, uint8_t header_space);
uint8_t queue_pop_u8(queue_t* q);
void* queue_pop_value(queue_t* q);
uint8_t queue_read_u8(queue_t* q, uint8_t position);
void* queue_read_value(queue_t* q, uint8_t position);
bool queue_push_u8(queue_t* q, uint8_t data);
bool queue_push_u8_array(queue_t* q, uint8_t* data, uint8_t length);
bool queue_push_value(queue_t* q, void* data);
void queue_set_u8(queue_t* q, uint8_t data, uint8_t position);
void queue_insert_u8(queue_t* q, uint8_t data, uint8_t position);
void queue_insert_value(queue_t* q, void* data, uint8_t position);
bool queue_is_empty(queue_t* q);
void queue_clear(queue_t* q);

bool queue_create_header_space(queue_t* q, uint8_t required_header_space);
bool queue_create_space(queue_t* q, uint8_t required_space);

#endif /* QUEUE_H_ */
