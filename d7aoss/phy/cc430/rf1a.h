/*
 * (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 *     	glenn.ergeerts@uantwerpen.be
 *     	maarten.weyn@uantwerpen.be
 *		alexanderhoet@gmail.com
 *
 */

#ifndef RF1A_H
#define RF1A_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define RADIO_INST_READY_WAIT()		while(!(RF1AIFCTL1 & RFINSTRIFG))
#define RADIO_DIN_READY_WAIT()		while(!(RF1AIFCTL1 & RFDINIFG))
#define RADIO_STAT_READY_WAIT()		while(!(RF1AIFCTL1 & RFSTATIFG))
#define RADIO_DOUT_READY_WAIT()		while(!(RF1AIFCTL1 & RFDOUTIFG))

typedef struct {
    uint8_t iocfg2;    // IOCFG2.GDO2_CFG output pin configuration
    uint8_t iofcg1;    // IOCFG1.GDO1_CFG output pin configuration
    uint8_t iofcg0;    // IOCFG0.GDO0_CFG output pin configuration
    uint8_t fifothr;   // RXFIFO and TXFIFO thresholds.
    uint8_t sync1;     // Sync word, high byte
    uint8_t sync0;     // Sync word, low byte
    uint8_t pktlen;    // Packet length.
    uint8_t pktctrl1;  // Packet automation control.
    uint8_t pktctrl0;  // Packet automation control.
    uint8_t addr;      // Device address.
    uint8_t channr;    // Channel number.
    uint8_t fsctrl1;   // Frequency synthesizer control.
    uint8_t fsctrl0;   // Frequency synthesizer control.
    uint8_t freq2;     // Frequency control word, high byte.
    uint8_t freq1;     // Frequency control word, middle byte.
    uint8_t freq0;     // Frequency control word, low byte.
    uint8_t mdmcfg4;   // Modem configuration - Channel Bandwidth and symbol rate (exponent)
    uint8_t mdmcfg3;   // Modem configuration - Symbol rate (mantissa)
    uint8_t mdmcfg2;   // Modem configuration.
    uint8_t mdmcfg1;   // Modem configuration.
    uint8_t mdmcfg0;   // Modem configuration.
    uint8_t deviatn;   // Modem deviation setting (when FSK modulation is enabled).
    uint8_t mcsm2;    // Main Radio Control State Machine configuration.
    uint8_t mcsm1;     // Main Radio Control State Machine configuration.
    uint8_t mcsm0;     // Main Radio Control State Machine configuration.
    uint8_t foccfg;    // Frequency Offset Compensation Configuration.
    uint8_t bscfg;     // Bit synchronization Configuration.
    uint8_t agcctrl2;  // AGC control.
    uint8_t agcctrl1;  // AGC control.
    uint8_t agcctrl0;  // AGC control.
    uint8_t worevt1;   // High byte Event0 timeout
    uint8_t worevt0;   // Low byte Event0 timeout
    uint8_t worctrl;    // Wake On Radio control
    uint8_t frend1;    // Front end RX configuration
    uint8_t frend0;    // Front end TX configuration
    uint8_t fscal3;    // Frequency synthesizer calibration.
    uint8_t fscal2;    // Frequency synthesizer calibration.
    uint8_t fscal1;    // Frequency synthesizer calibration.
    uint8_t fscal0;    // Frequency synthesizer calibration.
} RF_SETTINGS;

uint8_t Strobe(unsigned char strobe);
void ResetRadioCore (void);
void WriteRfSettings(RF_SETTINGS *pRfSettings);

uint8_t ReadSingleReg(uint8_t addr);
void WriteSingleReg(uint8_t addr, uint8_t value);
void ReadBurstReg(uint8_t addr, uint8_t* buffer, uint8_t count);
void WriteBurstReg(uint8_t addr, uint8_t* buffer, uint8_t count);
void WriteSinglePATable(uint8_t value);
void WriteBurstPATable(uint8_t* buffer, uint8_t count);

#ifdef __cplusplus
}
#endif

#endif /* RF1A_H */
