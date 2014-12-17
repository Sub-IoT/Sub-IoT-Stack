/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "phy.h"
#include "fec.h"
#include "cc1101_phy.h"
#include "cc1101_registers.h"
#include "cc1101_constants.h"
#include "../framework/log.h"

#include "cc1101_core.h"
#include "radio_hw.h"
/*
 * Variables
 */
extern RF_SETTINGS rfSettings;

RadioState state;

uint8_t buffer[255];
uint8_t packetLength;
uint8_t* bufferPosition;
uint16_t remainingBytes;

bool fec;
static uint8_t frequency_band;
uint8_t channel_center_freq_index;
uint8_t channel_bandwidth_index;
uint8_t preamble_size;
uint16_t sync_word;

bool init_and_close_radio = true;

static uint8_t previous_spectrum_id = 0xFF;
static uint8_t previous_sync_word_class = 0xFF;

phy_rx_data_t rx_data;

phy_tx_cfg_t last_tx_cfg;


//for this moment both int8, should be changed to float or double values.
static int8_t eirp_values[103] = {9.9, 9.5, 9.2, 8.8, 8.5, 8.1, 7.8, 7.4, 7.1, 6.8, 6.4, 6.3, 6.1, 6.0, 5.8, 5.5, 5.1, 4.9, 4.8, 4.4, //20
							4.0, 3.6, 3.2, 2.8, 2.3, 2.0, 1.9, 1.4, 0.4, 0.1, -0.3, -0.5, -0.8, -0.9, -1.1, -1.4, -1.5, -1.7, -2.1, //40
                		   -2.2, -2.3, -2.5, -2.8, -2.9, -3.0, -3.1, -3.3, -3.5, -3.8, -4.0, -4.1, -4.6, -4.7, -5.3, -5.6, -5.9, //56
                		   -6.0, -6.5, -6.8, -6.9, -7.1, -7.7, -7.8, -8.3, -8.7, -8.9, -9.9, -10.1, -11.4, -12.3, -13.3, -13.7, //72
                		   -14.3, -14.9, -15.5, -15.6, -15.7, -16.2, -17.0, -17.8, -18.8, -19.0, -19.3,-19.8, -20.4, -21.0, -21.3, //87
                		   -21.7, -22.5, -23.3, -24.3, -24.5, -25.3, -26.5, -27.9, -29.5, -29.6, -31.4, -33.8, -36.5,-38.3, -38.4, -62.7}; //103

static int8_t eirp_reg_values[103]={0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0x80, 0xCB, 0x81, 0xCC, 0xCD, 0x84, 0xCE, 0x85, 0x86, //20
							0x87, 0x88, 0x89, 0x8A, 0x8B, 0xCF,0x8C, 0x8D, 0x8E, 0x60, 0x51, 0x61, 0x40, 0x52, 0x3, 0x3E, 0x53, 0x3D, 0x3C, 0x54, //40
							0x64, 0x3B, 0x55, 0x65, 0x2F, 0x3A, 0x56, 0x66, 0x39, 0x57, 0x67, 0x8F, 0x2C, 0x2B, 0x37, 0x6A, 0x2A, 0x6B, 0x36, 0x29,//60
							0x6C, 0x6D, 0x28, 0x35, 0x27, 0x6E, 0x26, 0x34, 0x25, 0x33, 0x24, 0x1F, 0x1E, 0x1D, 0x6F, 0x23, 0x32, 0x1B, 0x1A, 0x19,//80
							0x18, 0x22, 0xF, 0x17, 0xD, 0xC, 0x31, 0xB, 0x15, 0x9, 0x14, 0x21, 0x7, 0x13, 0x5, 0x4, 0x12, 0x3, 0x11, 0x1, 0x10, 0x30, 0x0}; //103

static void phy_set_gdo_values(GDOLine gdoLine, GDOEdge gdoEdge, uint8_t gdoValue) {
	WriteSingleReg(gdoLine, gdoValue  | gdoEdge);
}
/*
 * Phy implementation functions
 */
