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

#define CC430 0
#define CC1101 1
#define CC1120 2
#define RF CC1101

static uint8_t packet[7] = { 0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
//static uint8_t packet[14] = { 0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
static uint8_t buffer[128];
//
//phy_tx_cfg_t tx_cfg = {
//	{ 0, 0 },
//	0,
//	10,
//};
//
//void main(void) {
//    system_init(buffer, sizeof(buffer), buffer, sizeof(buffer));
//    phy_init();
//    //phy_keep_radio_on(1);
//    queue_push_u8_array(&tx_queue, packet, sizeof(packet));
//    while(1)
//    {
//        //if(!phy_is_tx_in_progress())
//        {
//            led_toggle(0);
//
//            //queue_push_u8_array(&tx_queue, packet, sizeof(packet));
//            phy_tx(&tx_cfg);
//        }
//    }
//
//    while(1);
//}

#if RF == CC430
	#include "../../d7aoss/phy/cc430/cc430_registers.h"
	#include "../../d7aoss/phy/cc430/cc430_phy.h"
#elif RF == CC1101
	#include "../../d7aoss/phy/cc1101/cc1101_constants.h"
	#include "../../d7aoss/phy/cc1101/cc1101_phy.h"
#elif RF ==  CC1120
	#include "cc1120.h"
#endif
/*
// Rf settings for CC430
RF_SETTINGS rfSettings = {
    0x0B,  // IOCFG2        GDO2 Output Configuration
    0x2D,  // IOCFG0        GDO0 Output Configuration
    0x47,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
    0x22,  // PKTCTRL0      Packet Automation Control
    0x06,  // FSCTRL1       Frequency Synthesizer Control
    0x10,  // FREQ2         Frequency Control Word, High Byte
    0xB1,  // FREQ1         Frequency Control Word, Middle Byte
    0x3A,  // FREQ0         Frequency Control Word, Low Byte
    0xF5,  // MDMCFG4       Modem Configuration
    0x83,  // MDMCFG3       Modem Configuration
    0x10,  // MDMCFG2       Modem Configuration
    0x15,  // DEVIATN       Modem Deviation Setting
    0x10,  // MCSM0         Main Radio Control State Machine Configuration
    0x16,  // FOCCFG        Frequency Offset Compensation Configuration
    0xFB,  // WORCTRL       Wake On Radio Control
    0xE9,  // FSCAL3        Frequency Synthesizer Calibration
    0x2A,  // FSCAL2        Frequency Synthesizer Calibration
    0x00,  // FSCAL1        Frequency Synthesizer Calibration
    0x1F,  // FSCAL0        Frequency Synthesizer Calibration
    0x81,  // TEST2         Various Test Settings
    0x35,  // TEST1         Various Test Settings
    0x09,  // TEST0         Various Test Settings
};
*/

#if RF == CC1120
RF_SETTINGS rfSettings = {
    0xB0,  // IOCFG3              GPIO3 IO Pin Configuration
    0x08,  // IOCFG2              GPIO2 IO Pin Configuration
    0xB0,  // IOCFG1              GPIO1 IO Pin Configuration
    0x09,  // IOCFG0              GPIO0 IO Pin Configuration
    0x08,  // SYNC_CFG1           Sync Word Detection Configuration Reg. 1
    0x99,  // DEVIATION_M         Frequency Deviation Configuration
    0x0D,  // MODCFG_DEV_E        Modulation Format and Frequency Deviation Configur..
    0x15,  // DCFILT_CFG          Digital DC Removal Configuration
    0x00,  // PREAMBLE_CFG1       Preamble Length Configuration Reg. 1
    0x3A,  // FREQ_IF_CFG         RX Mixer Frequency Configuration
    0x00,  // IQIC                Digital Image Channel Compensation Configuration
    0x02,  // CHAN_BW             Channel Filter Configuration
    0x06,  // MDMCFG1             General Modem Parameter Configuration Reg. 1
    0x45,  // MDMCFG0             General Modem Parameter Configuration Reg. 0
    0x99,  // SYMBOL_RATE2        Symbol Rate Configuration Exponent and Mantissa [1..
    0x99,  // SYMBOL_RATE1        Symbol Rate Configuration Mantissa [15:8]
    0x99,  // SYMBOL_RATE0        Symbol Rate Configuration Mantissa [7:0]
    0x3C,  // AGC_REF             AGC Reference Level Configuration
    0xEF,  // AGC_CS_THR          Carrier Sense Threshold Configuration
    0xA9,  // AGC_CFG1            Automatic Gain Control Configuration Reg. 1
    0xC0,  // AGC_CFG0            Automatic Gain Control Configuration Reg. 0
    0x00,  // FIFO_CFG            FIFO Configuration
    0x14,  // FS_CFG              Frequency Synthesizer Configuration
    0x07,  // PKT_CFG2            Packet Configuration Reg. 2
    0x00,  // PKT_CFG1            Packet Configuration Reg. 1
    0x20,  // PKT_CFG0            Packet Configuration Reg. 0
    0x79,  // PA_CFG0             Power Amplifier Configuration Reg. 0
    0x00,  // IF_MIX_CFG          IF Mix Configuration
    0x0A,  // TOC_CFG             Timing Offset Correction Configuration
    0x6C,  // FREQ2               Frequency Configuration [23:16]
    0x80,  // FREQ1               Frequency Configuration [15:8]
    0x00,  // FS_DIG1             Frequency Synthesizer Digital Reg. 1
    0x5F,  // FS_DIG0             Frequency Synthesizer Digital Reg. 0
    0x40,  // FS_CAL1             Frequency Synthesizer Calibration Reg. 1
    0x0E,  // FS_CAL0             Frequency Synthesizer Calibration Reg. 0
    0x03,  // FS_DIVTWO           Frequency Synthesizer Divide by 2
    0x33,  // FS_DSM0             FS Digital Synthesizer Module Configuration Reg. 0
    0x17,  // FS_DVC0             Frequency Synthesizer Divider Chain Configuration ..
    0x50,  // FS_PFD              Frequency Synthesizer Phase Frequency Detector Con..
    0x6E,  // FS_PRE              Frequency Synthesizer Prescaler Configuration
    0x14,  // FS_REG_DIV_CML      Frequency Synthesizer Divider Regulator Configurat..
    0xAC,  // FS_SPARE            Frequency Synthesizer Spare
    0xB4,  // FS_VCO0             FS Voltage Controlled Oscillator Configuration Reg..
    0x0E,  // XOSC5               Crystal Oscillator Configuration Reg. 5
    0x03,  // XOSC1               Crystal Oscillator Configuration Reg. 1
    0x08,  // SERIAL_STATUS       Serial Status
};
#else
// Address config = No address check
// Sync word qualifier mode = No preamble/sync
// PA ramping = false
// Base frequency = 433.999573
// Data rate = 38.3835
// RX filter BW = 101.562500
// Packet length = 255
// Manchester enable = false
// Packet length mode = Infinite packet length mode
// CRC enable = false
// TX power = 0
// Modulation format = ASK/OOK
// Device address = 0
// Whitening = false
// Channel spacing = 199.951172
// Data format = Random mode
// Deviation = 20.629883
// Preamble count = 4
// CRC autoflush = false
// Carrier frequency = 433.999573
// Modulated = false
// Channel number = 0
// Rf settings for CC1101
RF_SETTINGS rfSettings = {
    0x0B,  // IOCFG2              GDO2 Output Pin Configuration
    0x2E,  // IOCFG1              GDO1 Output Pin Configuration
    0x0C,  // IOCFG0              GDO0 Output Pin Configuration
    0x47,  // FIFOTHR             RX FIFO and TX FIFO Thresholds
    0xD3,  // SYNC1               Sync Word, High Byte
    0x91,  // SYNC0               Sync Word, Low Byte
    0xFF,  // PKTLEN              Packet Length
    0x04,  // PKTCTRL1            Packet Automation Control
    0x22,  // PKTCTRL0            Packet Automation Control
    0x00,  // ADDR                Device Address
    0x00,  // CHANNR              Channel Number
    0x08,  // FSCTRL1             Frequency Synthesizer Control
    0x00,  // FSCTRL0             Frequency Synthesizer Control
    0x10,  // FREQ2               Frequency Control Word, High Byte
    0xB1,  // FREQ1               Frequency Control Word, Middle Byte
    0x3A,  // FREQ0               Frequency Control Word, Low Byte
    0xCA,  // MDMCFG4             Modem Configuration
    0x83,  // MDMCFG3             Modem Configuration
    0xB0,  // MDMCFG2             Modem Configuration
    0x22,  // MDMCFG1             Modem Configuration
    0xF8,  // MDMCFG0             Modem Configuration
    0x35,  // DEVIATN             Modem Deviation Setting
    0x07,  // MCSM2               Main Radio Control State Machine Configuration
    0x30,  // MCSM1               Main Radio Control State Machine Configuration
    0x18,  // MCSM0               Main Radio Control State Machine Configuration
    0x16,  // FOCCFG              Frequency Offset Compensation Configuration
    0x6C,  // BSCFG               Bit Synchronization Configuration
    0x43,  // AGCCTRL2            AGC Control
    0x40,  // AGCCTRL1            AGC Control
    0x91,  // AGCCTRL0            AGC Control
    0x87,  // WOREVT1             High Byte Event0 Timeout
    0x6B,  // WOREVT0             Low Byte Event0 Timeout
    0xFB,  // WORCTRL             Wake On Radio Control
    0x56,  // FREND1              Front End RX Configuration
    0x11,  // FREND0              Front End TX Configuration
    0xE9,  // FSCAL3              Frequency Synthesizer Calibration
    0x2A,  // FSCAL2              Frequency Synthesizer Calibration
    0x00,  // FSCAL1              Frequency Synthesizer Calibration
    0x1F,  // FSCAL0              Frequency Synthesizer Calibration
    0x41,  // RCCTRL1             RC Oscillator Configuration
    0x00,  // RCCTRL0             RC Oscillator Configuration
    0x59,  // FSTEST              Frequency Synthesizer Calibration Control
    0x7F,  // PTEST               Production Test
    0x3F,  // AGCTEST             AGC Test
    0x81,  // TEST2               Various Test Settings
    0x35,  // TEST1               Various Test Settings
    0x09,  // TEST0               Various Test Settings
    0x00,  // PARTNUM             Chip ID
    0x04,  // VERSION             Chip ID
    0x00,  // FREQEST             Frequency Offset Estimate from Demodulator
    0x00,  // LQI                 Demodulator Estimate for Link Quality
    0x80,  // RSSI                Received Signal Strength Indication
    0x01,  // MARCSTATE           Main Radio Control State Machine State
    0x00,  // WORTIME1            High Byte of WOR Time
    0x00,  // WORTIME0            Low Byte of WOR Time
    0x00,  // PKTSTATUS           Current GDOx Status and Packet Status
    0x94,  // VCO_VC_DAC          Current Setting from PLL Calibration Module
    0x00,  // TXBYTES             Underflow and Number of Bytes
    0x00,  // RXBYTES             Overflow and Number of Bytes
    0x00,  // RCCTRL1_STATUS      Last RC Oscillator Calibration Result
    0x00,  // RCCTRL0_STATUS      Last RC Oscillator Calibration Result
};
#endif

void log_state()
{
	int s = ReadSingleReg(MARCSTATE);
	log_print_string("MARCSTATE 0x%x", s);
}

void main()
{
	system_init(buffer, sizeof(buffer), buffer, sizeof(buffer));
#if RF == CC1101 || RF == CC1120
	spi_init();
#endif

	ResetRadioCore();
	int p = ReadSingleReg(PARTNUM);
	log_print_string("PARTNUM 0x%x", p);
	p = ReadSingleReg(VERSION);
	log_print_string("VERSION 0x%x", p);
	log_print_string("started");

	WriteRfSettings(&rfSettings);
//	WriteSingleReg(IOCFG2,0x0B);    //GDO2 Output Pin Configuration
//	WriteSingleReg(IOCFG0,0x0C);    //GDO0 Output Pin Configuration
//	WriteSingleReg(FIFOTHR,0x47);   //RX FIFO and TX FIFO Thresholds
//	WriteSingleReg(PKTCTRL0,0x22);  //Packet Automation Control
//	WriteSingleReg(FSCTRL1,0x08);   //Frequency Synthesizer Control
//	WriteSingleReg(FREQ2,0x10);     //Frequency Control Word, High Byte
//	WriteSingleReg(FREQ1,0xB1);     //Frequency Control Word, Middle Byte
//	WriteSingleReg(FREQ0,0x3A);     //Frequency Control Word, Low Byte
//	WriteSingleReg(MDMCFG4,0xCA);   //Modem Configuration
//	WriteSingleReg(MDMCFG3,0x83);   //Modem Configuration
//	WriteSingleReg(MDMCFG2,0xB0);   //Modem Configuration
//	WriteSingleReg(DEVIATN,0x35);   //Modem Deviation Setting
//	WriteSingleReg(MCSM0,0x18);     //Main Radio Control State Machine Configuration
//	WriteSingleReg(FOCCFG,0x16);    //Frequency Offset Compensation Configuration
//	WriteSingleReg(AGCCTRL2,0x43);  //AGC Control
//	WriteSingleReg(WORCTRL,0xFB);   //Wake On Radio Control
//	WriteSingleReg(FREND0,0x11);    //Front End TX Configuration
//	WriteSingleReg(FSCAL3,0xE9);    //Frequency Synthesizer Calibration
//	WriteSingleReg(FSCAL2,0x2A);    //Frequency Synthesizer Calibration
//	WriteSingleReg(FSCAL1,0x00);    //Frequency Synthesizer Calibration
//	WriteSingleReg(FSCAL0,0x1F);    //Frequency Synthesizer Calibration
//	WriteSingleReg(TEST2,0x81);     //Various Test Settings
//	WriteSingleReg(TEST1,0x35);     //Various Test Settings
//	WriteSingleReg(TEST0,0x09);     //Various Test Settings
//	WriteSingleReg(RSSI,0x80);      //Received Signal Strength Indication
//	WriteSingleReg(MARCSTATE,0x01); //Main Radio Control State Machine State
//	WriteSingleReg(VCO_VC_DAC,0x94);//Current Setting from PLL Calibration Module

	//phy_keep_radio_on(true);

	Strobe(RF_SIDLE);
	Strobe(RF_SFTX);

	//uint8_t spectrum_id[2] = { 4, 0 };
	//phy_translate_and_set_settings(spectrum_id, 0);

	//set_length_infinite(false);
	//WriteSingleReg(PKTLEN, 7);

	Strobe(RF_STX);

	//WriteSingleReg(PKTCTRL0, 0x22);

	//WriteSingleReg(FIFOTHR, RADIO_FIFOTHR_FIFO_THR_17_48);

	//WriteBurstReg(RF_TXFIFOWR, packet, 7);




	/*
	ResetRadioCore();
	log_state();
	Strobe(RF_SIDLE);
	WriteRfSettings(&rfSettings);
	log_print_string("wrote settings");
	log_state();
	Strobe(RF_STX);
	log_print_string("strobed TX");
	*/

	int i;
	for(i = 0; i < 50;i++)
	{
		log_state();
	}



	while(1);
}

//#include "../../d7aoss/phy/cc1101/cc1101_core.h"
//#include "../../d7aoss/phy/cc1101/cc1101_registers.h"
//#include "../../d7aoss/phy/cc1101/cc1101_constants.h"
//#include "../../d7aoss/phy/cc1101/cc1101_phy.h"

//// Rf settings for CC1101
//RF_SETTINGS rfSettingsCc1101 = {
//    0x06,  // IOCFG0        GDO0 Output Pin Configuration
//    0x05,  // PKTCTRL0      Packet Automation Control
//    0x0C,  // FSCTRL1       Frequency Synthesizer Control
//    0x10,  // FREQ2         Frequency Control Word, High Byte
//    0xB1,  // FREQ1         Frequency Control Word, Middle Byte
//    0x3B,  // FREQ0         Frequency Control Word, Low Byte
//    0x2D,  // MDMCFG4       Modem Configuration
//    0x3B,  // MDMCFG3       Modem Configuration
//    0x13,  // MDMCFG2       Modem Configuration
//    0x62,  // DEVIATN       Modem Deviation Setting
//    0x18,  // MCSM0         Main Radio Control State Machine Configuration
//    0x1D,  // FOCCFG        Frequency Offset Compensation Configuration
//    0x1C,  // BSCFG         Bit Synchronization Configuration
//    0xC7,  // AGCCTRL2      AGC Control
//    0x00,  // AGCCTRL1      AGC Control
//    0xB0,  // AGCCTRL0      AGC Control
//    0xFB,  // WORCTRL       Wake On Radio Control
//    0xB6,  // FREND1        Front End RX Configuration
//    0xEA,  // FSCAL3        Frequency Synthesizer Calibration
//    0x2A,  // FSCAL2        Frequency Synthesizer Calibration
//    0x00,  // FSCAL1        Frequency Synthesizer Calibration
//    0x1F,  // FSCAL0        Frequency Synthesizer Calibration
//    0x09,  // TEST0         Various Test Settings
//};
//
//void main()
//{
//	system_init(buffer, sizeof(buffer), buffer, sizeof(buffer));
//	spi_init();
//	ResetRadioCore();
//	WriteRfSettings(&rfSettings);
//	Strobe(RF_STX);
//	while(1);
//}
