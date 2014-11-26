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
 *
 */


#ifndef CC430_REGISTERS_H
#define CC430_REGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// Radio Core Interrupt Flags
#define RFIFG_FLAG_IOCFG0        	0x0001  // RFIFG0
#define RFIFG_FLANK_IOCFG0       	0x0000
#define RFIFG_FLAG_IOCFG1        	0x0002  // RFIFG1
#define RFIFG_FLANK_IOCFG1       	0x0000
#define RFIFG_FLAG_IOCFG2        	0x0004  // RFIFG2
#define RFIFG_FLANK_IOCFG2       	0x0000
#define RFIFG_FLAG_RXFilled      	0x0008  // RFIFG3 Positive Edge
#define RFIFG_FLANK_RXFilled      	0x0000
#define RFIFG_FLAG_RXDrained  		0x0008  // RFIFG3 Negative Edge
#define RFIFG_FLANK_RXDrained  		0x0008
#define RFIFG_FLAG_RXFilledOrEOP 	0x0010  // RFIFG4 Positive Edge
#define RFIFG_FLANK_RXFilledOrEOP   0x0000
#define RFIFG_FLAG_RXEmpty          0x0010  // RFIFG4 Negative Edge
#define RFIFG_FLANK_RXEmpty    		0x0010
#define RFIFG_FLAG_TXFilled      	0x0020  // RFIFG5 Positive Edge
#define RFIFG_FLANK_TXFilled      	0x0000
#define RFIFG_FLAG_TXBelowThresh    0x0020  // RFIFG5 Negative Edge
#define RFIFG_FLANK_TXBelowThresh   0x0020
#define RFIFG_FLAG_TXFull           0x0040  // RFIFG6 Positive Edge
#define RFIFG_FLANK_TXFull          0x0000
#define RFIFG_FLAG_TXDrained     	0x0040  // RFIFG6 Negative Edge
#define RFIFG_FLANK_TXDrained     	0x0040
#define RFIFG_FLAG_RXOverflow       0x0080  // RFIFG7 Positive Edge
#define RFIFG_FLANK_RXOverflow      0x0000
#define RFIFG_FLAG_RXFlushed        0x0080  // RFIFG7 Negative Edge
#define RFIFG_FLANK_RXFlushed       0x0080
#define RFIFG_FLAG_TXUnderflow    	0x0100  // RFIFG8 Positive Edge
#define RFIFG_FLANK_TXUnderflow   	0x0000
#define RFIFG_FLAG_TXFlushed        0x0100  // RFIFG8 Negative Edge
#define RFIFG_FLANK_TXFlushed       0x0100
#define RFIFG_FLAG_SyncWord         0x0200  // RFIFG9 Positive Edge
#define RFIFG_FLANK_SyncWord        0x0000
#define RFIFG_FLAG_EndOfPacket      0x0200  // RFIFG9 Negative Edge
#define RFIFG_FLANK_EndOfPacket     0x0200
#define RFIFG_FLAG_CRCOK         	0x0400  // RFIFG10 Positive Edge
#define RFIFG_FLANK_CRCOK         	0x0000
#define RFIFG_FLAG_RXFirstByte      0x0400  // RFIFG10 Negative Edge
#define RFIFG_FLANK_RXFirstByte     0x0400
#define RFIFG_FLAG_PQTReached       0x0800  // RFIFG11 Positive Edge
#define RFIFG_FLANK_PQTReached      0x0000
#define RFIFG_FLAG_LPW              0x0800  // RFIFG11 Negative Edge
#define RFIFG_FLANK_LPW             0x0800
#define RFIFG_FLAG_ClearChannel     0x1000  // RFIFG12 Positive Edge
#define RFIFG_FLANK_ClearChannel    0x0000
#define RFIFG_FLAG_RSSIAboveThr     0x1000  // RFIFG12 Negative Edge
#define RFIFG_FLANK_RSSIAboveThr    0x1000
#define RFIFG_FLAG_CarrierSense     0x2000  // RFIFG13 Positive Edge
#define RFIFG_FLANK_CarrierSense    0x0000
#define RFIFG_FLAG_CSRSSIAboveThr   0x2000  // RFIFG13 Negative Edge
#define RFIFG_FLANK_CSRSSIAboveThr  0x2000
#define RFIFG_FLAG_WOREvent0        0x4000  // RFIFG14 Positive Edge
#define RFIFG_FLANK_WOREvent0       0x0000
#define RFIFG_FLAG_WOREvent0ACLK    0x4000  // RFIFG14 Negative Edge
#define RFIFG_FLANK_WOREvent0ACLK   0x4000
#define RFIFG_FLAG_WORevent1        0x8000  // RFIFG15 Positive Edge
#define RFIFG_FLANK_WORevent1       0x0000
#define RFIFG_FLAG_OscStable        0x8000  // RFIFG15 Negative Edge
#define RFIFG_FLANK_OscStable       0x8000
#define RFIFG_FLAG_AllPositiveEdge  0xFFFF
#define RFIFG_FLANK_AllPositiveEdge 0x0000
#define RFIFG_FLAG_AllNegativeEdge  0xFFFF
#define RFIFG_FLANK_AllNegativeEdge 0xFFFF

