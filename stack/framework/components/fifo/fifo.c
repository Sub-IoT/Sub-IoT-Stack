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
    fifo->buffer = buffer;
    fifo->head_idx = 0;
    fifo->tail_idx = 0;
    fifo->max_size = max_size;
}

error_t fifo_put(fifo_t *fifo, uint8_t *data, uint16_t len)
{
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

error_t fifo_pop(fifo_t* fifo, uint8_t* buffer, uint16_t len)
{
    if(len > fifo_get_size(fifo)) return ESIZE;

    if(fifo->head_idx + len < fifo->max_size)
    {
        memcpy(buffer, fifo->buffer + fifo->head_idx, len);
        fifo->head_idx += len;
        return SUCCESS;
    }

    // wrap
    uint16_t len_until_max_size = fifo->max_size - fifo->head_idx;
    uint16_t len_after_wrap = len - len_until_max_size;
    memcpy(buffer, fifo->buffer + fifo->head_idx, len_until_max_size);
    memcpy(buffer + len_until_max_size, fifo->buffer, len_after_wrap);
    fifo->head_idx = len_after_wrap;
    return SUCCESS;
}

error_t fifo_peek(fifo_t* fifo, uint8_t* buffer, uint16_t offset, uint16_t len)
{
    if(len > fifo_get_size(fifo)) return ESIZE;

    uint16_t start_idx = fifo->head_idx + offset;
    if(fifo->head_idx + offset + len < fifo->max_size)
    {
        memcpy(buffer, fifo->buffer + start_idx, len);
        return SUCCESS;
    }

    if(start_idx > fifo->max_size)
        start_idx -= fifo->max_size;

    uint16_t space_before_max_size = fifo->max_size - start_idx;
    memcpy(buffer, fifo->buffer + start_idx, space_before_max_size);
    memcpy(buffer + space_before_max_size, fifo->buffer, len - space_before_max_size);
    return SUCCESS;
}

int16_t fifo_get_size(fifo_t* fifo)
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
