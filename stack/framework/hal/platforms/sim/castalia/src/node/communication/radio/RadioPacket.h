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

/*
 * RadioPacket.h
 *
 *  Created on: 12 Feb 2015
 *      Author: Daniel van den Akker
 */

#ifndef _RADIO_RADIOPACKET_H_
#define _RADIO_RADIOPACKET_H_

#include "RadioPacket_m.h"
class RadioPacket : public RadioPacket_Base
{
  public:
	RadioPacket(const char *name=NULL, int kind = 0);
	RadioPacket(const RadioPacket& other);
	RadioPacket& operator=(const RadioPacket& other);
    virtual RadioPacket *dup() const;

    uint8_t* getBufferPtr();

};

#endif /* _RADIOPACKET_H_ */
