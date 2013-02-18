/*
 * cc430_phy.h
 *
 *  Created on: 13-feb.-2013
 *      Author: Miesalex
 */

#ifndef CC430_PHY_H_
#define CC430_PHY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>

#include "../phy.h"
#include "rf1a.h"
#include "cc430_registers.h"

/*
 * Defines
 */
#define RSSI_OFFSET 74;
#define PATABLE_VAL 0xC4
#define CCA_RSSI_THRESHOLD -86 // TODO configurable

/*
 * Type definitions
 */

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

/*
 * Interrupt handler function pointer
 */
typedef void (*InterruptHandler)(void);

/*
 * Phy implementation function prototypes
 */
void phy_init(void);
bool phy_tx(phy_tx_cfg* cfg);
bool phy_rx_start(phy_rx_cfg* cfg);
void phy_rx_stop(void);
bool phy_read(phy_rx_data* data);
bool phy_is_rx_in_progress(void);
bool phy_is_tx_in_progress(void);
bool phy_cca(void);

/*
 * Interrupt function prototypes
 */
void no_interrupt_isr();
void end_of_packet_isr();
void rx_data_isr();
void tx_data_isr();
void rxtx_finish_isr();

/*
 * Local function prototypes
 */
RadioState get_radiostate(void);
void set_channel(uint8_t channel_center_freq_index, uint8_t channel_bandwidth_index);
void set_sync_word(uint16_t sync_word);
void set_preamble_size(uint8_t preamble_size);
void set_data_whitening(bool  white_data);
void set_length_infinite(bool infinite);
void set_timeout(uint16_t timeout);
int16_t calculate_rssi(int8_t rssi_raw);

#ifdef __cplusplus
}
#endif

#endif /* CC430_PHY_H_ */
