/*
 *  Created on: May 9, 2013
 *  Authors:
 *  	glenn.ergeerts@artesis.be
 */
#include <stm32l1xx_crc.h>

#include <crc.h>

#define CRC16_POLY 0x8005
#define CRC_INIT 0xFFFF

uint16_t culCalcCRC(uint8_t crcData, uint16_t crcReg) {
	uint8_t i;
	for (i = 0; i < 8; i++) {
		if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
			crcReg = (crcReg << 1) ^ CRC16_POLY;
		else
			crcReg = (crcReg << 1);
		crcData <<= 1;
	}
	return crcReg;
} // culCalcCRC

uint16_t crc_calculate(uint8_t* data, uint8_t length) {
	// Init value for CRC calculation
	uint16_t checksum = CRC_INIT;
	uint8_t i;
	for (i = 0; i < length; i++) {
		checksum = culCalcCRC(data[i], checksum);
	}
	return checksum;
}
