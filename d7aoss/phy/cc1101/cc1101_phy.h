/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@uantwerpen.be
 *  	glenn.ergeerts@uantwerpen.be
 *  	alexanderhoet@gmail.com
 */

#ifndef CC1101_PHY_H_
#define CC1101_PHY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "phy.h"
#include "cc1101_core.h"
#include "cc1101_registers.h"

#define RSSI_OFFSET 74
#define PATABLE_VAL 0xC4
#define PN9_BUFFER_SIZE 16
#define FEC_BUFFER_SIZE 32

typedef enum {
    Idle = 0,
    Receive = 1,
    Transmit = 2,
    FastTransmit = 3,
    Calibrate = 4,
    Settling = 5,
    RxOverflow = 6,
    TxOverflow = 7,
    Sleeping = 8
} RadioState;

typedef void (*InterruptHandler)(void);

typedef struct {
	uint8_t gdoSetting;
	GDOEdge edge;
	InterruptHandler handler;
} InterruptHandlerDescriptor;

void phy_init(void);
void phy_idle(void);
bool phy_tx(phy_tx_cfg_t* cfg);
bool phy_rx(phy_rx_cfg_t* cfg);
bool phy_is_rx_in_progress(void);
bool phy_is_tx_in_progress(void);
int16_t phy_get_rssi(uint8_t spectrum_id[2], uint8_t sync_word_class);

void end_of_packet_isr();
void rx_timeout_isr();
void rx_fifo_overflow_isr();
void rx_data_isr();
void tx_data_isr();
void rxtx_finish_isr();

bool phy_init_tx();
bool phy_set_tx(phy_tx_cfg_t* cfg);
bool phy_tx_data(phy_tx_cfg_t* cfg);

#ifdef __cplusplus
}
#endif

#endif /* CC1101_PHY_H_ */
