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
// Whitening = false
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
    0x47,  // FIFOTHR             RX FIFO and TX FIFO Thresholds
    0xD3,  // SYNC1               Sync Word, High Byte
    0x91,  // SYNC0               Sync Word, Low Byte
    0xFF,  // PKTLEN              Packet Length
    0x04,  // PKTCTRL1            Packet Automation Control
    0x22,  // PKTCTRL0            Packet Automation Control
    0x00,  // ADDR                Device Address
    0x00,  // CHANNR              Channel Number
    0x06,  // FSCTRL1             Frequency Synthesizer Control
    0x00,  // FSCTRL0             Frequency Synthesizer Control
    0x10,  // FREQ2               Frequency Control Word, High Byte
    0xA8,  // FREQ1               Frequency Control Word, Middle Byte
    0xF5,  // FREQ0               Frequency Control Word, Low Byte
    0xFB,  // MDMCFG4             Modem Configuration
    0x18,  // MDMCFG3             Modem Configuration
    0x10,  // MDMCFG2             Modem Configuration
    0x22,  // MDMCFG1             Modem Configuration
    0xF8,  // MDMCFG0             Modem Configuration
    0x50,  // DEVIATN             Modem Deviation Setting
    0x07,  // MCSM2               Main Radio Control State Machine Configuration
    0x30,  // MCSM1               Main Radio Control State Machine Configuration
    0x18,  // MCSM0               Main Radio Control State Machine Configuration
    0x16,  // FOCCFG              Frequency Offset Compensation Configuration
    0x6C,  // BSCFG               Bit Synchronization Configuration
    0x03,  // AGCCTRL2            AGC Control
    0x40,  // AGCCTRL1            AGC Control
    0x91,  // AGCCTRL0            AGC Control
    0x87,  // WOREVT1             High Byte Event0 Timeout
    0x6B,  // WOREVT0             Low Byte Event0 Timeout
    0xFB,  // WORCTRL             Wake On Radio Control
    0x56,  // FREND1              Front End RX Configuration
    0x10,  // FREND0              Front End TX Configuration
    0xEA,  // FSCAL3              Frequency Synthesizer Calibration
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
