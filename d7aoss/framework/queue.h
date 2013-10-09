/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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

typedef struct
{
	uint16_t size;
	uint8_t length;
	uint8_t* front;
	uint8_t* rear;
	uint8_t* start;
} queue;


void queue_init(queue* q, uint8_t* buffer, uint16_t size);
uint8_t queue_pop_u8(queue* q); // TODO remove?
uint16_t queue_pop_u16(queue* q); // TODO remove?
void* queue_pop_value(queue* q, uint8_t size);
uint8_t queue_read_u8(queue* q, uint8_t position); // TODO remove?
uint16_t queue_read_u16(queue* q, uint8_t position); // TODO remove?
void* queue_read_value(queue* q, uint8_t position, uint8_t size);
bool queue_push_u8(queue* q, uint8_t data); // TODO remove?
bool queue_push_u16(queue* q, uint16_t data); // TODO remove?
bool queue_push_value(queue* q, void* data, uint8_t size);
void queue_set_u8(queue* q, uint8_t data, uint8_t position); // TODO remove?
void queue_set_u16(queue* q, uint16_t data, uint8_t position); // TODO remove?
void queue_insert_u8(queue* q, uint8_t data, uint8_t position); // TODO remove?
void queue_insert_u16(queue* q, uint16_t data, uint8_t position); // TODO remove?
void queue_insert_value(queue* q, void* data, uint8_t position, uint8_t size);
bool queue_is_empty(queue* q);

#endif /* QUEUE_H_ */