void phy_init(void)
{
	//Set radio state
	state = Idle;
	spi_init();
	radioConfigureInterrupt();

	//Reset the radio core
	ResetRadioCore();

	//Write configuration
	WriteRfSettings(&rfSettings);

	last_tx_cfg.eirp=0;
    last_tx_cfg.spectrum_id[0] = 0;
    last_tx_cfg.spectrum_id[1] = 0;
	last_tx_cfg.sync_word_class=0;

}

void phy_idle(void)
{
	if (state != Idle)
		rxtx_finish_isr();
}

bool phy_translate_and_set_settings(uint8_t spectrum_id, uint8_t sync_word_class)
{
	if (previous_spectrum_id == spectrum_id && previous_sync_word_class == sync_word_class)
		return true;

	Strobe(RF_SIDLE);

    if(!phy_translate_settings(spectrum_id, sync_word_class, &fec, &frequency_band, &channel_center_freq_index, &channel_bandwidth_index, &preamble_size, &sync_word))
	{
		#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "PHY Cannot translate settings");
		#endif

		return false;
	}

	set_channel(channel_center_freq_index, channel_bandwidth_index);
	set_preamble_size(preamble_size);
	set_sync_word(sync_word);

	previous_spectrum_id = spectrum_id;
	previous_sync_word_class = sync_word_class;

	return true;
}


// configure using cfg
extern bool phy_set_tx(phy_tx_cfg_t* cfg)
{
	//General configuration
	phy_translate_and_set_settings(cfg->spectrum_id, cfg->sync_word_class);

	set_eirp(cfg->eirp);

	//TODO Return error if fec not enabled but requested
	#ifdef D7_PHY_USE_FEC
	if (fec) {
		//Disable hardware data whitening
		set_data_whitening(false);
	} else {
#endif
		//Enable hardware data whitening
		set_data_whitening(true);
#ifdef D7_PHY_USE_FEC
	}
	#endif

	return true;
}
// send data
extern bool phy_tx_data(phy_tx_cfg_t* cfg)
{

	//TODO Return error if fec not enabled but requested
	#ifdef D7_PHY_USE_FEC
	if (fec) {
		//Initialize fec encoding
		fec_init_encode(cfg->data);

		//Configure length settings
		set_length_infinite(true);
		fec_set_length(cfg->length);
		remainingBytes = ((cfg->length & 0xFE) + 2) << 1;
		WriteSingleReg(PKTLEN, (uint8_t)(remainingBytes & 0x00FF));
	} else {
	#endif

		//Set buffer position
        //bufferPosition = cfg->data;

		//Configure length settings
		set_length_infinite(false);
        remainingBytes = tx_queue.length;
		WriteSingleReg(PKTLEN, (uint8_t)remainingBytes);
	#ifdef D7_PHY_USE_FEC
	}
	#endif


	//Write initial data to txfifo
	tx_data_isr();
	//Configure txfifo threshold
	WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);

	//Enable interrupts
	radioClearInterruptPendingLines();
	phy_set_gdo_values(GDOLine2, GDO_EDGE_TXBelowThresh, GDO_SETTING_TXBelowThresh);
//	phy_set_gdo_values(GDOLine2, GDO_EDGE_TXUnderflow, GDO_SETTING_TXUnderflow);
	phy_set_gdo_values(GDOLine0, GDO_EDGE_EndOfPacket, GDO_SETTING_EndOfPacket);
	radioEnableGDO2Interrupt();
	radioEnableGDO0Interrupt();

	//Start transmitting
	Strobe(RF_STX);

	return true;
}

bool phy_init_tx()
{
	if(get_radiostate() != Idle)
	{
		#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "PHY radio not idle");
		#endif

		return false;
	}

	//Set radio state
	state = Transmit;

	//Go to idle and flush the txfifo
	Strobe(RF_SIDLE);
	Strobe(RF_SFTX);

	return true;
}

bool phy_tx(phy_tx_cfg_t* cfg)
{
	if (init_and_close_radio)
	{
		if (!phy_init_tx())
		{
			return false;
		}

		//if (last_tx_cfg.spectrum_id != cfg->spectrum_id || last_tx_cfg.sync_word_class != cfg->sync_word_class || last_tx_cfg.eirp != cfg->eirp)
		if (memcmp(&last_tx_cfg, cfg, 3) != 0)
		{
			if (!phy_set_tx(cfg))
				return false;
			memcpy(&last_tx_cfg, cfg, 3);
		}
	}

	return phy_tx_data(cfg);
}

