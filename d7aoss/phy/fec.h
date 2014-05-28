/*! \file fec.h
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * \author glenn.ergeerts@uantwerpen.be
 * \author maarten.weyn@uantwerpen.be
 *	\author alexanderhoet@gmail.com
 *
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
	uint16_t path;
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
