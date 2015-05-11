/*! \file trans.h
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
 * \author maarten.weyn@uantwerpen.be
 *
 */

#ifndef TRANS_H_
#define TRANS_H_

typedef struct {
    union {
        uint8_t addressee_ctrl;
        struct {
            uint8_t _rfu : 2;
            bool addressee_ctrl_unicast : 1;
            bool addressee_ctrl_virtual_id : 1;
            uint8_t addressee_ctrl_access_class : 2;
        };
    };
    uint8_t addressee_id[8]; // TODO assuming 8 byte id for now
} trans_addressee_t;

#endif /* TRANS_H_ */