// IOCFGx.GDOx_CFG
#define RADIO_GDO2_VALUE                0x29            // IOCFG2.GDO2_CFG - RF_RDYn Do not change! Is used in the strobe command.
#define RADIO_GDO1_VALUE                0x1E            // IOCFG1.GDO1_CFG - RSSI Valid Do not change! Is used in the get_rssi command.
#define RADIO_GDO0_VALUE                0x1F            // IOCFG0.GDO0_CFG

// FIFO THR
#define RADIO_FIFOTHR_CLOSE_IN_RX_0db   (0 << 4)
#define RADIO_FIFOTHR_CLOSE_IN_RX_6db   (1 << 4)
#define RADIO_FIFOTHR_CLOSE_IN_RX_12db  (2 << 4)
#define RADIO_FIFOTHR_CLOSE_IN_RX_18db  (3 << 4)
#define RADIO_FIFOTHR_FIFO_THR_61_4     (0)             // FIFOTHR.FIFO_THR 61B TX /  4B RX
#define RADIO_FIFOTHR_FIFO_THR_33_32    (7)             // FIFOTHR.FIFO_THR 33B TX / 32B RX
#define RADIO_FIFOTHR_FIFO_THR_17_48    (11)             // FIFOTHR.FIFO_THR 17B TX / 48B RX
#define RADIO_FIFOTHR_FIFO_THR_1_64     (15)            // FIFOTHR.FIFO_THR  1B TX / 64B RX

#define RADIO_SYNC1      				0xE6            // SYNC1
#define RADIO_SYNC0      				0xD0            // SYNC0

#define RADIO_PKTLEN                    0xFF            // PKTLEN (default)

#define RADIO_PKTCTRL1_PQT(VAL)         ((VAL&7) << 5)  // PKTCTRL1.PQT must be at least VAL * 4 good bits
#define RADIO_PKTCTRL1_CRC_AUTOFLUSH    (1 << 3)        // Enable automatic flush of RX FIFO when CRC in not OK. This requires that only one packet is in the RX FIFO and that packet length is limited to the RX FIFO size.
#define RADIO_PKTCTRL1_APPEND_STATUS    (1 << 2)        // When enabled, two status bytes are appended to the payload of the packet. The status bytes contain RSSI and LQI values, as well as CRC OK.
#define RADIO_PKTCTRL1_ADR_CHK_NONE     (0)             // Controls address check configuration of received packages.
#define RADIO_PKTCTRL1_ADR_CHK_ON       (1)
#define RADIO_PKTCTRL1_ADR_CHK_ON_00    (2)
#define RADIO_PKTCTRL1_ADR_CHK_ON_FF    (3)


