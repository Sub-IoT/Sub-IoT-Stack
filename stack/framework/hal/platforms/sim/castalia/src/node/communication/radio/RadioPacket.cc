#include "RadioPacket.h"


Register_Class(RadioPacket);

RadioPacket::RadioPacket(const char *name, int kind):RadioPacket_Base(name, kind)
{
}
RadioPacket::RadioPacket(const RadioPacket& other): RadioPacket_Base(other)
{
}

RadioPacket& RadioPacket::operator=(const RadioPacket& other)
{
	RadioPacket_Base::operator=(other);
	return *this;
}

RadioPacket *RadioPacket::dup() const
{
	return new RadioPacket(*this);
}

uint8_t* RadioPacket::getBufferPtr()
{
	return buffer_var;
}
