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

#include "hwdma.h"
#include "debug.h"
#include "stm32_device.h"
#include "ports.h"

// private definition of the DMA handle, passed around publicly as a pointer
struct dma_handle
{
    const dma_channel_t* dma_channel;
    DMA_HandleTypeDef dma_hal_handle;
    bool is_active;
    
};

// private storage of handles, pointers to these records are passed around
static dma_handle_t handle[DMA_COUNT];

dma_handle_t* dma_channel_init(uint8_t channel_idx)
{
    assert(channel_idx < DMA_COUNT);
    handle[channel_idx].dma_channel = &dma_channels[channel_idx];
    handle[channel_idx].is_active = false;
    return &handle[channel_idx];
}

dma_handle_t* dma_channel_get_handle(uint8_t channel_idx)
{
    if(handle[channel_idx].dma_channel)
    {
        return &handle[channel_idx];
    }
    return NULL;
}

void* dma_channel_get_hal_handle(uint8_t channel_idx)
{
    if(handle[channel_idx].dma_channel)
    {
        return &handle[channel_idx].dma_hal_handle;
    }
    return NULL;
}

bool dma_channel_enable(dma_handle_t* dma_handle)
{
    switch(dma_handle->dma_channel->peripheral)
    {
        case PERIPH_USART1:
        {
            dma_handle->dma_hal_handle.Init.Request = DMA_REQUEST_3;
            break;
        }
        case PERIPH_USART2:
        {
            dma_handle->dma_hal_handle.Init.Request = DMA_REQUEST_4;
            break;
        }
        case PERIPH_LPUART1:
        {
            dma_handle->dma_hal_handle.Init.Request = DMA_REQUEST_5;
            break;
        }
        default:
        {
            return false;
        }
    }
    // Note that the direction selection code is only valid for UARTS
    if(dma_handle->dma_channel->channel_nr == 2 || dma_handle->dma_channel->channel_nr == 4  || dma_handle->dma_channel->channel_nr == 7)
    {
        dma_handle->dma_hal_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
    }
    else if(dma_handle->dma_channel->channel_nr == 3 || dma_handle->dma_channel->channel_nr == 5  || dma_handle->dma_channel->channel_nr == 6)
    {
        dma_handle->dma_hal_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    }
    else
    {
        return false;
    }
    
    dma_handle->dma_hal_handle.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_handle->dma_hal_handle.Init.MemInc = DMA_MINC_ENABLE;
    dma_handle->dma_hal_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_handle->dma_hal_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_handle->dma_hal_handle.Init.Mode = DMA_NORMAL;
    if(dma_handle->dma_hal_handle.Init.Direction == DMA_MEMORY_TO_PERIPH)
    {
        dma_handle->dma_hal_handle.Init.Priority = DMA_PRIORITY_LOW;
    }
    else
    {
        dma_handle->dma_hal_handle.Init.Priority = DMA_PRIORITY_MEDIUM;
    }
    switch(dma_handle->dma_channel->channel_nr)
    {
        case 1:
        {
            dma_handle->dma_hal_handle.Instance = DMA1_Channel1;
            break;
        }
        case 2:
        {
            dma_handle->dma_hal_handle.Instance = DMA1_Channel2;
            break;
        }
        case 3:
        {
            dma_handle->dma_hal_handle.Instance = DMA1_Channel3;
            break;
        }
        case 4:
        {
            dma_handle->dma_hal_handle.Instance = DMA1_Channel4;
            break;
        }
        case 5:
        {
            dma_handle->dma_hal_handle.Instance = DMA1_Channel5;
            break;
        }
        case 6:
        {
            dma_handle->dma_hal_handle.Instance = DMA1_Channel6;
            break;
        }
        case 7:
        {
            dma_handle->dma_hal_handle.Instance = DMA1_Channel7;
            break;
        }
        default:
        {
            return false;
        }
    }

    // Check if DMA clock is enabled
    if(__HAL_RCC_DMA1_IS_CLK_DISABLED())
    {
        __HAL_RCC_DMA1_CLK_ENABLE();
        assert(__HAL_RCC_DMA1_IS_CLK_ENABLED());
    }
    dma_handle->is_active = true;

    if(HAL_DMA_Init(&dma_handle->dma_hal_handle) != HAL_OK)
    {
        return false;
    }

    return true;
}

bool dma_channel_disable(dma_handle_t* dma_handle)
{
    HAL_DMA_DeInit(&dma_handle->dma_hal_handle);
    dma_handle->is_active = false;
    bool disable_dma_clock = true;
    for(uint8_t index = 0; index < DMA_COUNT; index++)
    {
        if(handle[index].is_active)
        {
            disable_dma_clock = false;
            break;
        }
    }
    if(disable_dma_clock)
    {
        __HAL_RCC_DMA1_CLK_DISABLE();
    }
    return true;
}

void dma_channel_interrupt_enable(dma_handle_t* dma_handle)
{
  HAL_NVIC_ClearPendingIRQ(dma_handle->dma_channel->irq);
  HAL_NVIC_EnableIRQ(dma_handle->dma_channel->irq);
}

void dma_channel_interrupt_disable(dma_handle_t* dma_handle)
{
  HAL_NVIC_ClearPendingIRQ(dma_handle->dma_channel->irq);
  HAL_NVIC_DisableIRQ(dma_handle->dma_channel->irq);
}

void DMA1_Channel2_3_IRQHandler(void)
{
    for(uint8_t index = 0; index < DMA_COUNT; index++)
    {
        if(handle[index].is_active)
        {
            if(__HAL_DMA_GET_FLAG(&handle[index].dma_hal_handle, __HAL_DMA_GET_TC_FLAG_INDEX(&handle[index].dma_hal_handle) | 
                                                             __HAL_DMA_GET_HT_FLAG_INDEX(&handle[index].dma_hal_handle) |
                                                             __HAL_DMA_GET_TE_FLAG_INDEX(&handle[index].dma_hal_handle) |
                                                             __HAL_DMA_GET_GI_FLAG_INDEX(&handle[index].dma_hal_handle)))
                                                             {
                                                                 HAL_DMA_IRQHandler(&handle[index].dma_hal_handle);
                                                             }
        }
    }
}
