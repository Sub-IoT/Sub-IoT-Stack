/*
 * The PHY layer API
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include <stdint.h>

#include "phy.h"
#include "fec.h"

#ifdef D7_PHY_USE_FEC

const uint8_t fec_lut[16] = {0, 3, 1, 2, 3, 0, 2, 1 , 3 , 0, 2, 1, 0, 3, 1, 2};

uint8_t remainingbytes;
uint16_t fecremainingbytes;
uint8_t* inputbuffer;

uint16_t pn9;
uint8_t fecstate;

void fec_encode_init(uint8_t* input, uint8_t length)
{
	inputbuffer = input;
	remainingbytes = length;
	fecremainingbytes = ((length & 0xFE) + 2) << 1;

	pn9 = INITIAL_PN9;
}

uint8_t fec_encode(uint8_t* output, uint8_t length)
{
	uint8_t i;
	uint8_t pn9buffer[2];
	uint16_t fecbuffer[2];
	uint16_t output0;
	uint16_t output1;
	uint8_t outputlength;

	//Make sure the expected output length is a multiple of 4
	length &= 0xFC;

	//Reset the number of processed bytes
	outputlength = 0;

	while(fecremainingbytes > 0 && length > 0)
	{
		// Output bytes are generated 4 at a time, fec doubles the amount of bytes so the number of input bytes must be 2.
		for(i = 0; i < 2; i++)
		{
			// Get the next  byte from the input buffer if available and apply data whitening, otherwise append trellis terminator
			if(remainingbytes > 0) {
				pn9buffer[i] = *inputbuffer++;

				//Apply pn9 data whitening
				pn9buffer[i] = pn9buffer[i] ^ (uint8_t)pn9;

				pn9 |= ((pn9 << 9) & 0x1E00) ^ ((pn9 << 4) & 0x1E00);
				pn9 >>= 4;
				pn9 |= ((pn9 << 9) & 0x1E00) ^ ((pn9 << 4) & 0x1E00);
				pn9 >>= 4;

				remainingbytes--;
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
		*output++ = (uint8_t)(output0 >> 8);
		*output++ = (uint8_t)output0;
		*output++ = (uint8_t)(output1 >> 8);
		*output++ = (uint8_t)output1;

		fecremainingbytes -= 4;
		length -= 4;
		outputlength += 4;
	}

	return outputlength;
}

#endif /* D7_PHY_USE_FEC */
