
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

/**
 * @file fifo.h
 * @addtogroup fifo
 * @ingroup framework
 * @{
 * @brief A generic FIFO implementation which allows pushing and popping bytes in a circular buffer.
 *
 */

#ifndef FIFO_H
#define FIFO_H

#include "types.h"

/**
 * @brief This struct contains the FIFO state variables
 *
 * A pointer to this is passed to alle functions of the fifo module
 **/
typedef struct {
    uint16_t head_idx;      /**< The index in buffer to first data byte of the FIFO */
    uint16_t tail_idx;      /**< The index in buffer to first empty byte of the FIFO */
    uint16_t max_size;      /**< The maximum number of bytes contained in the FIFO */
    uint8_t* buffer;        /**< The buffer where the data is stored*/
    bool is_full;          /**< Used to discern between full and empty when tail_idx == head_idx */
    bool is_subview;
} fifo_t;

/**
 * @brief Initializes the fifo.
 * @param fifo          Fifo state, initialized by this function
 * @param buffer        The buffer used for the fifo, the caller is responsible for allocating this to be big enough for max_size
 * @param max_size      The maximum size of bytes contained in the FIFO
 */
void fifo_init(fifo_t* fifo, uint8_t* buffer, uint16_t max_size);

/**
 * @brief Initializes the fifo with a pre-filled buffer
 * @param fifo          Fifo state, initialized by this function
 * @param buffer        The buffer used for the fifo, the caller is responsible for allocating this to be big enough for max_size
 * @param filled_size   The length of the pre-filled buffer
 * @param max_size      The maximum size of bytes contained in the FIFO
 */
void fifo_init_filled(fifo_t *fifo, uint8_t *buffer, uint16_t filled_size, uint16_t max_size);

/**
 * @brief Initializes a fifo which is a subset of an existing fifo, starting from original_fifo's head.
 * Note that appending to a subset fifo is not allowed. Popping from a subset fifo is allowed, but does not change original_fifo's head.
 * @param subset_fifo   The fifo containing the subset
 * @param original_fifo The original fifo
 * @param offset        The offset index in original fifo which will be used as the subset's head (starting from original_fifo's head)
 * @param subset_size   The size of the subset (must be smaller than fifo_get_size(&original_fifo))
 */
error_t fifo_init_subview(fifo_t *subview_fifo, fifo_t* original_fifo, uint16_t offset, uint16_t subset_size);

/**
 * @brief Put bytes in to the FIFO
 * @param fifo  Pointer to the fifo object
 * @param data  Pointer to the data to be put in the FIFO
 * @param len   Number of bytes to put in the FIFO
 * @returns SUCCESS or ESIZE when data would overwrite head of FIFO
 */
error_t fifo_put(fifo_t* fifo, uint8_t* data, uint16_t len);

/**
 * @brief Put byte in to the FIFO
 * @param fifo  Pointer to the fifo object
 * @param byte  Byte to be put in the FIFO
 * @returns SUCCESS or ESIZE when data would overwrite head of FIFO
 */
error_t fifo_put_byte(fifo_t* fifo, uint8_t byte);

/**
 * @brief Peek at the FIFO contents without popping. Fills buffer with the data in the FIFO starting from head_idx + offset for len bytes
 * @param fifo      Pointer to the fifo object
 * @param buffer    buffer to be filled
 * @param offset    offset starting from head
 * @param len       length in number of bytes to read
 * @returns SUCCESS or ESIZE when len > max_size or len > current size, or EINVAL when fifo is a subview
 */
error_t fifo_peek(fifo_t* fifo, uint8_t* buffer, uint16_t offset, uint16_t len);

/**
 * @brief Read and pop bytes from the FIFO
 * @param fifo      Pointer to the fifo object
 * @param buffer    Pointer to buffer where the first len bytes of FIFO can be copied to. Caller is responsible for allocating this buffer.
 * @param len       number of bytes to read/pop
 * @returns SUCCESS or ESIZE if len > current size or when FIFO empty
 */
error_t fifo_pop(fifo_t* fifo, uint8_t* buffer, uint16_t len);

/**
 * @brief Skips bytes from the FIFO
 * @param fifo      Pointer to the fifo object
 * @param len       number of bytes to stike
 * @returns SUCCESS or ESIZE if len > current size or when FIFO empty
 */
error_t fifo_skip(fifo_t* fifo, uint16_t len);

/**
 * @brief Clears the FIFO
* @param fifo      Pointer to the fifo object
 */
void fifo_clear(fifo_t* fifo);

/**
 * @brief Returns the number of bytes currently in the FIFO
 * @param fifo      Pointer to the fifo object
 * @return Number of bytes currently in the FIFO
 */
uint16_t fifo_get_size(fifo_t* fifo);

/**
 * @brief Gives access to the continuos block of bytes currently in the FIFO
 * @param fifo      Pointer to the fifo object
 * @param pdata     Pointer to a data pointer in which the start of the continuos block will be written
 * @param plen      Pointer to a length variable in which the length of the continuos block will be written
 */
void fifo_get_continuos_raw_data(fifo_t* fifo, uint8_t** pdata, uint16_t* plen);

/**
 * @brief Returns if the FIFO is completely full or if there is still space left
 * @param fifo      Pointer to the fifo object
 * @return Flag indicating if FIFO is full
 */
bool fifo_is_full(fifo_t* fifo);

/**
 * @brief Remove the last byte put in the FIFO
 * @param fifo      Pointer to the fifo object
 * @return SUCCESS or ESIZE when FIFO empty
 */
error_t fifo_remove_last_byte(fifo_t* fifo);

#endif // FIFO_H

/** @}*/
