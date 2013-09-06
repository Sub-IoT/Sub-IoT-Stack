/*
 * stub_phy.h
 *
 *  Created on: Sep 6, 2013
 *      Author: Fritz
 */

#ifndef STUB_PHY_H_
#define STUB_PHY_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>

#include "../../phy/phy.h"

/*
 * Phy implementation function prototypes
 */
void phy_init(void);
void phy_idle(void);
bool phy_tx(phy_tx_cfg_t* cfg);
bool phy_rx(phy_rx_cfg_t* cfg);
bool phy_is_rx_in_progress(void);
bool phy_is_tx_in_progress(void);
int16_t phy_get_rssi(uint8_t spectrum_id, uint8_t sync_word_class);

/*
 * Interrupt function prototypes
 */
void no_interrupt_isr();
void end_of_packet_isr();
void rx_timeout_isr();
void rx_fifo_overflow_isr();
void rx_data_isr();
void tx_data_isr();
void rxtx_finish_isr();

/*
 * Local function prototypes
 */
void set_channel(uint8_t channel_center_freq_index, uint8_t channel_bandwidth_index);
void set_sync_word(uint16_t sync_word);
void set_preamble_size(uint8_t preamble_size);
void set_data_whitening(bool  white_data);
void set_length_infinite(bool infinite);
void set_timeout(uint16_t timeout);
void set_eirp(int8_t eirp);
int16_t calculate_rssi(int8_t rssi_raw);

void dissable_autocalibration();
void enable_autocalibration();
void manual_calibration();

bool phy_init_tx();
bool phy_set_tx(phy_tx_cfg_t* cfg);
bool phy_tx_data(phy_tx_cfg_t* cfg);

#ifdef __cplusplus
}
#endif

#endif /* STUB_PHY_H_ */
