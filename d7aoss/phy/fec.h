/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 *     	glenn.ergeerts@uantwerpen.be
 *     	maarten.weyn@uantwerpen.be
 *		alexanderhoet@gmail.com
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
