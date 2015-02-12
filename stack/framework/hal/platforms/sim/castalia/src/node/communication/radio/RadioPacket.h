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
