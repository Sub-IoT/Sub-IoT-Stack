/*
 *  Created on: Jan 11, 2013
 *  Authors:
 *  	alexanderhoet@gmail.com
 */

#include <stdint.h>

#include "fec.h"

uint8_t fec_lut[16] = {0, 3, 1, 2, 3, 0, 2, 1 , 3 , 0, 2, 1, 0, 3, 1, 2};

void conv_init(uint8_t* state)
{
	*state = 0;
}

void conv_encode(uint8_t* input, uint16_t* output, uint8_t* state)
{
	register uint8_t state_tmp;
	register uint8_t input_tmp;
	register uint16_t output_tmp;

	input_tmp = *input;
	state_tmp = *state;

	//bit 7
	state_tmp = (state_tmp << 1) & 0x0E;
	state_tmp |= (input_tmp >> 7) & 0x01;
	output_tmp = fec_lut[state_tmp] << 14;

	//bit 6
	state_tmp = (state_tmp << 1) & 0x0E;
	state_tmp |= (input_tmp >> 6) & 0x01;
	output_tmp |= fec_lut[state_tmp] << 12;

	//bit 5
	state_tmp = (state_tmp << 1) & 0x0E;
	state_tmp |= (input_tmp >> 5) & 0x01;
	output_tmp |= fec_lut[state_tmp] << 10;

	//bit 4
	state_tmp = (state_tmp << 1) & 0x0E;
	state_tmp |= (input_tmp >> 4) & 0x01;
	output_tmp |= fec_lut[state_tmp] << 8;

	//bit 3
	state_tmp = (state_tmp << 1) & 0x0E;
	state_tmp |= (input_tmp >> 3) & 0x01;
	output_tmp |= fec_lut[state_tmp] << 6;

	//bit 2
	state_tmp = (state_tmp << 1) & 0x0E;
	state_tmp |= (input_tmp >> 2) & 0x01;
	output_tmp |= fec_lut[state_tmp] << 4;

	//bit 1
	state_tmp = (state_tmp << 1) & 0x0E;
	state_tmp |= (input_tmp >> 1) & 0x01;
	output_tmp |= fec_lut[state_tmp] << 2;

	//bit 0
	state_tmp = (state_tmp << 1) & 0x0E;
	state_tmp |= input_tmp & 0x01;
	output_tmp |= fec_lut[state_tmp];

	*output = output_tmp;
	*state = state_tmp;
}

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
