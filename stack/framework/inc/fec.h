/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
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
 * \author maarten.weyn@uantwerpen.be
 * \author glenn.ergeerts@uantwerpen.be
 * \author alexanderhoet@gmail.com
 *
 */

/*! \file fec.h
 * \addtogroup fec
 * \ingroup framework
 * @{
 * \brief Implements the FEC Encoder used for error correction
 *
 * FEC makes sense when operating near the sensitivity limit.
 * This is a technique used for controlling errors in data transmission over unreliable or noisy communication channels.
 * FEC is accomplished by adding redundancy to the transmitted information.
 *
 * Channel errors tend to occur in bursts. FEC is associated with an interleaver. Interleaving ameliorates this problem by shuffling source symbols across several code words, thereby creating a more uniform distribution of errors.
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

//void print_array(uint8_t* buffer, uint8_t length);

uint16_t fec_encode(uint8_t *data, uint16_t nbytes);
uint16_t fec_decode_packet(uint8_t* data, uint16_t packet_length, uint16_t output_length);
uint16_t fec_calculated_decoded_length(uint16_t packet_length);

#ifdef __cplusplus
}
#endif

#endif /* FEC_H_ */

/** @}*/
