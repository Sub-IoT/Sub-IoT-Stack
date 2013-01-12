/*
 *  Created on: Jan 11, 2013
 *  Authors:
 *  	alexanderhoet@gmail.com
 */

#include <stdint.h>

#include "fec.h"

/*
 * input/output must be a pointer to an array of 1 x uint32 or 2 x uint16 or 4 x uint8
 */
void interleave_deinterleave(uint16_t* input, uint16_t* output)
{
	register uint16_t input0;
	register uint16_t input1;
	register uint16_t output0;
	register uint16_t output1;

	input0 = input[0];
	input1 = input[1];

	output0 = (input0 >> 10) & 0x03;
	output0 |= ((input0 >> 2) & 0x03) << 2;
	output0 |= ((input1 >> 10) & 0x03) << 4;
	output0 |= ((input1 >> 2) & 0x03) << 6;
	output0 |= ((input0 >> 8) & 0x03) << 8;
	output0 |= (input0 & 0x03) << 10;
	output0 |= ((input1 >> 8) & 0x03) << 12;
	output0 |= (input1 & 0x03) << 14;

	output1 = (input0 >> 14) & 0x03;
	output1 |= ((input0 >> 6) & 0x03) << 2;
	output1 |= ((input1 >> 14) & 0x03) << 4;
	output1 |= ((input1 >> 6) & 0x03) << 6;
	output1 |= ((input0 >> 12) & 0x03) << 8;
	output1 |= ((input0 >> 4) & 0x03) << 10;
	output1 |= ((input1 >> 12) & 0x03) << 12;
	output1 |= ((input1 >> 4) & 0x03) << 14;

	output[0] = output0;
	output[1] = output1;
}
