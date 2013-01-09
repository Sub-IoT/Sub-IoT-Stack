/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#ifndef CC430_RAL_H
#define CC430_RAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "../ral.h"

#define RSSI_OFFSET 74;
#define PATABLE_VAL 0xC4

//The CC430 implementation of the RAL interface
extern const struct ral_interface cc430_ral;

//Interrupt handler function pointer
typedef void (*InterruptHandler)(void);

//Radio state enum
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

//Interface function prototypes
void cc430_ral_init(void);
void cc430_ral_tx(ral_tx_cfg* data);
void cc430_ral_rx_start(ral_rx_cfg* cfg);
void cc430_ral_rx_stop(void);
bool cc430_ral_read(ral_rx_data* data);
bool cc430_ral_is_rx_in_progress(void);
bool cc430_ral_is_tx_in_progress(void);

#ifdef __cplusplus
}
#endif

#endif /* CC430_RAL_H */
