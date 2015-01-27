/*
 *  This program does a continuous TX on a specific channel
 *  Authors:
 *      glenn.ergeerts@artesis.be
 */


#include <string.h>
#include <stdint.h>

#include <d7aoss.h>

#include <phy/phy.h>

#include <hal/system.h>
#include <hal/leds.h>

#include <framework/log.h>

#include "../../d7aoss/phy/cc1101/cc1101_constants.h"
#include "../../d7aoss/phy/cc1101/cc1101_phy.h"

// Packet length mode = Infinite packet length mode
// PA ramping = false
// TX power = 0
// Manchester enable = false
// Sync word qualifier mode = No preamble/sync
// Modulation format = GFSK
// Data rate = 55.542
// Preamble count = 4
// Packet length = 255
// Whitening = true
// CRC enable = false
// Channel number = 0
// Carrier frequency = 433.159698
// Deviation = 50.781250
// RX filter BW = 58.035714
// Address config = No address check
// CRC autoflush = false
// Channel spacing = 199.951172
// Data format = Random mode
// Modulated = true
// Base frequency = 433.159698
// Device address = 0
// Rf settings for CC1101
RF_SETTINGS rfSettings = {
    0x0B,  // IOCFG2              GDO2 Output Pin Configuration
    0x2E,  // IOCFG1              GDO1 Output Pin Configuration
    0x0C,  // IOCFG0              GDO0 Output Pin Configuration
    RADIO_FIFOTHR_FIFO_THR_61_4,  // FIFOTHR             RX FIFO and TX FIFO Thresholds
    	// TODO ADC_RETENTION and CLOSE_IN_RX ?
    RADIO_SYNC1,  // SYNC1               Sync Word, High Byte
    RADIO_SYNC0,  // SYNC0               Sync Word, Low Byte
    RADIO_PKTLEN,  // PKTLEN              Packet Length
    RADIO_PKTCTRL1_PQT(3) | RADIO_PKTCTRL1_APPEND_STATUS,  // PKTCTRL1            Packet Automation Control
    RADIO_PKTCTRL0_WHITE_DATA,  // PKTCTRL0            Packet Automation Control
    RADIO_ADDR,  // ADDR                Device Address
    RADIO_CHAN,  // CHANNR              Channel Number
    RADIO_FREQ_IF,  // FSCTRL1             Frequency Synthesizer Control
    RADIO_FREQOFF,  // FSCTRL0             Frequency Synthesizer Control
    RADIO_FREQ2,  // FREQ2               Frequency Control Word, High Byte
    RADIO_FREQ1,  // FREQ1               Frequency Control Word, Middle Byte
    RADIO_FREQ0,  // FREQ0               Frequency Control Word, Low Byte
    RADIO_MDMCFG4_CHANBW_E(1) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(11),   // MDMCFG4   Modem configuration.
    RADIO_MDMCFG3_DRATE_M(24),  // MDMCFG3             Modem Configuration
    RADIO_MDMCFG2_DEM_DCFILT_ON | RADIO_MDMCFG2_MOD_FORMAT_GFSK | RADIO_MDMCFG2_SYNC_MODE_16in16CS,  // MDMCFG2             Modem Configuration
    RADIO_MDMCFG1_NUM_PREAMBLE_4B | RADIO_MDMCFG1_CHANSPC_E(2),  // MDMCFG1             Modem Configuration
    RADIO_MDMCFG0_CHANSPC_M(16),  // MDMCFG0             Modem Configuration
    RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0),  // DEVIATN             Modem Deviation Setting
    RADIO_MCSM2_RX_TIME(7),  // MCSM2               Main Radio Control State Machine Configuration
    RADIO_MCSM1_CCA_RSSILOWRX | RADIO_MCSM1_RXOFF_MODE_IDLE | RADIO_MCSM1_TXOFF_MODE_IDLE,  // MCSM1               Main Radio Control State Machine Configuration
    RADIO_MCSM0_FS_AUTOCAL_FROMIDLE,  // MCSM0               Main Radio Control State Machine Configuration
    RADIO_FOCCFG_FOC_PRE_K_3K | RADIO_FOCCFG_FOC_POST_K_HALFK | RADIO_FOCCFG_FOC_LIMIT_4THBW,  // FOCCFG              Frequency Offset Compensation Configuration
    RADIO_BSCFG_BS_PRE_KI_2KI | RADIO_BSCFG_BS_PRE_KP_3KP | RADIO_BSCFG_BS_POST_KI_1KP | RADIO_BSCFG_BS_POST_KP_1KP | RADIO_BSCFG_BS_LIMIT_0,  // BSCFG               Bit Synchronization Configuration
    RADIO_AGCCTRL2_MAX_DVGA_GAIN_ALL | RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB0 | RADIO_AGCCTRL2_MAX_MAGN_TARGET_33,  // AGCCTRL2            AGC Control
    RADIO_AGCCTRL1_AGC_LNA_PRIORITY | RADIO_AGCCTRL1_CS_REL_THR_DISABLED | RADIO_AGCCTRL1_CS_ABS_THR_FLAT,  // AGCCTRL1            AGC Control
    RADIO_AGCCTRL0_HYST_LEVEL_MED | RADIO_AGCCTRL0_WAIT_ITME_16 | RADIO_AGCCTRL0_AGC_FREEZE_NORMAL | RADIO_AGCCTRL0_FILTER_LENGTH_16,  // AGCCTRL0            AGC Control
    RADIO_WOREVT1_EVENT0_HI(128),  // WOREVT1             High Byte Event0 Timeout
    RADIO_WOREVT0_EVENT0_LO(0),  // WOREVT0             Low Byte Event0 Timeout
    RADIO_WORCTRL_ALCK_PD,  // WORCTRL             Wake On Radio Control
    RADIO_FREND1_LNA_CURRENT(1) | RADIO_FREND1_LNA2MIX_CURRENT(1) | RADIO_FREND1_LODIV_BUF_CURRENT_RX(1) | RADIO_FREND1_MIX_CURRENT(2),  // FREND1              Front End RX Configuration
    RADIO_FREND0_LODIV_BUF_CURRENT_TX(1) | RADIO_FREND0_PA_POWER(0),  // FREND0              Front End TX Configuration
    RADIO_FSCAL3_HI(3) | RADIO_FSCAL3_CHP_CURR_CAL_EN(2) | RADIO_FSCAL3_LO(10),  // FSCAL3              Frequency Synthesizer Calibration
    RADIO_FSCAL2_FSCAL2(10),  // FSCAL2              Frequency Synthesizer Calibration
    RADIO_FSCAL1(0),   // FSCAL1              Frequency Synthesizer Calibration
    RADIO_FSCAL0(31)  // FSCAL0              Frequency Synthesizer Calibration
};

uint8_t buffer[256];

void main()
{
	system_init(buffer, sizeof(buffer), buffer, sizeof(buffer));
	spi_init();

	int p = ReadSingleReg(PARTNUM);
	log_print_string("PARTNUM 0x%x", p);
	p = ReadSingleReg(VERSION);
	log_print_string("VERSION 0x%x", p);
	log_print_string("started");

	ResetRadioCore();
	WriteRfSettings(&rfSettings);
	Strobe(RF_SFTX);

	//uint8_t spectrum_id[2] = { 4, 0 };
	//phy_translate_and_set_settings(spectrum_id, 0);

	Strobe(RF_STX);

	while(1);
}
