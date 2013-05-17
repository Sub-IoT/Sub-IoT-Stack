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

#endif /* CRC_H_ */