#define RADIO_PKTCTRL0_WHITE_DATA       (1 << 6)        // PKTCTRL0.WHITE_DATA is ON!
#define RADIO_PKTCTRL0_PKT_FOR_NORMAL   (0 << 4)
#define RADIO_PKTCTRL0_PKT_FOR_SERIAL   (1 << 4)
#define RADIO_PKTCTRL0_PKT_FOR_RANDOM   (2 << 4)
#define RADIO_PKTCTRL0_PKT_FOR_ASYNC    (3 << 4)
#define RADIO_PKTCTRL0_CRC              (1 << 2)        // PKTCTRL0.CRC_EN is OFF!
#define RADIO_PKTCTRL0_LENGTH_FIXED     (0)             // PKTCTRL0.LENGTH_CONFIG is fixed mode
#define RADIO_PKTCTRL0_LENGTH_VAR       (1)             // PKTCTRL0.LENGTH_CONFIG is variable (default)
#define RADIO_PKTCTRL0_LENGTH_INF       (2)             // PKTCTRL0.LENGTH_CONFIG is infinite

#define RADIO_ADDR                      0x00

#define RADIO_CHAN                      0              // CHANNR (default)

#define RADIO_FREQ_IF                   0x06            // FSCTRL1.FREQ_IF - (152.34375kHz) fIF = (fRFXT2/210) × FREQ_IF
#define RADIO_FREQOFF                   0               // FSCTRL0 Frequency offset

#define RADIO_FREQ_433                  ((uint32_t)0x0010A900)   // pchan 1 fc = 433.164062 MHz
#define RADIO_FREQ_868                  ((uint32_t)0x00216267)   // pchan 1 fc = 867.999994 MHz
#define RADIO_FREQ_915                  ((uint32_t)0x0022B13B)   // pchan 1 fc = 901.999969 MHz
#define RADIO_FREQ2                     (uint8_t)(RADIO_FREQ_433>>16 & 0xFF)
#define RADIO_FREQ1                     (uint8_t)(RADIO_FREQ_433>>8 & 0xFF)
#define RADIO_FREQ0                     (uint8_t)(RADIO_FREQ_433 & 0xFF)

#define RADIO_MDMCFG4_CHANBW_E(VAL)     ((VAL&3)<<6)    // 2 - 162.5 Khz - MDMCFG4 Channel Bandwith Exponent BDW = fxosc / (8 x (4 + chbw_M) x 2^chbw_E)
#define RADIO_MDMCFG4_CHANBW_M(VAL)     ((VAL&3)<<4)    // 1 - 162.5 Khz - MDMCFG4 Channel Bandwith Mantise
#define RADIO_MDMCFG4_DRATE_E(VAL)      (VAL&15)        // 11  - MDMCFG4 - Data Rate Exponent

#define RADIO_MDMCFG3_DRATE_M(VAL)      (VAL)           // 24 - 55.542 Kbaud - MDMCFG3 - Data Rate Mantissa DRate = ((256+DRATE_M)x2^DRATE_E) * fXosc / 2^28

