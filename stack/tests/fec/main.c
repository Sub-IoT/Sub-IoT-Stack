#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "fec.h"


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

/* Viterbi decoder */
//int
//fec_decode(
//unsigned long *metric,  /* Final path metric (returned value) */
//unsigned char *data,    /* Decoded output data */
//unsigned char *symbols, /* Raw deinterleaved input symbols */
//unsigned int nbits,     /* Number of output bits */
//){
//        unsigned int bitcnt = 0;
//        int beststate,i;
//        long cmetric[8],nmetric[8];
//        register int m0,m1;
//
//        /* This next line is arguably illegal C, but it works on
//         * GCC and it avoids having to reference this heavily used
//         * array through a pointer. If you can't compile this, use malloc.
//         */
//        unsigned long paths[nbits*1*sizeof(unsigned long)];
//        unsigned long *pp;
//        unsigned long dec;
//        int mets[4];
//
//        pp = paths;
//        /* Initialize starting metrics to prefer 0 state */
//        cmetric[0] = 0;
//        for(i=1;i<8;i++)
//                cmetric[i] = -999999;
//                for(;;){
////                mets[0] = mettab[0][symbols[0]] + mettab[0][symbols[1]];
////                mets[1] = mettab[0][symbols[0]] + mettab[1][symbols[1]];
////                mets[2] = mettab[1][symbols[0]] + mettab[0][symbols[1]];
////                mets[3] = mettab[1][symbols[0]] + mettab[1][symbols[1]];
//                mets[0] = ;//00
//                symbols += 2;
//                dec = 0;
//                /* state 0, symbols 00 */
//                m0 = cmetric[0] + mets[0];
//                m1 = cmetric[4] + mets[3];
//                nmetric[0] = m0;
//                if(m1 > m0){
//                        nmetric[0] = m1;
//                        dec |= 0x1;
//                }
//                m0 -= (mets[0] - mets[3]);
//                m1 += (mets[0] - mets[3]);
//                nmetric[1] = m0;
//                if(m1 > m0){
//                        nmetric[1] = m1;
//                        dec |= 0x2;
//                }
//                /* state 1, symbols 01 */
//                m0 = cmetric[1] + mets[2];
//                m1 = cmetric[5] + mets[1];
//                nmetric[2] = m0;
//                if(m1 > m0){
//                        nmetric[2] = m1;
//                        dec |= 0x4;
//                }
//                m0 -= (mets[2] - mets[1]);
//                m1 += (mets[2] - mets[1]);
//                nmetric[3] = m0;
//                if(m1 > m0){
//                        nmetric[3] = m1;
//                        dec |= 0x8;
//                }
//                /* state 2, symbols 11 */
//                m0 = cmetric[2] + mets[3];
//                m1 = cmetric[6] + mets[0];
//                nmetric[4] = m0;
//				                if(m1 > m0){
//                        nmetric[4] = m1;
//                        dec |= 0x10;
//                }
//                m0 -= (mets[3] - mets[0]);
//                m1 += (mets[3] - mets[0]);
//                nmetric[5] = m0;
//                if(m1 > m0){
//                        nmetric[5] = m1;
//                        dec |= 0x20;
//                }
//                /* state 3, symbols 10 */
//                m0 = cmetric[3] + mets[1];
//                m1 = cmetric[7] + mets[2];
//                nmetric[6] = m0;
//                if(m1 > m0){
//                        nmetric[6] = m1;
//                        dec |= 0x40;
//                }
//                m0 -= (mets[1] - mets[2]);
//                m1 += (mets[1] - mets[2]);
//                nmetric[7] = m0;
//                if(m1 > m0){
//                        nmetric[7] = m1;
//                        dec |= 0x80;
//                }
//                *pp++ = dec;
//                if(++bitcnt == nbits){
//                        beststate = 0;
//                        *metric = nmetric[beststate];
//                        break;
//                }
//                mets[0] = mettab[0][symbols[0]] + mettab[0][symbols[1]];
//                mets[1] = mettab[0][symbols[0]] + mettab[1][symbols[1]];
//                mets[2] = mettab[1][symbols[0]] + mettab[0][symbols[1]];
//                mets[3] = mettab[1][symbols[0]] + mettab[1][symbols[1]];
//                symbols += 2;
//                dec = 0;                 /* state 0, symbols 00 */
//                m0 = nmetric[0] + mets[0];
//                m1 = nmetric[4] + mets[3];
//                cmetric[0] = m0;
//                if(m1 > m0){
//                        cmetric[0] = m1;
//                        dec |= 0x1;
//                }
//                m0 -= (mets[0] - mets[3]);
//                m1 += (mets[0] - mets[3]);
//                cmetric[1] = m0;
//                if(m1 > m0){
//                        cmetric[1] = m1;
//                        dec |= 0x2;
//                }
//                /* state 1, symbols 01 */
//                m0 = nmetric[1] + mets[2];
//                m1 = nmetric[5] + mets[1];
//                cmetric[2] = m0;
//                if(m1 > m0){
//                        cmetric[2] = m1;
//                        dec |= 0x4;
//                }
//                m0 -= (mets[2] - mets[1]);
//                m1 += (mets[2] - mets[1]);
//                cmetric[3] = m0;
//                if(m1 > m0){
//                        cmetric[3] = m1;
//                        dec |= 0x8;
//                }
//                /* state 2, symbols 11 */
//                m0 = nmetric[2] + mets[3];
//                m1 = nmetric[6] + mets[0];
//                cmetric[4] = m0;
//                if(m1 > m0){
//                        cmetric[4] = m1;
//                        dec |= 0x10;
//                }
//                m0 -= (mets[3] - mets[0]);
//                m1 += (mets[3] - mets[0]);
//                cmetric[5] = m0;
//               if(m1 > m0){
//                        cmetric[4] = m1;
//                        dec |= 0x10;
//                }
//                m0 -= (mets[3] - mets[0]);
//                m1 += (mets[3] - mets[0]);
//                cmetric[5] = m0;
//                if(m1 > m0){
//                        cmetric[5] = m1;
//                        dec |= 0x20;
//                }
//                /* state 3, symbols 10 */
//                m0 = nmetric[3] + mets[1];
//                m1 = nmetric[7] + mets[2];
//                cmetric[6] = m0;
//                if(m1 > m0){
//                        cmetric[6] = m1;
//                        dec |= 0x40;
//                }
//                m0 -= (mets[1] - mets[2]);
//                m1 += (mets[1] - mets[2]);
//                cmetric[7] = m0;
//                if(m1 > m0){
//                        cmetric[7] = m1;
//                        dec |= 0x80;
//                }
//                *pp++ = dec;
//                if(++bitcnt == nbits){
//                        beststate = 0;
//                        *metric = cmetric[beststate];
//                        break;
//                }
//        }
//        pp -= 1;
//        pp -= 1;
//        /* Chain back from terminal state to produce decoded data */
//        memset(data,0,nbits/8);
//        for(i=nbits-4;i >= 0;i--){
//                if(pp[beststate >> 5] & (1 << (beststate & 31))){
//                        beststate |= 8; /* 2^(K-1) */
//                        data[i>>3] |= 0x80 >> (i&7);
//                }
//                beststate >>= 1;
//                pp -= 1;
//        }
//        return 0;
//}



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

	printf("Input : "); print_array((uint8_t*) fecbuffer, 4); printf("\n");
	printf("Interl: "); print_array((uint8_t*) buffer, 4); printf("\n");

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


	printf("Deint : "); print_array((uint8_t*) fecbuffer, 4); printf("\n");


}

