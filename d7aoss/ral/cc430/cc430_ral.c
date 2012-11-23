/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "rf1a.h"
#include "../../hal/system.h"
#include "cc430_ral.h"

#include "../../log.h"

#define CC430_RSSI_OFFSET 74;

static ral_tx_callback_t tx_callback;
static ral_rx_callback_t rx_callback;

ral_rx_res_t rx_response;

RadioStateEnum radioState;
u8 radioFlags;
u8 radioRxLimit;
u8 radioCSThreshold;
u16 radioRxTimout;

u8 rxData[255];
u8 rxLength;
u8 rxRemainingBytes;
u8* rxDataPointer;

u8 txData[255];
u8 txLength;
u8* txDataPointer;


RF_SETTINGS rfSettings = {
    RADIO_GDO2_VALUE,   // IOCFG2.GDO2_CFG
    RADIO_GDO1_VALUE,   // IOCFG1.GDO1_CFG
    RADIO_GDO0_VALUE,   // IOCFG0.GDO0_CFG
    (RADIO_FIFOTHR_CLOSE_IN_RX_0db | RADIO_FIFOTHR_FIFO_THR_61_4),   // FIFOTHR
    RADIO_SYNC1_CLASS1_NON_FEC,   // SYNC1
    RADIO_SYNC0_CLASS1_NON_FEC,   // SYNC0
    RADIO_PKTLEN,   // PKTLEN
    (RADIO_PKTCTRL1_PQT(3) | RADIO_PKTCTRL1_ADR_CHK_NONE),   // PKTCTRL1  Packet automation control
    (RADIO_PKTCTRL0_WHITE_DATA | RADIO_PKTCTRL0_PKT_FOR_NORMAL | RADIO_PKTCTRL0_CRC | RADIO_PKTCTRL0_LENGTH_INF),   // PKTCTRL0  Packet automation control
    RADIO_ADDR,   // ADDR      Device address
    RADIO_CHAN,   // CHANNR    Channel number.
    RADIO_FREQ_IF,   // FSCTRL1   Frequency synthesizer control.
    RADIO_FREQOFF,   // FSCTRL0   Frequency synthesizer control.
    RADIO_FREQ2,   // FREQ2     Frequency control word, high byte.
    RADIO_FREQ1,   // FREQ1     Frequency control word, middle byte.
    RADIO_FREQ0,   // FREQ0     Frequency control word, low byte.
    (RADIO_MDMCFG4_CHANBW_E(2) | RADIO_MDMCFG4_CHANBW_M(1) | RADIO_MDMCFG4_DRATE_E(11)),   // MDMCFG4   Modem configuration.
    RADIO_MDMCFG3_DRATE_M(26),   // MDMCFG3   Modem configuration.
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

static void reset(void)
{
	volatile u16 i;
	u8 x;
	
	// Reset radio core
	Strobe(RF_SRES);
	// Reset Radio Pointer
	Strobe(RF_SNOP);

	// Wait before checking IDLE 
	for (i=100; i>0; --i);
	do {
		x = Strobe(RF_SIDLE);
	} while ((x&0x70)!=0x00);  
	
	// Clear radio error register
	RF1AIFERR = 0;
}

static void disable_interrupt(void)
{
    RF1AIE = 0;
}

static void listen()
{
	radioState = RadioStateReceiveInit;
	WriteSingleReg(FIFOTHR, (RADIO_FIFOTHR_CLOSE_IN_RX_0db | RADIO_FIFOTHR_FIFO_THR_61_4));
	WriteSingleReg(PKTCTRL0, (RADIO_PKTCTRL0_WHITE_DATA | RADIO_PKTCTRL0_PKT_FOR_NORMAL | RADIO_PKTCTRL0_LENGTH_INF));
	WriteSingleReg(PKTLEN, 0xFF);

	RF1AIFG = 0;
	RF1AIE  = RFIFG_FLAG_SyncWord;
	RF1AIES = RFIFG_FLANK_SyncWord;

	Strobe(RF_SRX); // start rx
}

static void sleep()
{
    Strobe(RF_SIDLE);
    Strobe(RF_SPWD);
}

static void idle()
{
	Strobe(RF_SIDLE);
}

static void flush_tx()
{
    Strobe(RF_SFTX);
}

void flush_rx()
{
    Strobe(RF_SFRX);
}

static void stop_listening()
{
	radioState = RadioStateNone;
	disable_interrupt();
	idle();
	flush_rx();
}

// TODO call from application layer?
static int get_rssi()
{
    s8  rssi_raw;

    rssi_raw = (s8) ReadSingleReg(RSSI);      // CC430 RSSI is 0.5 dBm units, signed byte
    int rssi = (int)rssi_raw;         // Convert to signed 16 bit (1 instr on MSP)
    rssi += 128;                      // Make it positive...
    rssi >>= 1;                        // ...So division to 1 dBm units can be a shift...
    rssi -= 64 + CC430_RSSI_OFFSET;     // ...and then rescale it, including offset

    return rssi;
}

static void read_data()
{
    if (rxLength == 0)
    {
        // TODO should be working without this hack: now waiting until 30 bit is received
    	__delay_cycles(540*CLOCKS_PER_1us);
    	u8 rxBytes = ReadSingleReg(RXBYTES);
    	ReadBurstReg(RF_RXFIFORD, rxData, rxBytes);
        rxLength = rxData[0];
    	WriteSingleReg(PKTLEN, rxLength);
        rxRemainingBytes = rxLength>rxBytes ? rxLength-rxBytes: 0;
        rxDataPointer = &rxData[rxBytes];
    }

    while ((rxRemainingBytes > 0) && (ReadSingleReg(RXBYTES) > 0))
    {
	   *rxDataPointer = ReadSingleReg(RF_RXFIFORD);
		rxDataPointer++;
		rxRemainingBytes--;
    }
}

static void tx_switch_to_end_state()
{
    RF1AIFG = 0;
    RF1AIE  = RFIFG_FLAG_EndOfPacket;
    RF1AIES = RFIFG_FLANK_EndOfPacket;

}
//
static void tx_finish()
{
	disable_interrupt();
    sleep();
    radioState = RadioStateNone;

    tx_callback(); // TODO return code
}

static void rx_timeout_isr()
{
	// TODO: implement
}

static void rx_sync_isr()
{
    //				RFIFG3				RFIF4						RFIFG7				RFIFG9
    RF1AIE  = RFIFG_FLAG_RXFilled | RFIFG_FLAG_RXFilledOrEOP  | RFIFG_FLAG_RXOverflow | RFIFG_FLAG_SyncWord;
    RF1AIES = RFIFG_FLANK_RXFilled | RFIFG_FLANK_RXFilledOrEOP  | RFIFG_FLANK_RXOverflow | RFIFG_FLANK_SyncWord;
}

static void rx_data_isr()
{
    // disable interrupts, clear pending interrupts
    RF1AIE = 0x00;
    RF1AIFG = 0x00;

    if (radioState == RadioStateReceiveInit)
    {
		// make buffer size bigger to have less interrupts
		radioRxLimit = 54;
		WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_1_64);

		rxLength = 0;
		radioState = RadioStateReceive;
    }

    // Get data from RX FIFO
    read_data();

    if (rxRemainingBytes == 0)
    {
        radioState = RadioStateReceiveDone;
        idle();
        flush_rx();
        sleep(); // TODO sleep or power down

        rx_response.eirp = rxData[1] * 0.5 - 40;
        rx_response.len = rxLength;
        rx_response.data = rxData;
        rx_response.lqi = ReadSingleReg(LQI);
        rx_response.rssi = get_rssi();
        rx_response.crc_ok = ReadSingleReg(PKTSTATUS) >> 7;
        rx_callback(&rx_response); // TODO get callback out of ISR?
        return;
    }
    else if (rxRemainingBytes <= radioRxLimit)
    {
        radioRxLimit = rxRemainingBytes;
    }

    RF1AIE  = RFIFG_FLAG_RXFilled | RFIFG_FLAG_RXFilledOrEOP  | RFIFG_FLAG_RXOverflow | RFIFG_FLAG_SyncWord;
}


