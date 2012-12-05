/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include "rf1a.h"
#include "../../hal/system.h"
#include "cc430_ral.h"

#define CC430_RSSI_OFFSET 74;

static ral_tx_callback_t tx_callback;
static ral_rx_callback_t rx_callback;

ral_rx_res_t rx_response;

RadioStateEnum radioState;
u8 radioFlags;
u8 radioCSThreshold;
u16 radioRxTimout;

u8 rxData[255];
u8 rxLength;
u8 rxRemainingBytes;
u8* rxDataPointer;

u8 txData[255];
u8 txLength;
u8 txRemainingBytes;
u8* txDataPointer;

//Interrupt function defines
static void no_interrupt_isr(void);
static void endofpacket_isr(void);
static void rx_timeout_isr(void);
static void rx_sync_isr(void);
static void rx_data_isr(void);
static void tx_data_isr(void);
static void radio_finish(void);

// Interrupt branch table
InterruptHandler interrupt_table[34] = {
	//Rising Edges
	no_interrupt_isr,        	// No RF core interrupt pending
	rx_timeout_isr,				// RFIFG0 - TODO: timeout not implemented yet
	no_interrupt_isr,           // RFIFG1 - RSSI_VALID
	no_interrupt_isr,           // RFIFG2
	rx_data_isr,				// RFIFG3 - RX FIFO filled or above the RX FIFO threshold
	no_interrupt_isr,           // RFIFG4 - RX FIFO filled or above the RX FIFO threshold or end of packet is reached (Do not use for eop! Will not reset until rxfifo emptied)
	no_interrupt_isr,		    // RFIFG5 - TX FIFO filled or above the TX FIFO threshold
	no_interrupt_isr,			// RFIFG6 - TX FIFO full
	radio_finish, 			    // RFIFG7 - RX FIFO overflowed
	radio_finish,				// RFIFG8 - TX FIFO underflowed
	rx_sync_isr,				// RFIFG9 - Sync word sent or received
	no_interrupt_isr,           // RFIFG10 - Packet received with CRC OK
	no_interrupt_isr,           // RFIFG11 - Preamble quality reached (PQI) is above programmed PQT value
	no_interrupt_isr,           // RFIFG12 - Clear channel assessment when RSSI level is below threshold
	no_interrupt_isr,           // RFIFG13 - Carrier sense. RSSI level is above threshold
	no_interrupt_isr,           // RFIFG14 - WOR event 0
	no_interrupt_isr,       	// RFIFG15 - WOR event 1

	//Falling Edges
	no_interrupt_isr,        	// No RF core interrupt pending
	no_interrupt_isr,			// RFIFG0 TODO: timeout not implemented yet
	no_interrupt_isr,           // RFIFG1 - RSSI_VALID
	no_interrupt_isr,           // RFIFG2 -
	no_interrupt_isr,			// RFIFG3 - RX FIFO drained below RX FIFO threshold
	no_interrupt_isr,           // RFIFG4 - RX FIFO empty
	tx_data_isr,			    // RFIFG5 - TX FIFO below TX FIFO threshold
	no_interrupt_isr,			// RFIFG6 - TX FIFO below TX FIFO threshold
	no_interrupt_isr,           // RFIFG7 - RX FIFO flushed
	no_interrupt_isr,			// RFIFG8 - TX FIFO flushed
	endofpacket_isr,			// RFIFG9 - End of packet or in RX when optional address check fails or RX FIFO overflows or in TX when TX FIFO underflows
	no_interrupt_isr,           // RFIFG10 - First byte read from RX FIFO
	no_interrupt_isr,           // RFIFG11 - (LPW)
	no_interrupt_isr,           // RFIFG12 - RSSI level is above threshold
	no_interrupt_isr,           // RFIFG13 - RSSI level is below threshold
	no_interrupt_isr,           // RFIFG14 - WOR event 0 + 1 ACLK
	no_interrupt_isr,       	// RFIFG15 - RF oscillator stable or next WOR event0 triggered
};

