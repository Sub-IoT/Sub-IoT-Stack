/*
 *  Created on: Jan 11, 2013
 *  Authors:
 *  	alexanderhoet@gmail.com
 */

#ifndef PN9_H
#define PN9_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define INITIAL_PN9	0x01FF

void pn9_init(uint16_t* pn9);
void pn9_encode_decode(uint8_t* input, uint8_t* output, uint16_t length, uint16_t* pn9);

#ifdef __cplusplus
}
#endif

#endif /* PN9_H */
