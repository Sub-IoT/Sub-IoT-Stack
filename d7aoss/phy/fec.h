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

void fec_encode_init(uint8_t* input, uint8_t length);
uint8_t fec_encode(uint8_t* output, uint8_t length);


#ifdef __cplusplus
}
#endif

#endif /* FEC_H_ */