//RF settings
RF_SETTINGS rfSettings = {
    RADIO_GDO2_VALUE,   // IOCFG2.GDO2_CFG
    RADIO_GDO1_VALUE,   // IOCFG1.GDO1_CFG
    RADIO_GDO0_VALUE,   // IOCFG0.GDO0_CFG
    (RADIO_FIFOTHR_CLOSE_IN_RX_0db | RADIO_FIFOTHR_FIFO_THR_61_4),   // FIFOTHR
    RADIO_SYNC1_CLASS1_NON_FEC,   // SYNC1
    RADIO_SYNC0_CLASS1_NON_FEC,   // SYNC0
    RADIO_PKTLEN,   // PKTLEN
    (RADIO_PKTCTRL1_PQT(3) | RADIO_PKTCTRL1_ADR_CHK_NONE),   // PKTCTRL1  Packet automation control
    (RADIO_PKTCTRL0_WHITE_DATA | RADIO_PKTCTRL0_PKT_FOR_NORMAL | RADIO_PKTCTRL0_LENGTH_FIXED),   // PKTCTRL0  Packet automation control
    RADIO_ADDR,   // ADDR      Device address
    RADIO_CHAN,   // CHANNR    Channel number.
    RADIO_FREQ_IF,   // FSCTRL1   Frequency synthesizer control.
    RADIO_FREQOFF,   // FSCTRL0   Frequency synthesizer control.
    RADIO_FREQ2,   // FREQ2     Frequency control word, high byte.
    RADIO_FREQ1,   // FREQ1     Frequency control word, middle byte.
    RADIO_FREQ0,   // FREQ0     Frequency control word, low byte.
    (RADIO_MDMCFG4_CHANBW_E(2) | RADIO_MDMCFG4_CHANBW_M(1) | RADIO_MDMCFG4_DRATE_E(11)),   // MDMCFG4   Modem configuration.
    RADIO_MDMCFG3_DRATE_M(24),   // MDMCFG3   Modem configuration.
    (RADIO_MDMCFG2_DEM_DCFILT_ON | RADIO_MDMCFG2_MOD_FORMAT_GFSK | RADIO_MDMCFG2_SYNC_MODE_16in16CS),   // MDMCFG2   Modem configuration.
    (RADIO_MDMCFG1_NUM_PREAMBLE_4B | RADIO_MDMCFG1_CHANSPC_E(2)),   // MDMCFG1   Modem configuration.
    RADIO_MDMCFG0_CHANSPC_M(16),   // MDMCFG0   Modem configuration.
    (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)),   // DEVIATN   Modem deviation setting (when FSK modulation is enabled). FSK 50 kHz
    RADIO_MCSM2_RX_TIME(0),   // MCSM2     Main Radio Control State Machine configuration.
    (RADIO_MCSM1_CCA_RSSILOWRX | RADIO_MCSM1_RXOFF_MODE_IDLE | RADIO_MCSM1_TXOFF_MODE_IDLE),   // MCSM1     Main Radio Control State Machine configuration.
    (RADIO_MCSM0_FS_AUTOCAL_FROMIDLE | 0x08),   // MCSM0     Main Radio Control State Machine configuration.
    (RADIO_FOCCFG_FOC_PRE_K_3K | RADIO_FOCCFG_FOC_POST_K_HALFK | RADIO_FOCCFG_FOC_LIMIT_4THBW),   // FOCCFG    Frequency Offset Compensation Configuration.
    (RADIO_BSCFG_BS_PRE_KI_2KI | RADIO_BSCFG_BS_PRE_KP_3KP | RADIO_BSCFG_BS_POST_KI_1KP | RADIO_BSCFG_BS_POST_KP_1KP | RADIO_BSCFG_BS_LIMIT_0),   // BSCFG     Bit synchronization Configuration.
    (RADIO_AGCCTRL2_MAX_DVGA_GAIN_ALL | RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB0 | RADIO_AGCCTRL2_MAX_MAGN_TARGET_33),   // AGCCTRL2  AGC control.
    (RADIO_AGCCTRL1_AGC_LNA_PRIORITY | RADIO_AGCCTRL1_CS_REL_THR_DISABLED | RADIO_AGCCTRL1_CS_ABS_THR_FLAT),   // AGCCTRL1  AGC control.
    (RADIO_AGCCTRL0_HYST_LEVEL_MED | RADIO_AGCCTRL0_WAIT_ITME_16 | RADIO_AGCCTRL0_AGC_FREEZE_NORMAL | RADIO_AGCCTRL0_FILTER_LENGTH_16),   // AGCCTRL0  AGC control.
    RADIO_WOREVT1_EVENT0_HI(128), //0x80, // WOREVT1
    RADIO_WOREVT0_EVENT0_LO(0), // WOREVT0
    RADIO_WORCTRL_ALCK_PD, // WORCTRL
    (RADIO_FREND1_LNA_CURRENT(1) | RADIO_FREND1_LNA2MIX_CURRENT(1) | RADIO_FREND1_LODIV_BUF_CURRENT_RX(1) | RADIO_FREND1_MIX_CURRENT(2)), // FREND1
    (RADIO_FREND0_LODIV_BUF_CURRENT_TX(1) | RADIO_FREND0_PA_POWER(0)), // FREND0
    (RADIO_FSCAL3_HI(3) | RADIO_FSCAL3_CHP_CURR_CAL_EN(2) | RADIO_FSCAL3_LO(0x0F)),   // FSCAL3    Frequency synthesizer calibration.
    (RADIO_FSCAL2_VCO_CORE_H_EN| RADIO_FSCAL2_FSCAL2(0x1F)),   // FSCAL2    Frequency synthesizer calibration.
    RADIO_FSCAL1(0x3F),   // FSCAL1    Frequency synthesizer calibration.
    RADIO_FSCAL0(31),   // FSCAL0    Frequency synthesizer calibration.
};

