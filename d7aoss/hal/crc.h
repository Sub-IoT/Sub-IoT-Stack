/*
 * crc.h
 *
 *  Created on: 19-dec.-2012
 *      Author: Maarten Weyn
 */

#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>

uint16_t crc_calculate(uint8_t* data, uint8_t length);
uint16_t crc16(uint8_t *data_p, uint16_t length);
unsigned short CRCCCITT(unsigned char *data, uint8_t length, unsigned short seed, unsigned short final);


#endif /* CRC_H_ */