#define RADIO_MDMCFG2_DEM_DCFILT_OFF    (1 << 7)        // Disable digital dc blocking filter before demodulator. (current optimized)
#define RADIO_MDMCFG2_DEM_DCFILT_ON     (0 << 7)        // Enable digital dc blocking filter before demodulator. (better sensitivity)
#define RADIO_MDMCFG2_MOD_FORMAT_2FSK   (0 << 4)        // Modulation: 2-FSK
#define RADIO_MDMCFG2_MOD_FORMAT_GFSK   (1 << 4)        // Modulation: 2-GSK
#define RADIO_MDMCFG2_MOD_FORMAT_ASK    (3 << 4)        // Modulation: ASK/OOK
#define RADIO_MDMCFG2_MOD_FORMAT_MSK    (7 << 4)        // Modulation: MSK
#define RADIO_MDMCFG2_MANCHESTER_EN     (1 << 3)        // Enables manchester encoding
#define RADIO_MDMCFG2_SYNC_MODE_NONE    (0)             // Sync Word Qualifier: No preamble/sync
#define RADIO_MDMCFG2_SYNC_MODE_15in16  (1)             // Sync Word Qualifier: 15/16 sync words detected
#define RADIO_MDMCFG2_SYNC_MODE_16in16  (2)             // Sync Word Qualifier: 16/16 sync words detected
#define RADIO_MDMCFG2_SYNC_MODE_30in32  (3)             // Sync Word Qualifier: 30/32 sync words detected
#define RADIO_MDMCFG2_SYNC_MODE_CSONLY  (4)             // Sync Word Qualifier: no preamble/sync carrier sense, above threshold
#define RADIO_MDMCFG2_SYNC_MODE_15in16CS  (5)           // Sync Word Qualifier: 15/16 sync words detected + Carrier sense
#define RADIO_MDMCFG2_SYNC_MODE_16in16CS  (6)           // Sync Word Qualifier: 15/16 sync words detected + Carrier sense
#define RADIO_MDMCFG2_SYNC_MODE_30in32CS  (7)           // Sync Word Qualifier: 15/16 sync words detected + Carrier sense

#define RADIO_MDMCFG1_NUM_PREAMBLE_2B     (0<<4)        // 2 bytes preamble
#define RADIO_MDMCFG1_NUM_PREAMBLE_3B     (1<<4)
#define RADIO_MDMCFG1_NUM_PREAMBLE_4B     (2<<4)
#define RADIO_MDMCFG1_NUM_PREAMBLE_6B     (3<<4)
#define RADIO_MDMCFG1_NUM_PREAMBLE_8B     (4<<4)
#define RADIO_MDMCFG1_NUM_PREAMBLE_12B    (5<<4)
#define RADIO_MDMCFG1_NUM_PREAMBLE_16B    (6<<4)
#define RADIO_MDMCFG1_NUM_PREAMBLE_24B    (7<<4)
#define RADIO_MDMCFG1_CHANSPC_E(VAL)      (VAL&3)       // 2 - Channel spacing Exponent

#define RADIO_MDMCFG0_CHANSPC_M(VAL)      (VAL)         // 16 - 107.910 Khz Channel spacing Mantissa, chanspc = fosxc/2^18 x (256+chansp_m) x 2 ^chansp_e

#define RADIO_DEVIATN_E(VAL)                   ((VAL&7) << 4)// 5 - Deviation Exponent
#define RADIO_DEVIATN_M(VAL)                   (VAL&7)       // 0 - 50.78 Khz) Deviation Mantissa - dev = fosxc/2^17 x (8+dev_m) x 2 ^dev_e

#define RADIO_MCSM2_RX_TIME_RSSI          (1 << 4)      // Direct RX termination based on RSSI measurement (carrier sense)
#define RADIO_MCSM2_RX_TIME_QUAL          (1 << 3)
#define RADIO_MCSM2_RX_TIME(VAL)          (VAL&3)         // Timeout for sync word search

#define RADIO_MCSM1_CCA_ALWAYS             (0 << 4)    // MCSM1.CCA_MODE Always
#define RADIO_MCSM1_CCA_RSSILOW            (1 << 4)    // MCSM1.CCA_MODE If RSSI below threshold
#define RADIO_MCSM1_CCA_UNLESSRX           (2 << 4)    // MCSM1.CCA_MODE Unless currently receiving a packet
#define RADIO_MCSM1_CCA_RSSILOWRX          (3 << 4)    // Iff RSSI below threshold unless currently receiving a packet
#define RADIO_MCSM1_RXOFF_MODE_IDLE        (0 << 2)    // MCSM1.CCA_MODE Idle after packet received
#define RADIO_MCSM1_RXOFF_MODE_FSTXON      (1 << 2)    // MCSM1.CCA_MODE fstxon after packet received
#define RADIO_MCSM1_RXOFF_MODE_TX          (2 << 2)    // MCSM1.CCA_MODE tx after after packet received
#define RADIO_MCSM1_RXOFF_MODE_RX          (3 << 2)    // MCSM1.CCA_MODE stay in rx after packet received
#define RADIO_MCSM1_TXOFF_MODE_IDLE        (0)         // MCSM1.CCA_MODE idle after tx
#define RADIO_MCSM1_TXOFF_MODE_FSTXON      (1)         // MCSM1.CCA_MODE fstxon after tx
#define RADIO_MCSM1_TXOFF_MODE_TX          (2)         // MCSM1.CCA_MODE stay in tx
#define RADIO_MCSM1_TXOFF_MODE_RX          (3)         // MCSM1.CCA_MODE rx after tx

