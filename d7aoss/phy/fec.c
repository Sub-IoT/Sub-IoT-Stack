/*! \file fec.c
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * \author glenn.ergeerts@uantwerpen.be
 * \author maarten.weyn@uantwerpen.be
 *	\author alexanderhoet@gmail.com
 *
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "phy.h"
#include "fec.h"

#ifdef D7_PHY_USE_FEC

const uint8_t fec_lut[16] = {0, 3, 1, 2, 3, 0, 2, 1, 3, 0, 2, 1, 0, 3, 1, 2};
const uint8_t trellis0_lut[8] = {0, 1, 3, 2, 3, 2, 0, 1};
const uint8_t trellis1_lut[8] = {3, 2, 0, 1, 0, 1, 3, 2};

uint8_t* iobuffer;

uint8_t packetlength;
uint16_t fecpacketlength;

uint8_t processedbytes;
uint16_t fecprocessedbytes;

uint16_t pn9;
uint16_t fecstate;
VITERBISTATE vstate;

void fec_init_encode(uint8_t* input)
{
	iobuffer = input;

	packetlength = 255;
	fecpacketlength = 512;

	processedbytes = 0;
	fecprocessedbytes = 0;

	fecstate = INITIAL_FECSTATE;
}

void fec_init_decode(uint8_t* output)
{
	iobuffer = output;

	packetlength = 255;
	fecpacketlength = 512;

	processedbytes = 0;
	fecprocessedbytes = 0;

	vstate.path_size = 0;

	vstate.states1[0].cost = 100;
	vstate.states1[1].cost = 100;
	vstate.states1[2].cost = 100;
	vstate.states1[3].cost = 100;
	vstate.states1[4].cost = 100;
	vstate.states1[5].cost = 100;
	vstate.states1[6].cost = 100;
	vstate.states1[7].cost = 0;

	vstate.old = vstate.states1;
	vstate.new = vstate.states2;
}

void fec_set_length(uint8_t length)
{
	packetlength = length;
	fecpacketlength = ((length & 0xFE) + 2) << 1;
}

bool fec_encode(uint8_t* output)
{
	uint8_t i;
	uint16_t fecbuffer[2];

	if(fecprocessedbytes >= fecpacketlength)
		return false;

	for(i = 0; i < 2; i++)
	{
		//Get byte from the input buffer if available and apply data whitening, otherwise append trellis terminator
		// TODO: remove data whitening
		if(processedbytes < packetlength) {
			//Pn9 data whitening
//			pn9buffer = *iobuffer++ ^ (uint8_t)pn9;
//
//			//Rotate pn9 code
//			tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
//			pn9 = tmppn9 | (pn9 >> 4);
//			tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
//			pn9 = tmppn9 | (pn9 >> 4);
			pn9buffer = *iobuffer++;
			processedbytes++;
		} else {
			pn9buffer = TRELLIS_TERMINATOR;
		}

		//Convolutional encoding
		fecstate |= pn9buffer;

		fecbuffer[i] = fec_lut[fecstate >> 7] << 14;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 12;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 10;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 8;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 6;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 4;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 2;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7];
		fecstate = (fecstate << 1) & 0x07FF;
	}

	//Interleaving and write to output buffer
	output[0] = ((fecbuffer[0] >> 8) & 0x03);
	output[0] |= (fecbuffer[0] & 0x03) << 2;
	output[0] |= ((fecbuffer[1] >> 8) & 0x03) << 4;
	output[0] |= (fecbuffer[1] & 0x03) << 6;
	output[1] = (fecbuffer[0] >> 10) & 0x03;
	output[1] |= ((fecbuffer[0] >> 2) & 0x03) << 2;
	output[1] |= ((fecbuffer[1] >> 10) & 0x03) << 4;
	output[1] |= ((fecbuffer[1] >> 2) & 0x03) << 6;
	output[2] = ((fecbuffer[0] >> 12) & 0x03);
	output[2] |= ((fecbuffer[0] >> 4) & 0x03) << 2;
	output[2] |= ((fecbuffer[1] >> 12) & 0x03) << 4;
	output[2] |= ((fecbuffer[1] >> 4) & 0x03) << 6;
	output[3] = (fecbuffer[0] >> 14) & 0x03;
	output[3] |= ((fecbuffer[0] >> 6) & 0x03) << 2;
	output[3] |= ((fecbuffer[1] >> 14) & 0x03) << 4;
	output[3] |= ((fecbuffer[1] >> 6) & 0x03) << 6;

	fecprocessedbytes += 4;

	return true;
}

bool fec_decode(uint8_t* input)
{
	uint8_t i, j, k;
	uint8_t min_state;
	uint8_t symbol;
	uint16_t tmppn9;
	uint16_t fecbuffer[2];
	VITERBIPATH* vstate_tmp;

	if(fecprocessedbytes >= fecpacketlength)
		return false;

	//Deinterleaving (symbols are stored in reverse as this is easier for Viterbi decoding)
	fecbuffer[0]  = ((input[0] >> 2) & 0x03) << 14;
	fecbuffer[0] |= ((input[1] >> 2) & 0x03) << 12;
	fecbuffer[0] |= ((input[2] >> 2) & 0x03) << 10;
	fecbuffer[0] |= ((input[3] >> 2) & 0x03) << 8;
	fecbuffer[0] |= (input[0] & 0x03) << 6;
	fecbuffer[0] |= (input[1] & 0x03) << 4;
	fecbuffer[0] |= (input[2] & 0x03) << 2;
	fecbuffer[0] |= (input[3] & 0x03);
	fecbuffer[1]  = ((input[0] >> 6) & 0x03) << 14;
	fecbuffer[1] |= ((input[1] >> 6) & 0x03) << 12;
	fecbuffer[1] |= ((input[2] >> 6) & 0x03) << 10;
	fecbuffer[1] |= ((input[3] >> 6) & 0x03) << 8;
	fecbuffer[1] |= ((input[0] >> 4) & 0x03) << 6;
	fecbuffer[1] |= ((input[1] >> 4) & 0x03) << 4;
	fecbuffer[1] |= ((input[2] >> 4) & 0x03) << 2;
	fecbuffer[1] |= (input[3] >> 4) & 0x03;

	fecprocessedbytes +=4;

	for (i = 0; i < 2; i++) {
		//Viterbi decoding
		for (j = 8; j != 0; j--) {
			symbol = fecbuffer[i] & 0x03;
			fecbuffer[i] >>= 2;

			for(k = 0; k < 8; k++) {
				uint8_t cost0, cost1;
				uint8_t state0, state1;
				uint8_t hamming0, hamming1;

				state0 = k >> 1;
				state1 = state0 + 4;

				cost0  = vstate.old[state0].cost;
				cost1  = vstate.old[state1].cost;

				hamming0 = cost0 + ((trellis0_lut[state0] ^ symbol) + 1) >> 1;
				hamming1 = cost1 + ((trellis0_lut[state1] ^ symbol) + 1) >> 1;

				if(hamming0 <= hamming1) {
					vstate.new[k].cost = hamming0;
					vstate.new[k].path = vstate.old[state0].path << 1;
				} else {
					vstate.new[k].cost = hamming1;
					vstate.new[k].path = vstate.old[state1].path << 1;
				}

				k++;

				hamming0 = cost0 + ((trellis1_lut[state0] ^ symbol) + 1) >> 1;
				hamming1 = cost1 + ((trellis1_lut[state1] ^ symbol) + 1) >> 1;

				if(hamming0 <= hamming1) {
					vstate.new[k].cost = hamming0;
					vstate.new[k].path = vstate.old[state0].path << 1 | 0x01;
				} else {
					vstate.new[k].cost = hamming1;
					vstate.new[k].path = vstate.old[state1].path << 1 | 0x01;
				}
			}

			//Swap Viterbi paths
			vstate_tmp = vstate.new;
			vstate.new = vstate.old;
			vstate.old = vstate_tmp;
		}

		vstate.path_size++;

		//Flush out byte if path is full
		if ((vstate.path_size == 2) && (processedbytes < packetlength)) {
			//Calculate path with lowest cost
			min_state = 0;
			for (j = 7; j != 0; j--) {
				if(vstate.old[j].cost < vstate.old[min_state].cost)
					min_state = j;
			}

	        //Normalize costs
	        for (j = 0; j < 8; j++)
	        	vstate.old[j].cost -= vstate.old[min_state].cost;

			//Pn9 data dewhitening
			*iobuffer++ = (vstate.old[min_state].path >> 8) ^ pn9;

			//Rotate pn9 code
			tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
			pn9 = tmppn9 | (pn9 >> 4);
			tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
			pn9 = tmppn9 | (pn9 >> 4);

			vstate.path_size--;
			processedbytes++;
		}
	}

	return true;
}

#endif /* D7_PHY_USE_FEC */
