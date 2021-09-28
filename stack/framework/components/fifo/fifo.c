/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
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
#include "debug.h"

void fifo_init(fifo_t *fifo, uint8_t *buffer, uint16_t max_size)
{
    fifo_init_filled(fifo, buffer, 0, max_size);
}

void fifo_init_filled(fifo_t *fifo, uint8_t *buffer, uint16_t filled_size, uint16_t max_size)
{
    fifo->buffer = buffer;
    fifo->head_idx = 0;
    fifo->max_size = max_size;
    fifo->tail_idx = filled_size == 0 ? 0 : filled_size;
    fifo->tail_idx = filled_size == max_size ? 0 : filled_size;
    fifo->is_full = (filled_size == max_size);
    fifo->is_subview = false;    
}

error_t fifo_init_subview(fifo_t *subset_fifo, fifo_t* original_fifo, uint16_t offset, uint16_t subset_size) {
    if(offset + subset_size > fifo_get_size(original_fifo))
        return ESIZE;

    subset_fifo->buffer = original_fifo->buffer;
    subset_fifo->head_idx = (original_fifo->head_idx + offset) % original_fifo->max_size;
    subset_fifo->tail_idx = (original_fifo->head_idx + offset + subset_size) % original_fifo->max_size;
    subset_fifo->max_size = original_fifo->max_size;
    subset_fifo->is_full = (subset_size == subset_fifo->max_size);
    subset_fifo->is_subview = true;
    return SUCCESS;
}

error_t fifo_put(fifo_t *fifo, uint8_t *data, uint16_t len)
{
    if(fifo->is_subview)
        return EINVAL;
    
    if(fifo->is_full)
        return ESIZE;

    if(fifo->tail_idx < fifo->head_idx)
    {
        if(fifo->tail_idx + len > fifo->head_idx)
            return ESIZE;
        memcpy(fifo->buffer + fifo->tail_idx, data, len);
        fifo->tail_idx += len;
        fifo->is_full = (fifo->tail_idx == fifo->head_idx);
        return SUCCESS;
    }

    if(fifo->tail_idx + len < fifo->max_size)
    {
        memcpy(fifo->buffer + fifo->tail_idx, data, len);
        fifo->tail_idx += len;
        return SUCCESS;
    }

    uint16_t space_left_before_max_size = fifo->max_size - fifo->tail_idx;
    uint16_t space_needed_after_wrap = len - space_left_before_max_size;
    if(fifo->head_idx >= space_needed_after_wrap)
    {
        // wrap
        memcpy(fifo->buffer + fifo->tail_idx, data, space_left_before_max_size);
        memcpy(fifo->buffer, data + space_left_before_max_size, space_needed_after_wrap);
        fifo->tail_idx = space_needed_after_wrap;
        fifo->is_full = (fifo->tail_idx == fifo->head_idx);
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
  if(fifo->head_idx >= fifo->max_size)
  {
    fifo->head_idx = fifo->head_idx % fifo->max_size;
  }

  if(len > 0)
    fifo->is_full = 0;
}

error_t fifo_skip(fifo_t* fifo, uint16_t len) {
  error_t err = check_len(fifo, len);
  if(err != SUCCESS)
    return err;

  skip(fifo, len);

  return SUCCESS;
}

error_t fifo_remove(fifo_t *fifo, uint16_t len)
{
    if(check_len(fifo, len) != SUCCESS)
        return ESIZE;
        
    if(fifo->tail_idx > fifo->head_idx)
    {
        if((fifo->tail_idx - len) < fifo->head_idx)
            return ESIZE;
        fifo->tail_idx -= len;
        fifo->is_full = (len == 0);
        return SUCCESS;
    }

    if(fifo->tail_idx - len > 0)
    {
        fifo->tail_idx -= len;
        fifo->is_full = (len == 0);
        return SUCCESS;
    }

    uint16_t space_left_before_0 = fifo->tail_idx;
    uint16_t space_freed_after_wrap = len - space_left_before_0;
    if(fifo->max_size - fifo->head_idx > space_freed_after_wrap)
    {
        // wrap
        fifo->tail_idx = fifo->max_size - space_freed_after_wrap;
        fifo->is_full = (len == 0);
        return SUCCESS;
    }
    else
        return ESIZE;
}

error_t fifo_remove_last_byte(fifo_t* fifo) {
    return fifo_remove(fifo, 1);
}

error_t fifo_peek(fifo_t* fifo, uint8_t* buffer, uint16_t offset, uint16_t len) {
  error_t err = check_len(fifo, offset + len);
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
    if(fifo->head_idx == fifo->tail_idx)
        return fifo->is_full ? fifo->max_size : 0;
    else if(fifo->head_idx < fifo->tail_idx)
        return fifo->tail_idx - fifo->head_idx;
    else
        return fifo->tail_idx + (fifo->max_size - fifo->head_idx);
}

void fifo_get_continuos_raw_data(fifo_t* fifo, uint8_t** pdata, uint16_t* plen)
{
    *pdata = fifo->buffer + fifo->head_idx;
    if(fifo->is_full || (fifo->tail_idx < fifo->head_idx))
    {
        *plen = fifo->max_size - fifo->head_idx;
    }
    else
    {
        *plen = fifo->tail_idx - fifo->head_idx;
    }
}

void fifo_clear(fifo_t* fifo)
{
    fifo->head_idx = 0;
    fifo->tail_idx = 0;
    fifo->is_full = false;
}

bool fifo_is_full(fifo_t* fifo) {
    return fifo->is_full;
}
