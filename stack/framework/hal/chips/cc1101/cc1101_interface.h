/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 *  	armin@otheruse.nl
 */

#ifndef CC1101_INTERFACE_H
#define CC1101_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "cc1101.h"
#include "cc1101_constants.h"

typedef enum {
	GDOLine0 = 0x2,
	GDOLine1 = 0x1,
	GDOLine2 = 0x0
} GDOLine;

typedef enum {
	GDOEdgeRising = 0x40,
	GDOEdgeFalling = 0x0,
} GDOEdge;


// RF settings for CC1101
typedef struct {
    uint8_t iocfg2;           // GDO2 Output Pin Configuration
    uint8_t iocfg1;           // GDO1 Output Pin Configuration
    uint8_t iocfg0;           // GDO0 Output Pin Configuration
    uint8_t fifothr;          // RX FIFO and TX FIFO Thresholds
    uint8_t sync1;            // Sync Word, High Byte
    uint8_t sync0;            // Sync Word, Low Byte
    uint8_t pktlen;           // Packet Length
    uint8_t pktctrl1;         // Packet Automation Control
    uint8_t pktctrl0;         // Packet Automation Control
    uint8_t addr;             // Device Address
    uint8_t channr;           // Channel Number
    uint8_t fsctrl1;          // Frequency Synthesizer Control
    uint8_t fsctrl0;          // Frequency Synthesizer Control
    uint8_t freq2;            // Frequency Control Word, High Byte
    uint8_t freq1;            // Frequency Control Word, Middle Byte
    uint8_t freq0;            // Frequency Control Word, Low Byte
    uint8_t mdmcfg4;          // Modem Configuration
    uint8_t mdmcfg3;          // Modem Configuration
    uint8_t mdmcfg2;          // Modem Configuration
    uint8_t mdmcfg1;          // Modem Configuration
    uint8_t mdmcfg0;          // Modem Configuration
    uint8_t deviatn;          // Modem Deviation Setting
    uint8_t mcsm2;            // Main Radio Control State Machine Configuration
    uint8_t mcsm1;            // Main Radio Control State Machine Configuration
    uint8_t mcsm0;            // Main Radio Control State Machine Configuration
    uint8_t foccfg;           // Frequency Offset Compensation Configuration
    uint8_t bscfg;            // Bit Synchronization Configuration
    uint8_t agcctrl2;         // AGC Control
    uint8_t agcctrl1;         // AGC Control
    uint8_t agcctrl0;         // AGC Control
    uint8_t worevt1;          // High Byte Event0 Timeout
    uint8_t worevt0;          // Low Byte Event0 Timeout
    uint8_t worctrl;          // Wake On Radio Control
    uint8_t frend1;           // Front End RX Configuration
    uint8_t frend0;           // Front End TX Configuration
    uint8_t fscal3;           // Frequency Synthesizer Calibration
    uint8_t fscal2;           // Frequency Synthesizer Calibration
    uint8_t fscal1;           // Frequency Synthesizer Calibration
    uint8_t fscal0;           // Frequency Synthesizer Calibration
} RF_SETTINGS;

void cc1101_interface_init(end_of_packet_isr_t end_of_packet_isr_cb, fifo_thr_isr_t fifo_threshold_isr_cb);
void cc1101_interface_set_interrupts_enabled(cc1101_gdOx_t gdOx, bool enabled);
void c1101_interface_set_edge_interrupt(cc1101_gdOx_t gdOx, uint8_t edge);
uint8_t cc1101_interface_strobe(uint8_t strobe_command);
void cc1101_interface_reset_radio_core(void);
void cc1101_interface_write_rfsettings(RF_SETTINGS* rfsettings);
uint8_t cc1101_interface_read_single_reg(uint8_t addr);
void cc1101_interface_write_single_reg(uint8_t addr, uint8_t value);
void cc1101_interface_read_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count);
void cc1101_interface_write_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count);
void cc1101_interface_write_single_patable(uint8_t value);
void cc1101_interface_write_burst_patable(uint8_t* buffer, uint8_t count);

#ifdef __cplusplus
}
#endif

#endif /* CC1101_INTERFACE_H */
