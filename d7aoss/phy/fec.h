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

void conv_init(uint8_t* state);
void conv_encode(uint8_t* input, uint16_t* output, uint8_t* state);
void interleave_deinterleave(uint16_t* input, uint16_t* output);


#ifdef __cplusplus
}
#endif

#endif /* FEC_H_ */
