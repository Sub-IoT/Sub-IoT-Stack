/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file fifo.c
 * \author glenn.ergeerts@uantwerpen.be
 */

#include "fifo.h"
#include "string.h"
#include "errors.h"

void fifo_init(fifo_t *fifo, uint8_t *buffer, uint16_t max_size)
{
    fifo_init_filled(fifo, buffer, 0, max_size);
}

void fifo_init_filled(fifo_t *fifo, uint8_t *buffer, uint16_t filled_size, uint16_t max_size)
{
    fifo->buffer = buffer;
    fifo->head_idx = 0;
    fifo->max_size = max_size - 1;
    fifo->tail_idx = filled_size;
}

error_t fifo_put(fifo_t *fifo, uint8_t *data, uint16_t len)
{
    if(fifo->tail_idx < fifo->head_idx)
    {
        if(fifo->tail_idx + len >= fifo->head_idx)
            return ESIZE;
        memcpy(fifo->buffer + fifo->tail_idx, data, len);
        fifo->tail_idx += len;
        return SUCCESS;
    }

    if(fifo->tail_idx + len <= fifo->max_size)
    {
        memcpy(fifo->buffer + fifo->tail_idx, data, len);
        fifo->tail_idx += len;
        return SUCCESS;
    }

    uint16_t space_left_before_max_size = fifo->max_size - fifo->tail_idx;
    uint16_t space_needed_after_wrap = len - space_left_before_max_size;
    if(fifo->head_idx > space_needed_after_wrap)
    {
        // wrap
        memcpy(fifo->buffer + fifo->tail_idx, data, space_left_before_max_size);
        memcpy(fifo->buffer, data + space_left_before_max_size, space_needed_after_wrap);
        fifo->tail_idx = space_needed_after_wrap;
        return SUCCESS;
    }
    else
        return ESIZE;
}

error_t fifo_put_byte(fifo_t* fifo, uint8_t byte)
{
    return fifo_put(fifo, &byte, 1);
}

static error_t check_len(fifo_t* fifo, uint16_t len) {
  // quickly bail out if requested length is zero, nothing to do
  if(len == 0) { return SUCCESS; }

  // quick check if requested len doesn't exceed available data
  if(len > fifo_get_size(fifo)) { return ESIZE; }

  return SUCCESS;
}

static void skip(fifo_t* fifo, uint16_t len) {
  // progress head to implement popping behaviour
  fifo->head_idx = (fifo->head_idx + len);
  if(fifo->head_idx > fifo->max_size)
    fifo->head_idx = fifo->head_idx % fifo->max_size; // when head_idx == max_size we do not want to point to 0 since size will not be correct then
}

error_t fifo_skip(fifo_t* fifo, uint16_t len) {
  error_t err = check_len(fifo, len);
  if(err != SUCCESS)
    return err;

  skip(fifo, len);

  return SUCCESS;
}

error_t fifo_peek(fifo_t* fifo, uint8_t* buffer, uint16_t offset, uint16_t len) {
  error_t err = check_len(fifo, len);
  if(err != SUCCESS)
    return err;

  // determine start/end index (in circular buffer)
  uint16_t start_idx = (fifo->head_idx + offset) % fifo->max_size;
  uint16_t end_idx   = (start_idx      + len   ) % fifo->max_size;

  // simple case: the end doesn't wrap...
  // .............
  //     S-len->E
  if(end_idx >= start_idx) {
    memcpy(buffer, fifo->buffer + start_idx, len);
    return SUCCESS;
  }

  // the end does wrap...
  // .............
  // ->E S--len-->
  //      <--p1-->
  uint16_t part1 = fifo->max_size - start_idx;
  // copy first part from start up to the end of the buffer
  memcpy(buffer,         fifo->buffer + start_idx, part1);
  // copy remaining (wrapped) bytes from start
  memcpy(buffer + part1, fifo->buffer,             len - part1);

  return SUCCESS;
}

error_t fifo_pop(fifo_t* fifo, uint8_t* buffer, uint16_t len) {
  // use peek logic to retrieve data
  error_t err = fifo_peek(fifo, buffer, 0, len);
  if( err != SUCCESS ) { return err; }

  skip(fifo, len);

  return SUCCESS;
}

uint16_t fifo_get_size(fifo_t* fifo)
{
    if(fifo->head_idx <= fifo->tail_idx)
        return fifo->tail_idx - fifo->head_idx;
    else
        return fifo->tail_idx + (fifo->max_size - fifo->head_idx);
}

void fifo_clear(fifo_t* fifo)
{
    fifo->head_idx = 0;
    fifo->tail_idx = 0;
}

bool fifo_is_full(fifo_t* fifo) {
    return fifo_get_size(fifo) == fifo->max_size;
}
