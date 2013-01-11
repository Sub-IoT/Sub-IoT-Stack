/*
 *  Created on: Jan 11, 2013
 *  Authors:
 *  	alexanderhoet@gmail.com
 */

#include <stdint.h>

#include "pn9.h"

void pn9_init(uint16_t* pn9)
{
	*pn9 = INITIAL_PN9;
}

uint8_t pn9_encode_decode(uint8_t data, uint16_t* pn9)
{
	uint8_t result;
	register uint8_t i;
	register uint8_t bit0;
	register uint8_t bit5;

	result = data ^ (uint8_t)*pn9;

	for(i = 8; i != 0; i--) {
		bit0 = (uint8_t)(*pn9 & 0x01);
		bit5 = (uint8_t)((*pn9 >> 5) & 0x01);

		*pn9 >>= 1;
		*pn9 |= (bit0 ^ bit5) << 8;
	}

	return result;
}
