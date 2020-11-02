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

/*! \file gps.h
 *
 *  \author Liam Wickins <liamw9534@gmail.com>
 *
 */
#ifndef __NEO_M8N_GPS_H_
#define __NEO_M8N_GPS_H_

typedef enum
{
	GPS_EVENT_POS
} gps_event_id_t;

#pragma pack (1)
typedef struct
{
	gps_event_id_t id;
	union {
		struct {
			uint32_t itow;
			uint32_t longitude;
			uint32_t latitude;
			uint32_t height;
			uint32_t ttff;
			bool     is_fixed;
		} pos;
	};
} gps_event_t;

typedef void (*gps_callback_t)(gps_event_t *event);

void gps_init(gps_callback_t cb);
void gps_enable(void);
void gps_disable(void);

#endif // __NEO_M8N_GPS_H_
