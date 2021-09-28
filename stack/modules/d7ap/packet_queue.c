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

#include "packet_queue.h"
#include "MODULE_D7AP_defs.h"
#include "debug.h"
#include "packet.h"
#include "ng.h"
#include "log.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(MODULE_D7AP_PACKET_LOG_ENABLED)
#define DPRINT(...) log_print_stack_string(LOG_STACK_FWK, __VA_ARGS__)
#else
#define DPRINT(...)
#endif

typedef enum
{
    PACKET_QUEUE_ELEMENT_STATUS_FREE,       /*! The element is free */
    PACKET_QUEUE_ELEMENT_STATUS_ALLOCATED,  /*! The element is allocated and passed to hwradio for filling */
    PACKET_QUEUE_ELEMENT_STATUS_RECEIVED,   /*! The element contains a successfully received packet, ready for further processing */
    PACKET_QUEUE_ELEMENT_STATUS_TRANSMITTED,/*! The element contains a successfully transmitted packet */
    PACKET_QUEUE_ELEMENT_STATUS_PROCESSING  /*! Indicates the supplied packet is being processed */
} packet_queue_element_status_t;

static packet_t NGDEF(_packet_queue)[MODULE_D7AP_PACKET_QUEUE_SIZE];
#define packet_queue NG(_packet_queue)
static packet_queue_element_status_t NGDEF(_packet_queue_element_status)[MODULE_D7AP_PACKET_QUEUE_SIZE];
#define packet_queue_element_status NG(_packet_queue_element_status)

void packet_queue_init()
{
    for(uint8_t i = 0; i < MODULE_D7AP_PACKET_QUEUE_SIZE; i++)
    {
        packet_init(&(packet_queue[i]));
        packet_queue_element_status[i] = PACKET_QUEUE_ELEMENT_STATUS_FREE;
    }
}

packet_t* packet_queue_alloc_packet()
{
    for(uint8_t i = 0; i < MODULE_D7AP_PACKET_QUEUE_SIZE; i++)
    {
        if(packet_queue_element_status[i] == PACKET_QUEUE_ELEMENT_STATUS_FREE)
        {
            packet_queue_element_status[i] = PACKET_QUEUE_ELEMENT_STATUS_ALLOCATED;
            DPRINT("Packet queue alloc %p slot %i", &(packet_queue[i]), i);
            return &(packet_queue[i]);
        }
    }

    // should not happen, possible to small PACKET_QUEUE_SIZE or not always free()-ed correctly?
    DPRINT("Packet queue full, could not alloc new packet!");
    return NULL;
}

void packet_queue_free_packet(packet_t* packet)
{
    DPRINT("Packet queue mark free %p", packet);
    for(uint8_t i = 0; i < MODULE_D7AP_PACKET_QUEUE_SIZE; i++)
    {
        if(packet == &(packet_queue[i]))
        {
            DPRINT("packet slot %i", i);
            assert(packet_queue_element_status[i] >= PACKET_QUEUE_ELEMENT_STATUS_ALLOCATED);
            packet_queue_element_status[i] = PACKET_QUEUE_ELEMENT_STATUS_FREE;
            packet_init(&(packet_queue[i]));
            return;
        }
    }

    assert(false); // should never happen
}

packet_t* packet_queue_find_packet(hw_radio_packet_t* hw_radio_packet)
{
    for(uint8_t i = 0; i < MODULE_D7AP_PACKET_QUEUE_SIZE; i++)
    {
        if(&(packet_queue[i].hw_radio_packet) == hw_radio_packet)
            return &(packet_queue[i]);
    }

    return NULL;
}

void packet_queue_mark_processing(packet_t* packet)
{
    DPRINT("Packet queue mark processing %p", packet);
    for(uint8_t i = 0; i < MODULE_D7AP_PACKET_QUEUE_SIZE; i++)
    {
        if(packet == &(packet_queue[i]))
        {
            assert(packet_queue_element_status[i] != PACKET_QUEUE_ELEMENT_STATUS_FREE);
            DPRINT("Packet slot %i", i);
            packet_queue_element_status[i] = PACKET_QUEUE_ELEMENT_STATUS_PROCESSING;
            return;
        }
    }

    assert(false);
}
