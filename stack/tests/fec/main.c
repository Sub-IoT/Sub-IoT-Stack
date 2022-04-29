/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "fec.h"

#define BINARY 0


const char *byte_to_binary(uint8_t x)
{
    static char b[9];
    b[0] = '\0';

    uint8_t z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

unsigned char Partab[] = { 0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0
,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1
,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,};

#define TRELLIS_TERMINATOR 0x0B
const static uint8_t fec_lut[16] = {0, 3, 1, 2, 3, 0, 2, 1, 3, 0, 2, 1, 0, 3, 1, 2};



void test_interleaver()
{
	uint8_t fecbuffer[4] = {0xFF,0x00,0xAB,0x00};
	uint8_t buffer[4]= {0,0,0,0};
	uint8_t buffer2[4]= {0,0,0,0};
	uint8_t *output = buffer;
	uint8_t *input = buffer;

	*output++ = ((fecbuffer[0] & 0x03)) |\
				((fecbuffer[1] & 0x03) << 2) |\
				((fecbuffer[2] & 0x03) << 4) |\
				((fecbuffer[3] & 0x03) << 6);
	*output++ = (((fecbuffer[0] >> 2) & 0x03)) |\
					(((fecbuffer[1] >> 2) & 0x03) << 2) |\
					(((fecbuffer[2] >> 2) & 0x03) << 4) |\
					(((fecbuffer[3] >> 2) & 0x03) << 6);
	*output++ = (((fecbuffer[0] >> 4) & 0x03)) |\
					(((fecbuffer[1] >> 4) & 0x03) << 2) |\
					(((fecbuffer[2] >> 4) & 0x03) << 4) |\
					(((fecbuffer[3] >> 4) & 0x03) << 6);
	*output++ = (((fecbuffer[0] >> 6) & 0x03)) |\
					(((fecbuffer[1] >> 6) & 0x03) << 2) |\
					(((fecbuffer[2] >> 6) & 0x03) << 4) |\
					(((fecbuffer[3] >> 6) & 0x03) << 6);

	printf("Input : "); print_array((uint8_t*) fecbuffer, 4, BINARY); printf("\n");
	printf("Interl: "); print_array((uint8_t*) buffer, 4, BINARY); printf("\n");

	fecbuffer[0] = ((input[0] & 0x03)) |\
				((input[1] & 0x03) << 2) |\
				((input[2] & 0x03) << 4) |\
				((input[3] & 0x03) << 6);
	fecbuffer[1] = (((input[0] >> 2) & 0x03)) |\
					(((input[1] >> 2) & 0x03) << 2) |\
					(((input[2] >> 2) & 0x03) << 4) |\
					(((input[3] >> 2) & 0x03) << 6);
	fecbuffer[2] = (((input[0] >> 4) & 0x03)) |\
					(((input[1] >> 4) & 0x03) << 2) |\
					(((input[2] >> 4) & 0x03) << 4) |\
					(((input[3] >> 4) & 0x03) << 6);
	fecbuffer[3] = (((input[0] >> 6) & 0x03)) |\
					(((input[1] >> 6) & 0x03) << 2) |\
					(((input[2] >> 6) & 0x03) << 4) |\
					(((input[3] >> 6) & 0x03) << 6);


	printf("Deint : "); print_array((uint8_t*) fecbuffer, 4, BINARY); printf("\n");


}

int main(int argc, char *argv[])
{
	//test_interleaver();
	uint8_t input[] = {0x19,0x00,0x80,0x00,0x0B,0x57,0x00,0x00,0x07,0x8C,0x63,0x20,0x00,0x0B,0x57,0x00,0x00,0x07,0x8F,0x52,0x41,0x07,0x00,0x00,0xD0,0x81};
	uint8_t input_length = sizeof(input);
	uint8_t encoded[255];
	uint8_t decoded[255];
	uint8_t errors[255];

	memset(errors, 0, 255);


	printf("Input: %d ", input_length);
	print_array(input, input_length, BINARY);
	printf("\n");

	memcpy(encoded, input, input_length);

	uint16_t lenght_encoded = fec_encode(encoded, input_length);

	printf("Encoded: %d ", lenght_encoded);
	print_array(encoded, lenght_encoded, BINARY);
	printf("\n");

	memcpy(decoded, encoded, lenght_encoded);


	uint8_t length_decoded =  fec_decode_packet(decoded, lenght_encoded, 255);

	printf("Decoded: %d ", length_decoded);
	print_array(decoded, length_decoded, BINARY);
	printf("\n");

	int nr_errors = 1;
	int notrecovered = 0;
	srand(time(NULL));

	if(memcmp(input, decoded, input_length) == 0)
    {
        printf("Input was decoded successfully\n");
    }
    else
    {
        printf("error\n");
    }

//	while (notrecovered < 1)
//	{
//		int r = rand() % (lenght_encoded * 8);
//		//printf("Rand: %d\n", r);
//
//		encoded[r/8] ^= 1 << r % 8;
//		errors[r/8] ^= 1 << r % 8;
//		printf("Errors %d - BER %d%%\n", nr_errors, (nr_errors * 100) / (lenght_encoded * 8));
//		printf("Error: ");
//		print_array(errors, lenght_encoded);
//		printf("\n");
//		printf("Enc. error: ");
//		print_array(encoded, lenght_encoded);
//		printf("\n");
//
//		length_decoded =  fec_decode_packet(encoded, lenght_encoded, decoded, 255);
//
//		printf("Decoded: ");
//		print_array(decoded, length_decoded);
//		printf("\n");
//
//		if (memcmp(input, decoded, input_length) == 0)
//		{
//			printf("ok\n");
//		}
//		else
//		{
//			printf("error\n");
//			notrecovered++;
//		}
//
//		nr_errors++;
//	}


    return 0;
}