#pragma vector=CC1101_VECTOR
__interrupt void CC1101_ISR (void)
{
  u16 isr_vector =   RF1AIV;
  u16 core_vector = isr_vector & 0x003E;
  u16 core_edge = 1 << ((core_vector-2) >> 1);

  core_edge  &= RF1AIES;
  core_vector += (core_edge) ? 0x22 : 0;

  #ifdef DEBUG
  char msg[10] = "RFINT ";
  msg[6] = 0x30 + (core_vector / 10);
  msg[7] = 0x30 + (core_vector % 10);
  Log_PrintString(msg, 8);
  #endif

  switch(__even_in_range(core_vector, 0x42))
  {
    // rising edges
    case  0x00:  	break;                         // No RF core interrupt pending
    case  0x02:
      //TODO: timeout not implemented yet
      rx_timeout_isr();
      break;                         // RFIFG0
    case  0x04:      break;                         // RFIFG1 - RSSI_VALID
    case  0x06: 	 break;                         // RFIFG2
    case  0x08:
        rx_data_isr();
        break;                         // RFIFG3 RX FIFO filled or above RX FIFO threshold
    case 0x0A:
      rx_data_isr();
      break;                         // RFIFG4 RX FIFO filled or end of packet reached
    case 0x0C:      break;                         // RFIFG5 TXAboveThresh_ISR();
    case 0x0E: 		break;                         // RFIFG6
    case 0x10:    	break;                         // RFIFG7
    case 0x12:
    	tx_switch_to_end_state();
    	break;                         // RFIFG8
    case 0x14:                                // RFIFG9 SyncWord_ISR()
    	rx_sync_isr();
    	break;
    case 0x16:    	break;                         // RFIFG10 CRC OK
    case 0x18:      break;                         // RFIFG11 PQT Reached
    case 0x1A:      break;                         // RFIFG12 CCA
    case 0x1C:		break;                         // RFIFG13 CarrierSense (RSSI above threshold)
    case 0x1E:    	break;                         // RFIFG14
    case 0x20:   	break;                         // RFIFG15 WOR

    // Falling Edges
      case  0x22: break;                         // No RF core interrupt pending
      case  0x24: break;                         // RFIFG0
      case  0x26: break;                         // RFIFG1
      case  0x28: break;                         // RFIFG2
      case  0x2A: break;                                // RFIFG3
      case 0x2C: break;                         // RFIFG4
      case 0x2E:                                // RFIFG5 TXBelowThresh_ISR();
    	  tx_switch_to_end_state();
          break;
      case 0x30: break;                         // RFIFG6
      case 0x32: break;                         // RFIFG7
      case 0x34: break;                         // RFIFG8
      case 0x36:
        if (radioState == RadioStateTransmitData)
            tx_finish();
        break;
      case 0x38: break;                         // RFIFG10
      case 0x3A: break;                         // RFIFG11
      case 0x3C: break;                         // RFIFG12
      case 0x3E: break;                         // RFIFG13 RSSI below threshold
      case 0x40: break;                         // RFIFG14
      case 0x42: break;                         // RFIFG15
      default:
          __no_operation();
          break;

  }

  LPM4_EXIT;
}


