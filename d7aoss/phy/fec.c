/*
 *  Created on: Jan 11, 2013
 *  Authors:
 *  	alexanderhoet@gmail.com
 */

#include <stdint.h>

#include "fec.h"

uint16_t interleave_deinterleave(uint16_t data)
{
	uint16_t output = 0;

	output = (data & 0x01) << 3;
	output |= ((data >> 1) & 0x01) << 7;
	output |= ((data >> 2) & 0x01) << 11;
	output |= ((data >> 3) & 0x01) << 15;
	output |= ((data >> 4) & 0x01) << 2;
	output |= ((data >> 5) & 0x01) << 6;
	output |= ((data >> 6) & 0x01) << 10;
	output |= ((data >> 7) & 0x01) << 14;
	output |= ((data >> 8) & 0x01) << 1;
	output |= ((data >> 9) & 0x01) << 5;
	output |= ((data >> 10) & 0x01) << 9;
	output |= ((data >> 11) & 0x01) << 13;
	output |= ((data >> 12) & 0x01);
	output |= ((data >> 13) & 0x01) << 4;
	output |= ((data >> 14) & 0x01) << 8;
	output |= ((data >> 15) & 0x01) << 12;

	return output;
}