// TODO call from application layer?
static int get_rssi()
{
    s8  rssi_raw;

    rssi_raw = (s8) ReadSingleReg(RSSI);      	// CC430 RSSI is 0.5 dBm units, signed byte
    int rssi = (int)rssi_raw;         			// Convert to signed 16 bit (1 instr on MSP)
    rssi += 128;                      			// Make it positive...
    rssi >>= 1;                        			// ...So division to 1 dBm units can be a shift...
    rssi -= 64 + CC430_RSSI_OFFSET;     		// ...and then rescale it, including offset

    return rssi;
}

static void no_interrupt_isr() { }

static void endofpacket_isr()
{
    if (radioState == RadioStateTransmit) {
    	radio_finish();
        tx_callback();	// TODO get callback out of ISR?
    } else if(radioState == RadioStateReceive) {
    	rx_data_isr();
    	radio_finish();
    	rx_callback(&rx_response); // TODO get callback out of ISR?
    }
}

static void rx_timeout_isr()
{
	// TODO: implement
}

static void rx_sync_isr()
{
	//Only if radio receiving
	if(radioState != RadioStateReceive)
		radio_finish();

    //Reset receive data
    rxLength = 0;
    rxRemainingBytes = 0;
    rxDataPointer = &rxData[0];

	//Clear all interrupt flags, enable interrupts
	RF1AIFG = 0;
	RF1AIES = RFIFG_FLANK_RXFilled | RFIFG_FLANK_EndOfPacket;
	RF1AIE = RFIFG_FLAG_RXFilled | RFIFG_FLAG_EndOfPacket;
}

static void rx_data_isr()
{
	//Only if radio receiving
	if(radioState != RadioStateReceive)
		radio_finish();

	//Read number of bytes in RXFIFO
	u8 rxBytes = ReadSingleReg(RXBYTES);

	//If length is not set (first time after sync word)
	//get the length from RXFIFO and set PKTLEN so eop can be detected right
    if (rxLength == 0 && rxBytes > 0) {
    	rxLength = ReadSingleReg(RXFIFO);
    	WriteSingleReg(PKTLEN, rxLength);
    	WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);
    	rxRemainingBytes = rxLength - 1;
		rxData[0] = rxLength;
		rxDataPointer++;
    }

    //Never read the entire buffer as long as more data is going to be received
    if (rxRemainingBytes > rxBytes) {
    	rxBytes--;
    } else {
    	rxBytes = rxRemainingBytes;
    }

    //Read data from buffer
	ReadBurstReg(RXFIFO, rxDataPointer, rxBytes);
    rxRemainingBytes -= rxBytes;
    rxDataPointer += rxBytes;

    if (rxRemainingBytes == 0) {
        rx_response.eirp = (rxData[1] >> 1) - 40;
        rx_response.len = rxLength;
        rx_response.data = rxData;
        rx_response.lqi = ReadSingleReg(LQI);
        rx_response.rssi = get_rssi();
        rx_response.crc_ok = ReadSingleReg(PKTSTATUS) >> 7;
    }
}

