/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __LOG_H_
#define __LOG_H_

#include "types.h"

#include "phy/phy.h"

void log_print_string(char* message);

void log_packet(u8* packet);

void log_phy_rx_res(phy_rx_res_t* res);

#endif /* __LOG_H_ */
