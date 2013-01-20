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

void pn9_encode_decode(uint8_t* input, uint8_t* output, uint16_t length, uint16_t* pn9)
{
	register uint16_t i;
	register uint8_t j;
	register uint8_t bit0;
	register uint8_t bit5;

	for (i = 0; i < length; i++) {
		*output = *input ^ (uint8_t)*pn9;

		for (j = 8; j != 0; j--) {
			bit0 = (uint8_t)(*pn9 & 0x01);
			bit5 = (uint8_t)((*pn9 >> 5) & 0x01);

			*pn9 >>= 1;
			*pn9 |= (bit0 ^ bit5) << 8;
		}

		input++;
		output++;
	}
}
