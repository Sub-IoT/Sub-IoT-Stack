/*
 * The high level API to be used by applications which use the dash7 stack
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 *
 */

#ifndef D7STACK_H_
#define D7STACK_H_

// configuration
// TODO get from configure step or similar

#define DEBUG

#define RAL_IMPLEMENTATION cc430_ral

#if RAL_IMPLEMENTATION == stub_ral
#	include "ral/stub/stub_ral.h"
#elif RAL_IMPLEMENTATION == cc430_ral
#	include "ral/cc430/cc430_ral.h"
#endif

// the RAL implementation to use
extern const struct ral_interface RAL_IMPLEMENTATION;

#endif /* D7STACK_H_ */
