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

#include "packet_queue.h"

#include "assert.h"
#include "packet.h"
#include "ng.h"

#define PACKET_QUEUE_SIZE 3 // TODO define from cmake

typedef enum
{
    PACKET_QUEUE_ELEMENT_STATUS_FREE,
    PACKET_QUEUE_ELEMENT_STATUS_ALLOCATED
} packet_queue_element_status_t;

static packet_t NGDEF(_packet_queue)[PACKET_QUEUE_SIZE];
#define packet_queue NG(_packet_queue)
static packet_queue_element_status_t NGDEF(_packet_queue_element_status)[PACKET_QUEUE_SIZE];
#define packet_queue_element_status NG(_packet_queue_element_status)

void packet_queue_init()
{
    for(uint8_t i = 0; i < PACKET_QUEUE_SIZE; i++)
    {
        packet_init(&(packet_queue[i]));
        packet_queue_element_status[i] = PACKET_QUEUE_ELEMENT_STATUS_FREE;
    }
}

packet_t* packet_queue_alloc_packet()
{
    for(uint8_t i = 0; i < PACKET_QUEUE_SIZE; i++)
    {
        if(packet_queue_element_status[i] == PACKET_QUEUE_ELEMENT_STATUS_FREE)
        {
            packet_queue_element_status[i] = PACKET_QUEUE_ELEMENT_STATUS_ALLOCATED;
            return &(packet_queue[i]);
        }
    }

    assert(false); // should not happen, possible to small PACKET_QUEUE_SIZE or not always free()-ed correctly?
}

void packet_queue_free_packet(packet_t* packet)
{
    for(uint8_t i = 0; i < PACKET_QUEUE_SIZE; i++)
    {
        if(packet == &(packet_queue[i]))
        {
            assert(packet_queue_element_status[i] == PACKET_QUEUE_ELEMENT_STATUS_ALLOCATED);
            packet_queue_element_status[i] = PACKET_QUEUE_ELEMENT_STATUS_FREE;
            packet_init(&(packet_queue[i]));
            return;
        }
    }

    assert(false); // should never happen
}

packet_t* packet_queue_find_packet(hw_radio_packet_t* hw_radio_packet)
{
    for(uint8_t i = 0; i < PACKET_QUEUE_SIZE; i++)
    {
        if(&(packet_queue[i].hw_radio_packet) == hw_radio_packet)
            return &(packet_queue[i]);
    }

    return NULL;
}
