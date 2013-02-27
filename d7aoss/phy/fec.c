/*
 * The PHY layer API
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include <stdbool.h>
#include <stdint.h>

#include "phy.h"
#include "fec.h"

#ifdef D7_PHY_USE_FEC

const uint8_t fec_lut[16] = {0, 3, 1, 2, 3, 0, 2, 1 , 3 , 0, 2, 1, 0, 3, 1, 2};

uint8_t packetlength;
uint16_t fecpacketlength;

uint8_t processedbytes;
uint16_t fecprocessedbytes;

uint8_t fecstate;
uint16_t pn9;

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
	uint8_t pn9buffer[2];
	uint16_t fecbuffer[2];
	uint16_t output0;
	uint16_t output1;

	if(fecprocessedbytes >= fecpacketlength)
		return false;

	for(i = 0; i < 2; i++)
	{
		// Get byte from the input buffer if available and apply data whitening, otherwise append trellis terminator
		if(processedbytes < packetlength) {
			//Apply pn9 data whitening
			pn9buffer[i] = input[i] ^ (uint8_t)pn9;

			//Get next pn9 code
			//TODO could be made even faster
			pn9 |= ((pn9 << 9) & 0x1E00) ^ ((pn9 << 4) & 0x1E00);
			pn9 >>= 4;
			pn9 |= ((pn9 << 9) & 0x1E00) ^ ((pn9 << 4) & 0x1E00);
			pn9 >>= 4;

			processedbytes++;
		} else {
			pn9buffer[i] = TRELLIS_TERMINATOR;
		}

		//Apply convolutional encoding
		//bit 7
		fecstate = (fecstate << 1) & 0x0E;
		fecstate |= (pn9buffer[i] >> 7) & 0x01;
		fecbuffer[i] = fec_lut[fecstate] << 14;

		//bit 6
		fecstate = (fecstate << 1) & 0x0E;
		fecstate |= (pn9buffer[i] >> 6) & 0x01;
		fecbuffer[i] |= fec_lut[fecstate] << 12;

		//bit 5
		fecstate = (fecstate << 1) & 0x0E;
		fecstate |= (pn9buffer[i] >> 5) & 0x01;
		fecbuffer[i] |= fec_lut[fecstate] << 10;

		//bit 4
		fecstate = (fecstate << 1) & 0x0E;
		fecstate |= (pn9buffer[i] >> 4) & 0x01;
		fecbuffer[i] |= fec_lut[fecstate] << 8;

		//bit 3
		fecstate = (fecstate << 1) & 0x0E;
		fecstate |= (pn9buffer[i] >> 3) & 0x01;
		fecbuffer[i] |= fec_lut[fecstate] << 6;

		//bit 2
		fecstate = (fecstate << 1) & 0x0E;
		fecstate |= (pn9buffer[i] >> 2) & 0x01;
		fecbuffer[i] |= fec_lut[fecstate] << 4;

		//bit 1
		fecstate = (fecstate << 1) & 0x0E;
		fecstate |= (pn9buffer[i] >> 1) & 0x01;
		fecbuffer[i] |= fec_lut[fecstate] << 2;

		//bit 0
		fecstate = (fecstate << 1) & 0x0E;
		fecstate |= pn9buffer[i] & 0x01;
		fecbuffer[i] |= fec_lut[fecstate];
	}

	//Apply interleaving
	output0 = (fecbuffer[0] >> 10) & 0x03;
	output0 |= ((fecbuffer[0] >> 2) & 0x03) << 2;
	output0 |= ((fecbuffer[1] >> 10) & 0x03) << 4;
	output0 |= ((fecbuffer[1] >> 2) & 0x03) << 6;
	output0 |= ((fecbuffer[0] >> 8) & 0x03) << 8;
	output0 |= (fecbuffer[0] & 0x03) << 10;
	output0 |= ((fecbuffer[1] >> 8) & 0x03) << 12;
	output0 |= (fecbuffer[1] & 0x03) << 14;

	output1 = (fecbuffer[0] >> 14) & 0x03;
	output1 |= ((fecbuffer[0] >> 6) & 0x03) << 2;
	output1 |= ((fecbuffer[1] >> 14) & 0x03) << 4;
	output1 |= ((fecbuffer[1] >> 6) & 0x03) << 6;
	output1 |= ((fecbuffer[0] >> 12) & 0x03) << 8;
	output1 |= ((fecbuffer[0] >> 4) & 0x03) << 10;
	output1 |= ((fecbuffer[1] >> 12) & 0x03) << 12;
	output1 |= ((fecbuffer[1] >> 4) & 0x03) << 14;

	//Write output to output buffer
	output[0] = (uint8_t)(output0 >> 8);
	output[1] = (uint8_t)output0;
	output[2] = (uint8_t)(output1 >> 8);
	output[3] = (uint8_t)output1;

	fecprocessedbytes += 4;

	return true;
}

bool fec_decode(uint8_t* input, uint8_t* output)
{

}

#endif /* D7_PHY_USE_FEC */