static void tx_data_isr()
{
	//Only if radio transmitting
	if(radioState != RadioStateTransmit)
		radio_finish();

	//Read number of free bytes in TXFIFO
	u8 txBytes = 64 - ReadSingleReg(TXBYTES);

	if(txRemainingBytes < txBytes)
	{
		txBytes = txRemainingBytes;
	}

	WriteBurstReg(RF_TXFIFOWR, txDataPointer, txBytes);
	txRemainingBytes -= txBytes;
	txDataPointer += txBytes;
}

static void radio_finish()
{
	//Set radio state
	radioState = RadioStateIdle;

    //Disable interrupts
    RF1AIE = 0;
    RF1AIES = 0;
    RF1AIFG = 0;

	//Flush FIFOs and go to sleep
    Strobe(RF_SIDLE);
    Strobe(RF_SFRX);
    Strobe(RF_SFTX);
    Strobe(RF_SPWD);
}

static void set_channel(u8 channel_center_freq_index, u8 channel_bandwith_index)
{
	//Set channel center frequency
	WriteSingleReg(CHANNR, channel_center_freq_index);

	//Set channel bandwidth, modulation and symbol rate
	switch(channel_bandwith_index)
	{
	case 0:
		WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(24));
		WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(1) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(11)));
		WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
		break;
	case 1:
		WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(24));
		WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(2) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(11)));
		WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
		break;
	case 2:
		WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(248));
		WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(1) | RADIO_MDMCFG4_CHANBW_M(0) | RADIO_MDMCFG4_DRATE_E(12)));
		WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
		break;
	case 3:
		WriteSingleReg(MDMCFG3, RADIO_MDMCFG3_DRATE_M(248));
		WriteSingleReg(MDMCFG4, (RADIO_MDMCFG4_CHANBW_E(0) | RADIO_MDMCFG4_CHANBW_M(1) | RADIO_MDMCFG4_DRATE_E(12)));
		WriteSingleReg(DEVIATN, (RADIO_DEVIATN_E(5) | RADIO_DEVIATN_M(0)));
		break;
	}
}

static void set_sync_word_class(u8 sync_word_class, u8 coding_scheme)
{
	if(sync_word_class == 0x00) { // TODO assert valid class
		if(coding_scheme == 0x00) {
			WriteSingleReg(SYNC1, RADIO_SYNC1_CLASS0_NON_FEC);
			WriteSingleReg(SYNC0, RADIO_SYNC0_CLASS0_NON_FEC);
		} else if(coding_scheme == 0x01) {
			WriteSingleReg(SYNC1, RADIO_SYNC1_CLASS0_NON_FEC);
			WriteSingleReg(SYNC0, RADIO_SYNC0_CLASS0_NON_FEC);
		}

	} else if(sync_word_class == 0x01) {
		if(coding_scheme == 0x00) {
			WriteSingleReg(SYNC1, RADIO_SYNC1_CLASS1_FEC);
			WriteSingleReg(SYNC0, RADIO_SYNC0_CLASS1_FEC);
		} else if(coding_scheme == 0x01) {
			WriteSingleReg(SYNC1, RADIO_SYNC1_CLASS1_FEC);
			WriteSingleReg(SYNC0, RADIO_SYNC0_CLASS1_FEC);
		}
	}
}

#pragma vector=CC1101_VECTOR
__interrupt void CC1101_ISR (void)
{
  u16 isr_vector = RF1AIV >> 1;
  u16 edge = (1 << (isr_vector - 1)) & RF1AIES;
  if(edge) isr_vector += 0x11;
  interrupt_table[isr_vector]();
  LPM4_EXIT;
}

