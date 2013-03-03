/*
 * The PHY layer API
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#ifndef FEC_H_
#define FEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t cost;
	uint32_t path;
} VITERBIPATH;

typedef struct {
	uint8_t path_size;
	VITERBIPATH* old;
	VITERBIPATH* new;
	VITERBIPATH states1[8];
	VITERBIPATH states2[8];
} VITERBISTATE;


void fec_init_encode(uint8_t* input);
void fec_init_decode(uint8_t* output);
void fec_set_length(uint8_t length);
bool fec_encode(uint8_t* output);
bool fec_decode(uint8_t* input);

#ifdef __cplusplus
}
#endif

#endif /* FEC_H_ */
