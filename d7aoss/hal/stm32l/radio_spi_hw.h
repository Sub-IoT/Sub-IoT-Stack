/*
 * radio_spi.h
 *
 *  Created on: Aug 6, 2012
 *      Author: armin
 */

#ifndef RADIO_SPI_H_
#define RADIO_SPI_H_

#include <stdint.h>

void spiInit();
uint8_t spiSendByte(uint8_t data);

void radioSelect(void);
void radioDeselect(void);

void radioDisableGDO0Interrupt(void);
void radioEnableGDO0Interrupt(void);
void radioDisableGDO2Interrupt(void);
void radioEnableGDO2Interrupt(void);
void radioConfigureInterrupt(void);
void radioClearInterruptPendingLines();

#endif /* RADIO_SPI_H_ */
