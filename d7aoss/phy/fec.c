/*
 * The PHY layer API
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "phy.h"
#include "fec.h"

#ifdef D7_PHY_USE_FEC

const uint8_t fec_lut[16] = {0, 3, 1, 2, 3, 0, 2, 1 , 3 , 0, 2, 1, 0, 3, 1, 2};

uint8_t packetlength;
uint16_t fecpacketlength;

uint8_t processedbytes;
uint16_t fecprocessedbytes;

uint16_t pn9;
uint16_t fecstate;

void fec_init()
{
	packetlength = 255;
	fecpacketlength = 512;

	processedbytes = 0;
	fecprocessedbytes = 0;

	pn9 = INITIAL_PN9;
	fecstate = INITIAL_FECSTATE;
}

void fec_set_length(uint8_t length)
{
	packetlength = length;
	fecpacketlength = ((length & 0xFE) + 2) << 1;
}

bool fec_encode(uint8_t* input, uint8_t* output)
{
	uint8_t i;
	uint16_t tmppn9;
	uint8_t pn9buffer;
	uint16_t fecbuffer[2];

	if(fecprocessedbytes >= fecpacketlength)
		return false;

	for(i = 0; i < 2; i++)
	{


		// Get byte from the input buffer if available and apply data whitening, otherwise append trellis terminator
		if(processedbytes < packetlength) {
			if(input == NULL)
				return false;

			//Apply pn9 data whitening
			pn9buffer = input[i] ^ (uint8_t)pn9;

			//Get next pn9 code
			tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
			pn9 = tmp | (pn9 >> 4);
			tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
			pn9 = tmp | (pn9 >> 4);

			processedbytes++;
		} else {
			pn9buffer = TRELLIS_TERMINATOR;
		}

		//Apply convolutional encoding
		fecstate |= pn9buffer;

		fecbuffer[i] = fec_lut[fecstate >> 7] << 14;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] = fec_lut[fecstate >> 7] << 12;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] = fec_lut[fecstate >> 7] << 10;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] = fec_lut[fecstate >> 7] << 8;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] = fec_lut[fecstate >> 7] << 6;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] = fec_lut[fecstate >> 7] << 4;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] = fec_lut[fecstate >> 7] << 2;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] = fec_lut[fecstate >> 7];
		fecstate = (fecstate << 1) & 0x07FF;
	}

	//Apply interleaving and write to output buffer
	output[0] = (fecbuffer[0] >> 10) & 0x03;
	output[0] |= ((fecbuffer[0] >> 2) & 0x03) << 2;
	output[0] |= ((fecbuffer[1] >> 10) & 0x03) << 4;
	output[0] |= ((fecbuffer[1] >> 2) & 0x03) << 6;
	output[1] = ((fecbuffer[0] >> 8) & 0x03);
	output[1] |= (fecbuffer[0] & 0x03) << 2;
	output[1] |= ((fecbuffer[1] >> 8) & 0x03) << 4;
	output[1] |= (fecbuffer[1] & 0x03) << 6;
	output[2] = (fecbuffer[0] >> 14) & 0x03;
	output[2] |= ((fecbuffer[0] >> 6) & 0x03) << 2;
	output[2] |= ((fecbuffer[1] >> 14) & 0x03) << 4;
	output[2] |= ((fecbuffer[1] >> 6) & 0x03) << 6;
	output[3] = ((fecbuffer[0] >> 12) & 0x03);
	output[3] |= ((fecbuffer[0] >> 4) & 0x03) << 2;
	output[3] |= ((fecbuffer[1] >> 12) & 0x03) << 4;
	output[3] |= ((fecbuffer[1] >> 4) & 0x03) << 6;

	fecprocessedbytes += 4;

	return true;
}

bool fec_decode(uint8_t* input, uint8_t* output)
{
	uint8_t i;
	uint8_t pn9buffer[2];
	uint16_t fecbuffer[2];

	if(fecprocessedbytes >= fecpacketlength)
		return false;

	//Apply deinterleaving
	fecbuffer[0] = (input[0] >> 2) & 0x03;
	fecbuffer[0] |= ((input[1] >> 2) & 0x03) << 2;
	fecbuffer[0] |= ((input[2] >> 2) & 0x03) << 4;
	fecbuffer[0] |= ((input[3] >> 2) & 0x03) << 6;
	fecbuffer[0] |= (input[0] & 0x03) << 8;
	fecbuffer[0] |= (input[1] & 0x03) << 10;
	fecbuffer[0] |= (input[2] & 0x03) << 12;
	fecbuffer[0] |= (input[3] & 0x03) << 14;
	fecbuffer[1] = (input[0] >> 6) & 0x03;
	fecbuffer[1] |= ((input[1] >> 6) & 0x03) << 2;
	fecbuffer[1] |= ((input[2] >> 6) & 0x03) << 4;
	fecbuffer[1] |= ((input[3] >> 6) & 0x03) << 6;
	fecbuffer[1] |= ((input[0] >> 4) & 0x03) << 8;
	fecbuffer[1] |= ((input[1] >> 4) & 0x03) << 10;
	fecbuffer[1] |= ((input[2] >> 4) & 0x03) << 12;
	fecbuffer[1] |= ((input[3] >> 4) & 0x03) << 14;

	for(i = 0; i < 2; i++)
	{
		//Viterbi decoding


	}

	return true;
}

#endif /* D7_PHY_USE_FEC */
