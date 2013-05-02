/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#ifndef __LOG_H_
#define __LOG_H_

//#define LOG_DLL_ENABLED
//#define LOG_PHY_ENABLED
#define LOG_TRANS_ENABLED

#include "types.h"

#include "phy/phy.h"
#include "dll/dll.h"

#define LOG_TYPE_STRING 0x01
#define LOG_TYPE_DATA 0x02

#define LOG_TYPE_PHY_RX_RES 0x02
#define LOG_TYPE_PHY_RX_RES_SIZE 5

#define LOG_TYPE_DLL_RX_RES 0x03
#define LOG_TYPE_DLL_RX_RES_SIZE 2

void log_print_string(char* message);
void log_print_data(uint8_t* message, uint8_t length);

void log_phy_rx_res(phy_rx_data_t* res);

void log_dll_rx_res(dll_rx_res_t* res);

#endif /* __LOG_H_ */
