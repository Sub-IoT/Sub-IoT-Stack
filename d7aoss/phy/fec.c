/*
 *  Created on: Jan 11, 2013
 *  Authors:
 *  	alexanderhoet@gmail.com
 */

#include <stdint.h>

#include "fec.h"

const uint8_t fec_lut[16] = {0, 3, 1, 2, 3, 0, 2, 1 , 3 , 0, 2, 1, 0, 3, 1, 2};

/*
 * output array size must at least be 2 x length and a multiple of 4bytes
 * TODO append trellis terminator
 */
void conv_encode(uint8_t* input, uint8_t* output, uint16_t length)
{
	uint16_t i;
	register uint8_t state;
	register uint8_t input_tmp;
	register uint16_t output_tmp;

	state = 0;

	for (i = 0; i < length; i++) {
		input_tmp = input[i];

		//bit 7
		state = (state << 1) & 0x0E;
		state |= (input_tmp >> 7) & 0x01;
		output_tmp = fec_lut[state] << 14;

		//bit 6
		state = (state << 1) & 0x0E;
		state |= (input_tmp >> 6) & 0x01;
		output_tmp |= fec_lut[state] << 12;

		//bit 5
		state = (state << 1) & 0x0E;
		state |= (input_tmp >> 5) & 0x01;
		output_tmp |= fec_lut[state] << 10;

		//bit 4
		state = (state << 1) & 0x0E;
		state |= (input_tmp >> 4) & 0x01;
		output_tmp |= fec_lut[state] << 8;

		//bit 3
		state = (state << 1) & 0x0E;
		state |= (input_tmp >> 3) & 0x01;
		output_tmp |= fec_lut[state] << 6;

		//bit 2
		state = (state << 1) & 0x0E;
		state |= (input_tmp >> 2) & 0x01;
		output_tmp |= fec_lut[state] << 4;

		//bit 1
		state = (state << 1) & 0x0E;
		state |= (input_tmp >> 1) & 0x01;
		output_tmp |= fec_lut[state] << 2;

		//bit 0
		state = (state << 1) & 0x0E;
		state |= input_tmp & 0x01;
		output_tmp |= fec_lut[state];

		output[i << 1] = output_tmp >> 8;
		output[(i << 1) + 1] = output_tmp;
	}
}

void conv_decode()
{

}

/*
 * buffer size must be a multiple of 4
 * input and output buffer may be the same
 */
void interleave_deinterleave(uint8_t* input, uint8_t* output, uint16_t length)
{
	uint16_t i;
	register uint16_t input0;
	register uint16_t input1;
	register uint16_t output0;
	register uint16_t output1;

	for (i = 0; i < length; i+=4) {
		input0 = (input[i] << 8) | input[i+1];
		input1 = (input[i+2] << 8) | input[i+3];

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

		output[i] = (uint8_t)(output0 >> 8);
		output[i+1] = (uint8_t)output0;
		output[i+2] = (uint8_t)(output1 >> 8);
		output[i+3] = (uint8_t)output1;
	}
}
