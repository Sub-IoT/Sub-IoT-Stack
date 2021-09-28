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

#ifndef __DMA_H__
#define __DMA_H__

#include <stdlib.h>

#include "types.h"
#include "link_c.h"

// expose uart_handle with unknown internals
typedef struct dma_handle dma_handle_t;

typedef enum
{
    PERIPH_USART1,
    PERIPH_USART2,
    PERIPH_LPUART1,
} dma_peripheral_t;

__LINK_C dma_handle_t* dma_channel_init(uint8_t channel_idx);
__LINK_C dma_handle_t* dma_channel_get_handle(uint8_t channel_idx);
__LINK_C void* dma_channel_get_hal_handle(uint8_t channel_idx);
__LINK_C bool dma_channel_enable(dma_handle_t* dma_handle);
__LINK_C bool dma_channel_disable(dma_handle_t* dma_handle);
__LINK_C void dma_channel_interrupt_enable(dma_handle_t* dma_handle);
__LINK_C void dma_channel_interrupt_disable(dma_handle_t* dma_handle);

#endif // __DMA_H__