void cc430_ral_init(void)
{
	reset();

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
	// TODO use other tx_cfg settings

	flush_tx();
	WriteSingleReg(PKTCTRL0, (RADIO_PKTCTRL0_WHITE_DATA | RADIO_PKTCTRL0_PKT_FOR_NORMAL | RADIO_PKTCTRL0_LENGTH_FIXED));
	WriteSingleReg(PKTLEN, tx_cfg->len);
	WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_CLOSE_IN_RX_0db | RADIO_FIFOTHR_FIFO_THR_61_4);

	//TODO: this works only for packets < 62 bytes
	WriteBurstReg(RF_TXFIFOWR, tx_cfg->data, tx_cfg->len); // TODO remove length byte from data?

	radioState = RadioStateTransmitData;

	idle();

	RF1AIFG = 0;
	RF1AIE  = 0 | RFIFG_FLAG_TXBelowThresh | 0;
	RF1AIES = 0 | RFIFG_FLANK_TXBelowThresh | 0;
	Strobe(RF_STX);
}

void cc430_ral_rx_start(ral_rx_cfg_t* cfg)
{
	// TODO channel bandwith conf?
	// TODO center freq
	// TODO set modulation
	// TODO set symbol rate
	WriteSingleReg(CHANNR, cfg->channel_center_freq_index);
	if(cfg->sync_word_class == 0x00) // TODO assert valid class
	{
		if(cfg->coding_scheme == 0x00)
		{
			WriteSingleReg(SYNC1, RADIO_SYNC1_CLASS0_NON_FEC);
			WriteSingleReg(SYNC0, RADIO_SYNC0_CLASS0_NON_FEC);
		}

		// TODO assert, FEC not implemented yet
	}
	else if(cfg->sync_word_class == 0x01)
	{
		if(cfg->coding_scheme == 0x00)
		{
			WriteSingleReg(SYNC1, RADIO_SYNC1_CLASS1_NON_FEC);
			WriteSingleReg(SYNC0, RADIO_SYNC0_CLASS1_NON_FEC);
		}

		// TODO assert, FEC not implemented yet
	}

	listen();
}

void cc430_ral_rx_stop()
{
	if(cc430_ral_is_rx_in_progress())
		stop_listening();
}

bool cc430_ral_is_rx_in_progress()
{
	return radioState == RadioStateReceive || radioState == RadioStateReceiveInit;
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