#define RADIO_MCSM0_FS_AUTOCAL_NEVER      (0 << 4)    // MCSM0.AUTOCAL
#define RADIO_MCSM0_FS_AUTOCAL_FROMIDLE   (1 << 4)    // MCSM0.AUTOCAL
#define RADIO_MCSM0_FS_AUTOCAL_TOIDLE     (2 << 4)    // MCSM0.AUTOCAL
#define RADIO_MCSM0_FS_AUTOCAL_4THIDLE    (3 << 4)    // MCSM0.AUTOCAL
#define RADIO_MCSM0_PI_CTRL_EN            (1 << 1)    // Enables the pin radio control option
#define RADIO_XOSX_FORCE_ON               (1)         // Force the RF XT2 oscillator to stay on in the SLEEP state.

#define RADIO_FOCCFG_FOC_BS_CS_GATE       (1 << 5)    // If set, the demodulator freezes the frequency offset compensation and clock recovery feedback loops until the CS signal goes high.
#define RADIO_FOCCFG_FOC_PRE_K_1K         (0 << 3)
#define RADIO_FOCCFG_FOC_PRE_K_2K         (1 << 3)
#define RADIO_FOCCFG_FOC_PRE_K_3K         (2 << 3)
#define RADIO_FOCCFG_FOC_PRE_K_4K         (3 << 3)
#define RADIO_FOCCFG_FOC_POST_K_EQ        (0 << 2)
#define RADIO_FOCCFG_FOC_POST_K_HALFK     (1 << 2)
#define RADIO_FOCCFG_FOC_LIMIT_0          (0)
#define RADIO_FOCCFG_FOC_LIMIT_8THBW      (1)
#define RADIO_FOCCFG_FOC_LIMIT_4THBW      (2)
#define RADIO_FOCCFG_FOC_LIMIT_HALFBW     (3)

#define RADIO_BSCFG_BS_PRE_KI_1KI         (0 << 6)
#define RADIO_BSCFG_BS_PRE_KI_2KI         (1 << 6)
#define RADIO_BSCFG_BS_PRE_KI_3KI         (2 << 6)
#define RADIO_BSCFG_BS_PRE_KI_4KI         (3 << 6)
#define RADIO_BSCFG_BS_PRE_KP_1KP         (0 << 4)
#define RADIO_BSCFG_BS_PRE_KP_2KP         (1 << 4)
#define RADIO_BSCFG_BS_PRE_KP_3KP         (2 << 4)
#define RADIO_BSCFG_BS_PRE_KP_4KP         (3 << 4)
#define RADIO_BSCFG_BS_POST_KI_EQ         (0 << 3)
#define RADIO_BSCFG_BS_POST_KI_1KP        (1 << 3)
#define RADIO_BSCFG_BS_POST_KP_EQ         (0 << 2)
#define RADIO_BSCFG_BS_POST_KP_1KP        (1 << 2)
#define RADIO_BSCFG_BS_LIMIT_0            (0)
#define RADIO_BSCFG_BS_LIMIT_32NDOFFSET   (1)
#define RADIO_BSCFG_BS_LIMIT_16THOFFSET   (2)
#define RADIO_BSCFG_BS_LIMIT_8THOFFSET    (3)


