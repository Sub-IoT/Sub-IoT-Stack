/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
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

/*
 * Defines
 */
#define RSSI_OFFSET 74
#define PATABLE_VAL 0xC4
#define PN9_BUFFER_SIZE 16
#define FEC_BUFFER_SIZE 32

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

typedef struct {
	uint8_t gdoSetting;
	GDOEdge edge;
	InterruptHandler handler;
} InterruptHandlerDescriptor;
/*
 * Phy implementation function prototypes
 */
void phy_init(void);
void phy_idle(void);
bool phy_tx(phy_tx_cfg_t* cfg);
bool phy_rx(phy_rx_cfg_t* cfg);
bool phy_is_rx_in_progress(void);
bool phy_is_tx_in_progress(void);
int16_t phy_get_rssi(uint8_t spectrum_id[2], uint8_t sync_word_class);


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
RadioState get_radiostate(void);
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

#endif /* CC1101_PHY_H_ */
