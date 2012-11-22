/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

// RAL implementation for TI CC430 (code mostly taken from TI reference code for CC430.)


#ifndef CC430_RAL_H_
#define CC430_RAL_H_

#include "../ral.h"
#include "cc430_registers.h"
#include "../../types.h"

// The CC430 implementation of the RAL interface
extern const struct ral_interface cc430_ral;

typedef enum {
    RadioStateNone,
    RadioStateTransmit,
    RadioStateTransmitData,
    RadioStateReceiveInit,
    RadioStateReceive,
    RadioStateReceiveDone
} RadioStateEnum;

// RF_Settings can be created using TI RF Studio
// Check CC430 User Guide for references of configuration settings, Table 25-19
typedef struct S_RF_SETTINGS {
    // 25.3.5.1
    unsigned char iocfg2;    // IOCFG2.GDO2_CFG output pin configuration
    unsigned char iofcg1;    // IOCFG1.GDO1_CFG output pin configuration
    unsigned char iofcg0;    // IOCFG0.GDO0_CFG output pin configuration
    // Page 713
    unsigned char fifothr;   // RXFIFO and TXFIFO thresholds.
    unsigned char sync1;     //  Sync word, high byte
    unsigned char sync0;     // Sync word, low byte
    unsigned char pktlen;    // Packet length.
    // Page 714
    unsigned char pktctrl1;  // Packet automation control.
    unsigned char pktctrl0;  // Packet automation control.
    unsigned char addr;      // Device address.
    unsigned char channr;    // Channel number.
    unsigned char fsctrl1;   // Frequency synthesizer control.
    unsigned char fsctrl0;   // Frequency synthesizer control.
    unsigned char freq2;     // Frequency control word, high byte.
    unsigned char freq1;     // Frequency control word, middle byte.
    unsigned char freq0;     // Frequency control word, low byte.
    unsigned char mdmcfg4;   // Modem configuration - Channel Bandwidth and symbol rate (exponent)
    unsigned char mdmcfg3;   // Modem configuration - Symbol rate (mantissa)
    unsigned char mdmcfg2;   // Modem configuration.
    unsigned char mdmcfg1;   // Modem configuration.
    unsigned char mdmcfg0;   // Modem configuration.
    unsigned char deviatn;   // Modem deviation setting (when FSK modulation is enabled).
    //unsigned char frend1;    // Front end RX configuration.
    //unsigned char frend0;    // Front end RX configuration.
    unsigned char mcsm2;    // Main Radio Control State Machine configuration.
    unsigned char mcsm1;     // Main Radio Control State Machine configuration.
    unsigned char mcsm0;     // Main Radio Control State Machine configuration.
    unsigned char foccfg;    // Frequency Offset Compensation Configuration.
    unsigned char bscfg;     // Bit synchronization Configuration.
    unsigned char agcctrl2;  // AGC control.
    unsigned char agcctrl1;  // AGC control.
    unsigned char agcctrl0;  // AGC control.
    unsigned char worevt1;   // High byte Event0 timeout
    unsigned char worevt0;   // Low byte Event0 timeout
    unsigned char worctl;    // Wake On Radio control
    unsigned char frend1;    // Front end RX configuration
    unsigned char frend0;    // Front end TX configuration
    unsigned char fscal3;    // Frequency synthesizer calibration.
    unsigned char fscal2;    // Frequency synthesizer calibration.
    unsigned char fscal1;    // Frequency synthesizer calibration.
    unsigned char fscal0;    // Frequency synthesizer calibration.
    //unsigned char fstest;    // Frequency synthesizer calibration control
    // ptest
    // agctest
    //unsigned char test2;     // Various test settings.
    //unsigned char test1;     // Various test settings.
    //unsigned char test0;     // Various test settings.
} RF_SETTINGS;


void cc430_ral_init(void);
void cc430_ral_set_tx_callback(ral_tx_callback_t callback);
void cc430_ral_set_rx_callback(ral_rx_callback_t callback);
void cc430_ral_send(unsigned char* buffer);
void cc430_ral_rx_start(ral_rx_cfg_t* cfg);
void cc430_ral_rx_stop();
bool cc430_ral_is_rx_in_progress();

#endif /*CC430_RAL_H_*/