#define RADIO_AGCCTRL2_MAX_DVGA_GAIN_ALL   (0<<6)
#define RADIO_AGCCTRL2_MAX_DVGA_GAIN_SUB1  (1<<6)
#define RADIO_AGCCTRL2_MAX_DVGA_GAIN_SUB2  (2<<6)
#define RADIO_AGCCTRL2_MAX_DVGA_GAIN_SUB3  (3<<6)
#define RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB0   (0<<3)
#define RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB26  (1<<3)
#define RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB61  (2<<3)
#define RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB74  (3<<3)
#define RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB92  (4<<3)
#define RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB115 (5<<3)
#define RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB146 (6<<3)
#define RADIO_AGCCTRL2_MAX_LNA_GAIN_SUB171 (7<<3)
#define RADIO_AGCCTRL2_MAX_MAGN_TARGET_24  (0)
#define RADIO_AGCCTRL2_MAX_MAGN_TARGET_27  (1)
#define RADIO_AGCCTRL2_MAX_MAGN_TARGET_30  (2)
#define RADIO_AGCCTRL2_MAX_MAGN_TARGET_33  (3)
#define RADIO_AGCCTRL2_MAX_MAGN_TARGET_36  (4)
#define RADIO_AGCCTRL2_MAX_MAGN_TARGET_38  (5)
#define RADIO_AGCCTRL2_MAX_MAGN_TARGET_40  (6)
#define RADIO_AGCCTRL2_MAX_MAGN_TARGET_42  (7)

#define RADIO_AGCCTRL1_AGC_LNA_PRIORITY    (1<<6)
#define RADIO_AGCCTRL1_CS_REL_THR_DISABLED (0<<4)
#define RADIO_AGCCTRL1_CS_REL_THR_PLUS6    (1<<4)
#define RADIO_AGCCTRL1_CS_REL_THR_PLUS10   (2<<4)
#define RADIO_AGCCTRL1_CS_REL_THR_PLUS14   (3<<4)
#define RADIO_AGCCTRL1_CS_ABS_THR_FLAT     (0)
#define RADIO_AGCCTRL1_CS_ABS_THR_PLUS1    (1)
#define RADIO_AGCCTRL1_CS_ABS_THR_PLUS2    (2)
#define RADIO_AGCCTRL1_CS_ABS_THR_PLUS3    (3)
#define RADIO_AGCCTRL1_CS_ABS_THR_PLUS4    (4)
#define RADIO_AGCCTRL1_CS_ABS_THR_PLUS5    (5)
#define RADIO_AGCCTRL1_CS_ABS_THR_PLUS6    (6)
#define RADIO_AGCCTRL1_CS_ABS_THR_PLUS7    (7)
#define RADIO_AGCCTRL1_CS_ABS_THR_SUB8     (8)
#define RADIO_AGCCTRL1_CS_ABS_THR_SUB7     (9)
#define RADIO_AGCCTRL1_CS_ABS_THR_SUB6     (10)
#define RADIO_AGCCTRL1_CS_ABS_THR_SUB5     (11)
#define RADIO_AGCCTRL1_CS_ABS_THR_SUB4     (12)
#define RADIO_AGCCTRL1_CS_ABS_THR_SUB3     (13)
#define RADIO_AGCCTRL1_CS_ABS_THR_SUB2     (14)
#define RADIO_AGCCTRL1_CS_ABS_THR_SUB1     (15)

