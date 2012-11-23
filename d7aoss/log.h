/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __LOG_H_
#define __LOG_H_

#include "types.h"

void log_print_string(char* message, u8 length);

void log_packet(u8* packet);

#endif /* __LOG_H_ */
