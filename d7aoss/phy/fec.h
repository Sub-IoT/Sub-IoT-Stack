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

void conv_encode(uint8_t* input, uint8_t* output, uint16_t length);
void interleave_deinterleave(uint8_t* input, uint8_t* output, uint16_t length);


#ifdef __cplusplus
}
#endif

#endif /* FEC_H_ */