bool phy_rx(phy_rx_cfg_t* cfg)
{
	#ifdef LOG_PHY_ENABLED
	log_print_stack_string(LOG_PHY, "phy_rx");
	#endif

	RadioState current_state = get_radiostate();
	if(current_state != Idle && current_state != Receive)
	{
		#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "PHY Cannot RX, PHy not idle");
		#endif
		return false;
	}

	//Set radio state
	state = Receive;

	//Flush the txfifo
	Strobe(RF_SIDLE);
	Strobe(RF_SFRX);

	//Set configuration

	if (!phy_translate_and_set_settings(cfg->spectrum_id, cfg->sync_word_class))
		return false;

	set_timeout(cfg->timeout);

//TODO Return error if fec not enabled but requested
#ifdef D7_PHY_USE_FEC
	if (fec) {
		//Disable hardware data whitening
		set_data_whitening(false);

		//Initialize fec encoding
		fec_init_decode(buffer);

		//Configure length settings
		set_length_infinite(true);

		if(cfg->length == 0)
		{
			packetLength = 0;
			remainingBytes = 0;
			WriteSingleReg(PKTLEN, 0xFF);
			WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_61_4);
		} else {
			fec_set_length(cfg->length);
			packetLength = ((cfg->length & 0xFE) + 2) << 1;
			remainingBytes = packetLength;
			WriteSingleReg(PKTLEN, (uint8_t)(packetLength & 0x00FF));
			WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);
		}
	} else {
#endif
		//Enable hardware data whitening
		set_data_whitening(true);

		//Set buffer position
		bufferPosition = buffer;

		//Configure length settings and txfifo threshold
		set_length_infinite(false);

		if(cfg->length == 0)
		{
			packetLength = 0;
			remainingBytes = 0;
			WriteSingleReg(PKTLEN, 0xFF);
			WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_61_4);
		} else {
			packetLength = cfg->length;
			remainingBytes = packetLength;
			WriteSingleReg(PKTLEN, packetLength);
			WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);
		}
#ifdef D7_PHY_USE_FEC
	}
#endif

	//TODO: set minimum sync word rss to scan minimum energy
	//Enable interrupts
	phy_set_gdo_values(GDOLine2, GDO_EDGE_RXFilled, GDO_SETTING_RXFilled);
	phy_set_gdo_values(GDOLine0, GDO_EDGE_EndOfPacket, GDO_SETTING_EndOfPacket);
	radioClearInterruptPendingLines();
	radioEnableGDO2Interrupt();
	radioEnableGDO0Interrupt();

	//Start receiving
	Strobe(RF_SRX);

	return true;
}

bool phy_is_rx_in_progress(void)
{
	return (state == Receive);
}

bool phy_is_tx_in_progress(void)
{
	return (state == Transmit);
}

extern int16_t phy_get_rssi(uint8_t spectrum_id[2], uint8_t sync_word_class)
{
	uint8_t rssi_raw = 0;

	if (!phy_translate_and_set_settings(spectrum_id, sync_word_class))
		return false;


    //Start receiving
    Strobe(RF_SRX);

	//FIXME wait for RSSI VALID!!!
    // Is this possible with CC1101?

    rssi_raw = ReadSingleReg(RSSI);
    rxtx_finish_isr();

    return calculate_rssi(rssi_raw);
}

/*
 * Interrupt functions
 */

void no_interrupt_isr() { }

void end_of_packet_isr()
{
	if (state == Receive) {
		rx_data_isr();
		rxtx_finish_isr(); // TODO: should this be called by DLL?
		if(phy_rx_callback != NULL)
			phy_rx_callback(&rx_data);
	} else if (state == Transmit) {
		rxtx_finish_isr();
		if(phy_tx_callback != NULL)
			phy_tx_callback();
	} else {
		rxtx_finish_isr();
	}
}

