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

void fec_init();
void fec_set_length(uint8_t length);
bool fec_encode(uint8_t* input, uint8_t* output);
bool fec_decode(uint8_t* input, uint8_t* output);

#ifdef __cplusplus
}
#endif

#endif /* FEC_H_ */
