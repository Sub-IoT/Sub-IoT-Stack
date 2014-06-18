// *************************************************************************************************
//
// Actual revision: $Revision: $
// Revision label:  $Name: $
// Revision state:  $State: $
//
// *************************************************************************************************
// Radio core access functions. Taken from TI reference code for CC430.
// *************************************************************************************************

#include <stdbool.h>
#include <stdint.h>

#include "cc1101_constants.h"
//#include "radio_spi_hw.h" // TODO provide HAL SPI API
#include "cc1101_core.h"

// *************************************************************************************************
// @fn          Strobe
// @brief       Send command to radio.
// @param       none
// @return      none
// *************************************************************************************************
uint8_t Strobe(uint8_t strobe) {
	uint8_t statusByte = 0;
	uint8_t strobe_tmp = strobe & 0x7F;
//	uint16_t int_state;

	// Check for valid strobe command
	if ((strobe_tmp >= RF_SRES) && (strobe_tmp <= RF_SNOP)) {
		radioSelect();
		statusByte = spiSendByte(strobe & 0x3f);
		radioDeselect();
	}

	return statusByte;
}

// *****************************************************************************
// @fn          ResetRadioCore
// @brief       Reset the radio core using RF_SRES command
// @param       none
// @return      none
// *****************************************************************************
void ResetRadioCore(void) {
	radioDeselect();
	delayuS(30);
	radioSelect();
	delayuS(30);
	radioDeselect();
	delayuS(45);
	Strobe(RF_SRES);                          // Reset the Radio Core
	Strobe(RF_SNOP);                          // Get Radio Status
}

// *****************************************************************************
// @fn          WriteRfSettings
// @brief       Write the RF configuration register settings
// @param       RF_SETTINGS* rfsettings  Pointer to the structure that holds the rf settings
// @return      none
// *****************************************************************************
void WriteRfSettings(RF_SETTINGS *rfsettings) {
	WriteBurstReg(IOCFG2, (unsigned char*) rfsettings, sizeof(RF_SETTINGS));
}


static uint8_t readreg(uint8_t regAddr) {
	radioSelect();
	spiSendByte((regAddr & 0x3F) | READ_SINGLE);
	uint8_t val = spiSendByte(0); // send dummy byte to receive reply
	radioDeselect();
	return val;
}

static uint8_t readstatus(uint8_t regAddr) {
	uint8_t ret, retCheck, data, data2;
	uint8_t addr = (regAddr & 0x3F) | READ_BURST;
	radioSelect();
	ret = spiSendByte(addr);
	data = spiSendByte(0); // send dummy byte to receive reply

	//See CC1101's Errata for SPI read errors
	while (true) {
		retCheck = spiSendByte(addr);
		data2 = spiSendByte(0);
		if (ret == retCheck && data == data2)
			break;
		else {
			ret = retCheck;
			data = data2;
		}
	}
	radioDeselect();
	return data;
}


// *****************************************************************************
// @fn          ReadSingleReg
// @brief       Read a single byte from the radio register
// @param       unsigned char addr      Target radio register address
// @return      unsigned char data_out  Value of byte that was read
// *****************************************************************************
uint8_t ReadSingleReg(uint8_t addr) {
	uint8_t value;

	// Check for valid configuration register address, PATABLE or FIFO
	if ((addr <= 0x2E) || (addr >= 0x3E)) {
		value = readreg(addr);
	}
	else {
		value = readstatus(addr);
	}
	return value;
}

// *****************************************************************************
// @fn          WriteSingleReg
// @brief       Write a single byte to a radio register
// @param       unsigned char addr      Target radio register address
// @param       unsigned char value     Value to be written
// @return      none
// *****************************************************************************
void WriteSingleReg(uint8_t addr, uint8_t value) {
	radioSelect();
	spiSendByte((addr & 0x3F));
	spiSendByte(value);
	radioDeselect();
}

// *****************************************************************************
// @fn          ReadBurstReg
// @brief       Read multiple bytes to the radio registers
// @param       unsigned char addr      Beginning address of burst read
// @param       unsigned char *buffer   Pointer to data table
// @param       unsigned char count     Number of bytes to be read
// @return      none
// *****************************************************************************
void ReadBurstReg(uint8_t addr, uint8_t* buffer, uint8_t count) {
	uint8_t _addr = (addr & 0x3F) | READ_BURST;
	radioSelect();
	spiSendByte(_addr);
	uint8_t i;
	for (i = 0; i < count; i++) {
		buffer[i] = spiSendByte(0); // send dummy byte to receive reply
	}
	radioDeselect();
}

// *****************************************************************************
// @fn          WriteBurstReg
// @brief       Write multiple bytes to the radio registers
// @param       unsigned char addr      Beginning address of burst write
// @param       unsigned char *buffer   Pointer to data table
// @param       unsigned char count     Number of bytes to be written
// @return      none
// *****************************************************************************
void WriteBurstReg(uint8_t addr, uint8_t* buffer, uint8_t count) {
	uint8_t _addr = (addr & 0x3F) | WRITE_BURST;
	radioSelect();
	spiSendByte(_addr);
	uint8_t i;
	for (i = 0; i < count; i++) {
		spiSendByte(buffer[i]);
	}
	radioDeselect();
}

// *****************************************************************************
// @fn          WritePATable
// @brief       Write data to power table
// @param       unsigned char value		Value to write
// @return      none
// *****************************************************************************
void WriteSinglePATable(uint8_t value) {
	WriteSingleReg(PATABLE, value);
}

// *****************************************************************************
// @fn          WritePATable
// @brief       Write to multiple locations in power table 
// @param       unsigned char *buffer	Pointer to the table of values to be written 
// @param       unsigned char count	Number of values to be written
// @return      none
// *****************************************************************************
void WriteBurstPATable(uint8_t* buffer, uint8_t count) {
	WriteBurstReg(PATABLE, buffer, count);
}