void cc430_ral_init(void)
{
	u8 x;
	volatile u16 i;

	//Modify radio state
	radioState = RadioStateIdle;


	//Reset radio core
	ResetRadioCore();

	//Wait for idle
	for (i=100; i>0; --i);
	do {
		x = Strobe(RF_SIDLE);
	} while ((x&0x70)!=0x00);

	//Clear radio error register
	RF1AIFERR = 0;

	//Disable interrupts
    RF1AIE = 0;
    RF1AIES = 0;
    RF1AIFG = 0;

	//Write configuration
    WriteBurstReg(IOCFG2, (unsigned char*) &rfSettings, sizeof(rfSettings));
    WriteSingleReg(TEST0, (RADIO_TEST0_HI(2) | RADIO_TEST0_VCO_SEL_CAL_DIS ));

    // TODO: configurable
    WritePATable(0x51); // 0 dBm
}

void cc430_ral_set_tx_callback(ral_tx_callback_t callback)
{
    tx_callback = callback;
}

void cc430_ral_set_rx_callback(ral_rx_callback_t callback)
{
    rx_callback = callback;
}

void cc430_ral_tx(ral_tx_cfg_t* tx_cfg)
{
	//Only if radio idle
	if(radioState != RadioStateIdle)
		return;

	//Modify radio state
	radioState = RadioStateTransmit;

	//Set configuration
	set_channel(tx_cfg->channel_center_freq_index, tx_cfg->channel_bandwith_index);
	set_sync_word_class(tx_cfg->sync_word_class, tx_cfg->coding_scheme);

	//Set PKTLEN to the packet length
	//Set RXFIFO threshold
	WriteSingleReg(PKTLEN, tx_cfg->len);
	WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);

	//Clear all flags, enable interrupts
	RF1AIFG = 0;
	RF1AIES = RFIFG_FLANK_EndOfPacket | RFIFG_FLANK_TXBelowThresh | RFIFG_FLANK_TXUnderflow;
	RF1AIE = RFIFG_FLAG_EndOfPacket | RFIFG_FLAG_TXBelowThresh | RFIFG_FLAG_TXUnderflow;

	//Flush TXFIFO
	Strobe(RF_SIDLE);
	Strobe(RF_SFTX);

	//Prepare data
	u8 txBytes = 64;
	txLength = tx_cfg->len;
	txRemainingBytes = txLength;
	txDataPointer = tx_cfg->data; // TODO copy data

	if(txRemainingBytes < txBytes)
	{
		txBytes = txRemainingBytes;
	}

	WriteBurstReg(RF_TXFIFOWR, txDataPointer, txBytes);
	txRemainingBytes -= txBytes;
	txDataPointer += txBytes;

	// Start transmitting
	Strobe(RF_STX);
}

void cc430_ral_rx_start(ral_rx_cfg_t* cfg)
{
	//Only if radio idle
	if(radioState != RadioStateIdle)
		return;

	//Modify radio state
	radioState = RadioStateReceive;

	//Set configuration
	set_channel(cfg->channel_center_freq_index, cfg->channel_bandwith_index);
	set_sync_word_class(cfg->sync_word_class, cfg->coding_scheme);

	//Set PKTLEN to the highest possible number, we will change this to the right length later
	//Set RXFIFO threshold as low as possible
	WriteSingleReg(PKTLEN, RADIO_PKTLEN);
	WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_61_4);

	//Flush RXFIFO
	Strobe(RF_SIDLE);
	Strobe(RF_SFRX);

	//Clear all flags, enable interrupts
	RF1AIFG = 0;
	RF1AIES = RFIFG_FLANK_SyncWord;
	RF1AIE = RFIFG_FLAG_SyncWord;

	//Start receiving
	Strobe(RF_SRX);
}

void cc430_ral_rx_stop()
{
	radio_finish();
}

bool cc430_ral_is_rx_in_progress()
{
	return radioState == RadioStateReceive;
}

// The CC430 implementation of the RAL interface
const struct ral_interface cc430_ral =
{
	cc430_ral_init,
	cc430_ral_tx,
	cc430_ral_set_tx_callback,
	cc430_ral_set_rx_callback,
	cc430_ral_rx_start,
	cc430_ral_rx_stop,
	cc430_ral_is_rx_in_progress
};
