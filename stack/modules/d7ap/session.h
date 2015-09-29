/*! \file session.h
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
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
 *
 * \author glenn.ergeerts@uantwerpen.be
 *
 */

#ifndef SESSION_H_
#define SESSION_H_

#include "stdint.h"
#include "stdbool.h"


typedef enum {
    SESSION_STATE_IDLE = 0x00,
    SESSION_STATE_DORMANT = 0x01,
    SESSION_STATE_PENDING = 0x02,
    SESSION_STATE_ACTIVE = 0x03,
    SESSION_STATE_DONE = 0x04,
} session_state_t;

typedef enum  {
    SESSION_RESP_MODE_NONE = 0,
    SESSION_RESP_MODE_ALLCAST = 1,
    SESSION_RESP_MODE_ANYCAST = 2,
} session_resp_mode_t;

typedef struct {
    uint8_t qos_retry_total;
    uint8_t qos_retry_single;
    uint8_t qos_ack_period;
    union {
        uint8_t qos_ctrl;
        struct {
            session_resp_mode_t qos_ctrl_resp_mode : 2;
            uint8_t _rfu : 2;
            bool qos_ctrl_ack_not_void : 1;
            uint8_t _rfu2 : 3;
        };
    };
} session_qos_t;


#endif /* SESSION_H_ */
