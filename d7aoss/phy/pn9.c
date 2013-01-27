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
	register uint16_t pn9_tmp;

	pn9_tmp = *pn9;

	for (i = length; i > 0; i--) {
		*output = *input ^ (uint8_t)*pn9;

		pn9_tmp |= ((pn9_tmp << 9) & 0x1E00) ^ ((pn9_tmp << 4) & 0x1E00);
		pn9_tmp >>= 4;
		pn9_tmp |= ((pn9_tmp << 9) & 0x1E00) ^ ((pn9_tmp << 4) & 0x1E00);
		pn9_tmp >>= 4;

		input++;
		output++;
	}

	*pn9 = pn9_tmp;
}
