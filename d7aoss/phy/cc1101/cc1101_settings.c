#include "cc1101_core.h"
#include "cc1101_registers.h"

//RF settings
 RF_SETTINGS rfSettings = {
	RADIO_GDO2_VALUE,   			// IOCFG2    GDO2 output pin configuration.
	RADIO_GDO1_VALUE,    			// IOCFG1    GDO1 output pin configuration.
    RADIO_GDO0_VALUE,   			// IOCFG0    GDO0 output pin configuration.
    RADIO_FIFOTHR_FIFO_THR_61_4,   	// FIFOTHR   RXFIFO and TXFIFO thresholds.
    RADIO_SYNC1,     				// SYNC1	 Sync word, high byte
	RADIO_SYNC0,     				// SYNC0	 Sync word, low byte
    RADIO_PKTLEN,    				// PKTLEN    Packet length.
    RADIO_PKTCTRL1_PQT(3) | RADIO_PKTCTRL1_APPEND_STATUS,   // PKTCTRL1  Packet automation control.
    RADIO_PKTCTRL0_WHITE_DATA,      // PKTCTRL0  Packet automation control.
    RADIO_ADDR,   					// ADDR      Device address.
    RADIO_CHAN,   					// CHANNR    Channel number.
    RADIO_FREQ_IF,   				// FSCTRL1   Frequency synthesizer control.
    RADIO_FREQOFF,   				// FSCTRL0   Frequency synthesizer control.
	RADIO_FREQ2,   					// FREQ2     Frequency control word, high byte.
	RADIO_FREQ1,   					// FREQ1     Frequency control word, middle byte.
	RADIO_FREQ0,   					// FREQ0     Frequency control word, low byte.
	RADIO_MDMCFG4_CHANBW_E(1) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(11),   // MDMCFG4   Modem configuration.
	RADIO_MDMCFG3_DRATE_M(24),   	// MDMCFG3   Modem configuration.
	RADIO_MDMCFG2_DEM_DCFILT_ON | RADIO_MDMCFG2_MOD_FORMAT_GFSK | RADIO_MDMCFG2_SYNC_MODE_16in16CS,   // MDMCFG2   Modem configuration.
	RADIO_MDMCFG1_NUM_PREAMBLE_4B | RADIO_MDMCFG1_CHANSPC_E(2),   // MDMCFG1   Modem configuration.
	RADIO_MDMCFG0_CHANSPC_M(16),   	// MDMCFG0   Modem configuration.
	RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0),   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
    RADIO_MCSM2_RX_TIME(7),			// MCSM2		 Main Radio Control State Machine configuration.
    RADIO_MCSM1_CCA_RSSILOWRX | RADIO_MCSM1_RXOFF_MODE_IDLE | RADIO_MCSM1_TXOFF_MODE_IDLE,	// MCSM1 Main Radio Control State Machine configuration.
    RADIO_MCSM0_FS_AUTOCAL_FROMIDLE,// MCSM0     Main Radio Control State Machine configuration.
    RADIO_FOCCFG_FOC_PRE_K_3K | RADIO_FOCCFG_FOC_POST_K_HALFK | RADIO_FOCCFG_FOC_LIMIT_4THBW,   // FOCCFG    Frequency Offset Compensation Configuration.
    RADIO_BSCFG_BS_PRE_KI_2KI | RADIO_BSCFG_BS_PRE_KP_3KP | RADIO_BSCFG_BS_POST_KI_1KP | RADIO_BSCFG_BS_POST_KP_1KP | RADIO_BSCFG_BS_LIMIT_0,   // BSCFG     Bit synchronization Configuration.
    RADIO_AGCCTRL2_MAX_DVGA_GAIN_ALL | RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB0 | RADIO_AGCCTRL2_MAX_MAGN_TARGET_33,   // AGCCTRL2  AGC control.
    RADIO_AGCCTRL1_AGC_LNA_PRIORITY | RADIO_AGCCTRL1_CS_REL_THR_DISABLED | RADIO_AGCCTRL1_CS_ABS_THR_FLAT,   // AGCCTRL1  AGC control.
    RADIO_AGCCTRL0_HYST_LEVEL_MED | RADIO_AGCCTRL0_WAIT_ITME_16 | RADIO_AGCCTRL0_AGC_FREEZE_NORMAL | RADIO_AGCCTRL0_FILTER_LENGTH_16,   // AGCCTRL0  AGC control.
    RADIO_WOREVT1_EVENT0_HI(128), 	// WOREVT1
    RADIO_WOREVT0_EVENT0_LO(0),		// WOREVT0
    RADIO_WORCTRL_ALCK_PD,			// WORCTRL
	RADIO_FREND1_LNA_CURRENT(1) | RADIO_FREND1_LNA2MIX_CURRENT(1) | RADIO_FREND1_LODIV_BUF_CURRENT_RX(1) | RADIO_FREND1_MIX_CURRENT(2),   // FREND1    Front end RX configuration.
    RADIO_FREND0_LODIV_BUF_CURRENT_TX(1) | RADIO_FREND0_PA_POWER(0),   // FREND0    Front end TX configuration.
    RADIO_FSCAL3_HI(3) | RADIO_FSCAL3_CHP_CURR_CAL_EN(2) | RADIO_FSCAL3_LO(10),   // FSCAL3    Frequency synthesizer calibration.
    RADIO_FSCAL2_FSCAL2(10),   		// FSCAL2    Frequency synthesizer calibration.
    RADIO_FSCAL1(0),   				// FSCAL1    Frequency synthesizer calibration.
    RADIO_FSCAL0(31)   				// FSCAL0    Frequency synthesizer calibration.
};
