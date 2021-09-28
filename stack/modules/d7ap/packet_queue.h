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

/*! \file packet_queue.h
 * \addtogroup Packet_queue
 * \ingroup D7AP
 * @{
 * \brief Contains a (configurable) number of slots for keeping [packets](@ref packet_t) while processing though the different layers of the stack
 * \author glenn.ergeerts@uantwerpen.be
 */

#ifndef OSS_7_PACKET_QUEUE_H
#define OSS_7_PACKET_QUEUE_H

#include "packet.h"

/*! Initializes the packet queue */
void packet_queue_init();

/*! Returns the first free packet buffer in the queue and marks this as used until this is free()-ed again */
packet_t* packet_queue_alloc_packet();

/*! Marks the packet buffer as free again */
void packet_queue_free_packet(packet_t*);

/*! Finds the packet_t corresponding to the supplied hw_radio_packet_t */
packet_t* packet_queue_find_packet(hw_radio_packet_t*);

/*! Indicates the supplied packet has been successfully received and is ready for further processing */
packet_t* packet_queue_mark_received(hw_radio_packet_t*);

/*! Indicates the supplied packet is being processed */
void packet_queue_mark_processing(packet_t*);

/*! Get a received packet for further processing. Returns NULL if no received packet queued. */
packet_t* packet_queue_get_received_packet();
#endif //OSS_7_PACKET_QUEUE_H

/** @}*/
