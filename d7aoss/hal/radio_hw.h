/*
 * radio_spi.h
 *
 *  Created on: Aug 6, 2012
 *      Author: armin
 */

#ifndef RADIO_H_
#define RADIO_H_

#include <stdint.h>

#define RADIO_PORT_GDO0     gpioPortD
#define RADIO_PIN_GDO0      7

#define RADIO_PORT_GDO2     gpioPortC
#define RADIO_PIN_GDO2      6

void CC1101_ISR                     ( GDOLine gdo, GDOEdge edge );

void radioDisableGDO0Interrupt      ( void );
void radioEnableGDO0Interrupt       ( void );
void radioDisableGDO2Interrupt      ( void );
void radioEnableGDO2Interrupt       ( void );
void radioConfigureInterrupt        ( void );
void radioClearInterruptPendingLines( void );

#endif /* RADIO_SPI_H_ */
