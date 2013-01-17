/*
 *  Created on: Jan 11, 2013
 *  Authors:
 *  	alexanderhoet@gmail.com
 */

#ifndef FEC_H_
#define FEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
	uint8_t metric;
	uint8_t populated;
	uint8_t path[4];
	uint8_t pathmetric[4];
} VITERBISTATE;

typedef struct {
	VITERBISTATE states1[8];
	VITERBISTATE states2[8];
	VITERBISTATE* old;
	VITERBISTATE* new;
	uint8_t pathsize;
} CONVDECODESTATE;

void conv_encode(uint8_t* input, uint8_t* output, uint16_t length, uint8_t* state);
void conv_decode_init(CONVDECODESTATE* state);
void conv_decode(uint8_t* input, uint8_t* output, uint16_t length, CONVDECODESTATE* state);
void interleave_deinterleave(uint8_t* input, uint8_t* output, uint16_t length);


#ifdef __cplusplus
}
#endif

#endif /* FEC_H_ */