int main(int argc, char *argv[])
{
	test_interleaver();
	uint8_t input[] = {0xAB, 0x02, 0x03, 0x04};
	uint8_t input_length = sizeof(input);
	uint8_t encoded[255];
	uint8_t decoded[255];
	uint8_t errors[255];

	memset(errors, 0, 255);


	printf("Input: ");
	print_array(input, input_length);
	printf("\n");

	uint16_t lenght_encoded = fec_encode(encoded, input, input_length);

	printf("Encoded: ");
	print_array(encoded, lenght_encoded);
	printf("\n");

	uint8_t length_decoded =  fec_decode_packet(encoded, lenght_encoded, decoded, 255);

	printf("Decoded: ");
	print_array(decoded, length_decoded);
	printf("\n");

	int nr_errors = 1;
	int notrecovered = 0;
	srand(time(NULL));

	while (notrecovered < 1)
	{
		int r = rand() % (lenght_encoded * 8);
		//printf("Rand: %d\n", r);

		encoded[r/8] ^= 1 << r % 8;
		errors[r/8] ^= 1 << r % 8;
		printf("Errors %d - BER %d%%\n", nr_errors, (nr_errors * 100) / (lenght_encoded * 8));
		printf("Error: ");
		print_array(errors, lenght_encoded);
		printf("\n");
		printf("Enc. error: ");
		print_array(encoded, lenght_encoded);
		printf("\n");

		length_decoded =  fec_decode_packet(encoded, lenght_encoded, decoded, 255);

		printf("Decoded: ");
		print_array(decoded, length_decoded);
		printf("\n");

		if (memcmp(input, decoded, input_length) == 0)
		{
			printf("ok\n");
		}
		else
		{
			printf("error\n");
			notrecovered++;
		}

		nr_errors++;
	}


    return 0;
}