#define RADIO_AGCCTRL0_HYST_LEVEL_NONE     (0<<6)
#define RADIO_AGCCTRL0_HYST_LEVEL_LOW      (1<<6)
#define RADIO_AGCCTRL0_HYST_LEVEL_MED      (2<<6)
#define RADIO_AGCCTRL0_HYST_LEVEL_HIGH     (3<<6)
#define RADIO_AGCCTRL0_WAIT_ITME_8         (0<<4)
#define RADIO_AGCCTRL0_WAIT_ITME_16        (1<<4)
#define RADIO_AGCCTRL0_WAIT_ITME_24        (2<<4)
#define RADIO_AGCCTRL0_WAIT_ITME_32        (3<<4)
#define RADIO_AGCCTRL0_AGC_FREEZE_NORMAL   (0<<2)
#define RADIO_AGCCTRL0_AGC_FREEZE_ONSYNC   (1<<2)
#define RADIO_AGCCTRL0_AGC_FREEZE_AN       (2<<2)
#define RADIO_AGCCTRL0_AGC_FREEZE_ANDIG    (3<<2)
#define RADIO_AGCCTRL0_FILTER_LENGTH_8     (0)
#define RADIO_AGCCTRL0_FILTER_LENGTH_16    (1)
#define RADIO_AGCCTRL0_FILTER_LENGTH_32    (2)
#define RADIO_AGCCTRL0_FILTER_LENGTH_64    (3)

#define RADIO_WOREVT1_EVENT0_HI(VAL)           (VAL)       // WOREVT1 (dynamic)
#define RADIO_WOREVT0_EVENT0_LO(VAL)          (VAL)         // WOREVT2 (dynamic)

#define RADIO_WORCTRL_ALCK_PD              (1<<7)       // Wake On Radio Control
#define RADIO_WORCTRL_EVENT1_TIMEOUT4      (0<<4)
#define RADIO_WORCTRL_EVENT1_TIMEOUT6      (1<<4)
#define RADIO_WORCTRL_EVENT1_TIMEOUT8      (2<<4)
#define RADIO_WORCTRL_EVENT1_TIMEOUT12     (3<<4)
#define RADIO_WORCTRL_EVENT1_TIMEOUT16     (4<<4)
#define RADIO_WORCTRL_EVENT1_TIMEOUT24     (5<<4)
#define RADIO_WORCTRL_EVENT1_TIMEOUT32     (6<<4)
#define RADIO_WORCTRL_EVENT1_TIMEOUT48     (7<<4)
#define RADIO_WORCTRL_WOR_RES_29us         (0)
#define RADIO_WORCTRL_WOR_RES_920us        (1)
#define RADIO_WORCTRL_WOR_RES_32ms         (2)
#define RADIO_WORCTRL_WOR_RES_940ms        (3)

#define RADIO_FREND1_LNA_CURRENT(VAL)            ((VAL&3)<<6)   // Front End RX Configuration
#define RADIO_FREND1_LNA2MIX_CURRENT(VAL)        ((VAL&3)<<4)
#define RADIO_FREND1_LODIV_BUF_CURRENT_RX(VAL)   ((VAL&3)<<2)
#define RADIO_FREND1_MIX_CURRENT(VAL)            (VAL&3)

#define RADIO_FREND0_LODIV_BUF_CURRENT_TX(VAL)   ((VAL&3)<<4)
#define RADIO_FREND0_PA_POWER(VAL)               (VAL&7)

#define RADIO_FSCAL3_HI(VAL)                    ((VAL&3)<<6) // Frequency Synthesizer Calibration
#define RADIO_FSCAL3_CHP_CURR_CAL_EN(VAL)       ((VAL&3)<<4)
#define RADIO_FSCAL3_LO(VAL)                    (VAL&15)

#define RADIO_FSCAL2_VCO_CORE_H_EN       (1<<5)
#define RADIO_FSCAL2_FSCAL2(VAL)         (VAL&15)

#define RADIO_FSCAL1(VAL)         (VAL&31)

#define RADIO_FSCAL0(VAL)         (VAL&127)

#define RADIO_TEST0_HI(VAL)             ((VAL&63)<<2)
#define RADIO_TEST0_VCO_SEL_CAL_EN      (1 << 1)
#define RADIO_TEST0_VCO_SEL_CAL_DIS     (0 << 1)
#define RADIO_TEST0_LO_EN               (1)

#ifdef __cplusplus
}
#endif

#endif /* CC430_REGISTERS_H */