void tx_data_isr()
{
	//Calculate number of free bytes in TXFIFO
	uint8_t txBytes = 64 - ReadSingleReg(TXBYTES);

#ifdef D7_PHY_USE_FEC
	if(fec)
	{
		uint8_t fecbuffer[4];

		//If remaining bytes is equal or less than 255 - fifo size, set length to fixed
		if(remainingBytes < 192)
			set_length_infinite(false);

		while (txBytes >= 4) {
			//Get encoded data, stop when no more data available
			if(fec_encode(fecbuffer) == false)
				break;

			//Write data to tx fifo
			WriteBurstReg(TXFIFO, fecbuffer, 4);
			remainingBytes -= 4;
			txBytes -= 4;
		}
	} else {
#endif
		//Limit number of bytes to remaining bytes
		if(txBytes > remainingBytes)
			txBytes = remainingBytes;

		//Write data to tx fifo
		WriteBurstReg(TXFIFO, bufferPosition, txBytes);
		remainingBytes -= txBytes;
		bufferPosition += txBytes;
#ifdef D7_PHY_USE_FEC
	}
#endif
}

void rx_data_isr()
{

	//Read number of bytes in RXFIFO
	uint8_t rxBytes = ReadSingleReg(RXBYTES);
#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "rx_data_isr (%d bytes received)", rxBytes);
#endif

#ifdef D7_PHY_USE_FEC
	if(fec)
	{
		uint8_t fecbuffer[4];

	    //If length is not set get the length from RXFIFO and set PKTLEN
		if (packetLength == 0) {
			ReadBurstReg(RXFIFO, fecbuffer, 4);
			fec_decode(fecbuffer);
			fec_set_length(*buffer);
			packetLength = ((*buffer & 0xFE) + 2) << 1;
			remainingBytes = packetLength - 4;
			WriteSingleReg(PKTLEN, (uint8_t)(packetLength & 0x00FF));
			WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);
			rxBytes -= 4;
		}

		//If remaining bytes is equal or less than 255 - fifo size, set length to fixed
		if(remainingBytes < 192)
			set_length_infinite(false);

		//Read data from buffer and decode
		while (rxBytes >= 4) {
			ReadBurstReg(RXFIFO, fecbuffer, 4);

			if(fec_decode(fecbuffer) == false)
				break;

			remainingBytes -= 4;
			rxBytes -= 4;
		}
	} else {
#endif
    //If length is not set get the length from RXFIFO and set PKTLEN
	if (packetLength == 0) {
		packetLength = ReadSingleReg(RXFIFO);
		WriteSingleReg(PKTLEN, packetLength);
		WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);
		remainingBytes = packetLength - 1;
		bufferPosition[0] = packetLength;
		bufferPosition++;
		rxBytes--;
#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "rx_data_isr getting packetLength (%d)", packetLength);
	#endif
	}

	//Never read the entire buffer as long as more data is going to be received
    if (remainingBytes > rxBytes) {
    	rxBytes--;
    } else {
    	rxBytes = remainingBytes;
    }

    //Read data from buffer
	ReadBurstReg(RXFIFO, bufferPosition, rxBytes);
	remainingBytes -= rxBytes;
    bufferPosition += rxBytes;
#ifdef D7_PHY_USE_FEC
	}
#endif

    //When all data has been received read rssi and lqi value and set packetreceived flag
    if(remainingBytes == 0)
    {
    	rx_data.rssi = calculate_rssi(ReadSingleReg(RXFIFO));
    	rx_data.lqi = ReadSingleReg(RXFIFO) & 0x7F;
		rx_data.length = *buffer;
		rx_data.data = buffer;
		#ifdef LOG_PHY_ENABLED
			log_print_stack_string(LOG_PHY, "rx_data_isr packet received");
		#endif
    }

	#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "rx_data_isr 1");
	#endif
}

void rx_timeout_isr()
{
	rxtx_finish_isr();
	if(phy_rx_callback != NULL)
		phy_rx_callback(NULL);
}

void rx_fifo_overflow_isr()
{
	#ifdef LOG_PHY_ENABLED
		log_print_stack_string(LOG_PHY, "rx_fifo_overflow");
	#endif

	rxtx_finish_isr();
	if(phy_rx_callback != NULL)
		phy_rx_callback(NULL);
}

