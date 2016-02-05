/*! \file fec.h
 *
 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \author glenn.ergeerts@uantwerpen.be
 * \author maarten.weyn@uantwerpen.be
 * \author alexanderhoet@gmail.com
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