void rxtx_finish_isr()
{
	//Disable interrupts
	radioDisableGDO0Interrupt();
	radioDisableGDO2Interrupt();

	if (init_and_close_radio)
	{
		//Flush FIFOs and go to sleep
		Strobe(RF_SIDLE);
		Strobe(RF_SFRX);
		Strobe(RF_SFTX);
		Strobe(RF_SPWD);

		//Set radio state
		state = Idle;
	}
}

/*
 * Private functions
 */
RadioState get_radiostate(void)
{
	uint8_t state = Strobe(RF_SNOP) >> 4;

	if(state > 7)
		return Idle;

	return (RadioState)state;
}

void set_channel(uint8_t channel_center_freq_index, uint8_t channel_bandwith_index)
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

	// is this the right place?
	Strobe(RF_SCAL);
}

void set_sync_word(uint16_t sync_word)
{
	WriteSingleReg(SYNC0, (uint8_t)(sync_word >> 8));
	WriteSingleReg(SYNC1, (uint8_t)(sync_word & 0x00FF));
}

void set_preamble_size(uint8_t preamble_size)
{
	uint8_t mdmcfg1 = ReadSingleReg(MDMCFG1) & 0x03;

	if(preamble_size >= 24)
		mdmcfg1 |= 0x70;
	else if(preamble_size >= 16)
		mdmcfg1 |= 0x60;
	else if(preamble_size >= 12)
		mdmcfg1 |= 0x50;
	else if(preamble_size >= 8)
		mdmcfg1 |= 0x40;
	else if(preamble_size >= 6)
		mdmcfg1 |= 0x30;
	else if(preamble_size >= 4)
		mdmcfg1 |= 0x20;
	else if(preamble_size >= 3)
		mdmcfg1 |= 0x10;

	WriteSingleReg(MDMCFG1, mdmcfg1);
}

void set_data_whitening(bool white_data)
{
	uint8_t pktctrl0 = ReadSingleReg(PKTCTRL0) & 0xBF;

	if (white_data)
		pktctrl0 |= RADIO_PKTCTRL0_WHITE_DATA;

	WriteSingleReg(PKTCTRL0, pktctrl0);
}

void set_length_infinite(bool infinite)
{
	uint8_t pktctrl0 = ReadSingleReg(PKTCTRL0) & 0xFC;

	if (infinite)
		pktctrl0 |= RADIO_PKTCTRL0_LENGTH_INF;

	WriteSingleReg(PKTCTRL0, pktctrl0);
}

void set_timeout(uint16_t timeout)
{
	if (timeout == 0) {
		WriteSingleReg(WORCTRL, RADIO_WORCTRL_ALCK_PD);
		WriteSingleReg(MCSM2, RADIO_MCSM2_RX_TIME(7));
	} else {
		WriteSingleReg(WORCTRL, RADIO_WORCTRL_WOR_RES_32ms);
		WriteSingleReg(MCSM2, RADIO_MCSM2_RX_TIME(0));
		WriteSingleReg(WOREVT0, timeout & 0x00FF);
		WriteSingleReg(WOREVT1, timeout >> 8);
	}
}

void set_eirp(int8_t eirp)
{
	uint8_t i;
	for( i = 0; i < sizeof(eirp_values); i++ )
	{
		if (eirp >= eirp_values[i]) //round the given eirp to a lower possible value
		{
			WriteSinglePATable(eirp_reg_values[i]);
			break;
		}
	}
}

int16_t calculate_rssi(int8_t rssi_raw)
{
	// CC1101 RSSI is 0.5 dBm units, signed byte
    int16_t rssi = (int16_t)rssi_raw;		//Convert to signed 16 bit
    rssi += 128;                      		//Make it positive...
    rssi >>= 1;                        		//...So division to 1 dBm units can be a shift...
    rssi -= 64 + RSSI_OFFSET;     			// ...and then rescale it, including offset

    return rssi;
}

//void dissable_autocalibration()
//{
//	WriteSingleReg(MCSM0, RADIO_MCSM0_FS_AUTOCAL_NEVER);
//}
//
//void enable_autocalibration()
//{
//	WriteSingleReg(MCSM0, RADIO_MCSM0_FS_AUTOCAL_4THIDLE);
//}
//
//void manual_calibration()
//{
//	Strobe(RF_SCAL);
//}

void phy_keep_radio_on(bool status)
{
	init_and_close_radio = !status;
}
