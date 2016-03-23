#ifndef RADIO_CONFIG_H_
#define RADIO_CONFIG_H_


#include "ezradio_patch_revC2A_general.h"

#include "si4460_registers.h"


#define RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ 26000000L

#define RADIO_CONFIGURATION_DATA_RADIO_CHIP_FAMILY RADIO_CHIP_FAMILY_EZRADIOPRO

/*
// Command:                  RF_POWER_UP
// Description:              Command to power-up the device and select the operational mode and functionality.
*/
//#define RF_POWER_UP 0x02, 0x81, 0x01, 0x01, 0x8C, 0xBA, 0x80 //use patch
//#define RF_POWER_UP 0x02, 0x01, 0x01, 0x01, 0x8C, 0xBA, 0x80  // no patch - TCXO

#define RF_POWER_UP 0x02, 0x01, 0x00, 0x01, 0x8C, 0xBA, 0x80  // no patch - interal xtal

/*
// Command:                  RF_GPIO_PIN_CFG
// Description:              Configures the GPIO pins.
*/
//#define RF_GPIO_PIN_CFG 0x13, 0x00, 0x00, 0x20, 0x21, 0x00, 0x00, 0x00

#define RF_GPIO_PIN_CFG 0x13, 0x1D, 0x1D, 0x20, 0x21, 0x00, 0x00, 0x00

/*
// Set properties:           RF_GLOBAL_XO_TUNE_2
// Number of properties:     2
// Group ID:                 0x00
// Start ID:                 0x00
// Default values:           0x40, 0x00,
// Descriptions:
//   GLOBAL_XO_TUNE - Configure the internal capacitor frequency tuning bank for the crystal oscillator.
//   GLOBAL_CLK_CFG - Clock configuration options.
*/
#define RF_GLOBAL_XO_TUNE_2 0x11, 0x00, 0x02, 0x00, 0x50, 0x00
//#define RF_GLOBAL_XO_TUNE_2 0x11, 0x00, 0x01, 0x00, 0x45

/*
// Set properties:           RF_GLOBAL_CONFIG_1
// Number of properties:     1
// Group ID:                 0x00
// Start ID:                 0x03
// Default values:           0x20,
// Descriptions:
//   GLOBAL_CONFIG - Global configuration settings.
*/
#define RF_GLOBAL_CONFIG_1 0x11, 0x00, 0x01, 0x03, 0x20
//???

/*
// Set properties:           RF_INT_CTL_ENABLE_2
// Number of properties:     2
// Group ID:                 0x01
// Start ID:                 0x00
// Default values:           0x04, 0x00,
// Descriptions:
//   INT_CTL_ENABLE - This property provides for global enabling of the three interrupt groups (Chip, Modem and Packet Handler) in order to generate HW interrupts at the NIRQ pin.
//   INT_CTL_PH_ENABLE - Enable individual interrupt sources within the Packet Handler Interrupt Group to generate a HW interrupt on the NIRQ output pin.
*/
#define RF_INT_CTL_ENABLE_2 0x11, 0x01, 0x02, 0x00, 0x01, 0x3C

/*
// Set properties:           RF_FRR_CTL_A_MODE_4
// Number of properties:     4
// Group ID:                 0x02
// Start ID:                 0x00
// Default values:           0x01, 0x02, 0x09, 0x00,
// Descriptions:
//   FRR_CTL_A_MODE - Fast Response Register A Configuration.
//   FRR_CTL_B_MODE - Fast Response Register B Configuration.
//   FRR_CTL_C_MODE - Fast Response Register C Configuration.
//   FRR_CTL_D_MODE - Fast Response Register D Configuration.
*/
#define RF_FRR_CTL_A_MODE_4 0x11, 0x02, 0x04, 0x00, 0x02, 0x03, 0x09, 0x0A

/*
// Set properties:           RF_PREAMBLE_TX_LENGTH_9
// Number of properties:     9
// Group ID:                 0x10
// Start ID:                 0x00
// Default values:           0x08, 0x14, 0x00, 0x0F, 0x21, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PREAMBLE_TX_LENGTH - Configure length of TX Preamble.
//   PREAMBLE_CONFIG_STD_1 - Configuration of reception of a packet with a Standard Preamble pattern.
//   PREAMBLE_CONFIG_NSTD - Configuration of transmission/reception of a packet with a Non-Standard Preamble pattern.
//   PREAMBLE_CONFIG_STD_2 - Configuration of timeout periods during reception of a packet with Standard Preamble pattern.
//   PREAMBLE_CONFIG - General configuration bits for the Preamble field.
//   PREAMBLE_PATTERN_31_24 - Configuration of the bit values describing a Non-Standard Preamble pattern.
//   PREAMBLE_PATTERN_23_16 - Configuration of the bit values describing a Non-Standard Preamble pattern.
//   PREAMBLE_PATTERN_15_8 - Configuration of the bit values describing a Non-Standard Preamble pattern.
//   PREAMBLE_PATTERN_7_0 - Configuration of the bit values describing a Non-Standard Preamble pattern.
*/
#define RF_PREAMBLE_TX_LENGTH_9 0x11, 0x10, 0x09, 0x00, 0x08, 0x14, 0x00, 0x0F, 0x31, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_SYNC_CONFIG_6
// Number of properties:     6
// Group ID:                 0x11
// Start ID:                 0x00
// Default values:           0x01, 0x2D, 0xD4, 0x2D, 0xD4, 0x00,
// Descriptions:
//   SYNC_CONFIG - Sync Word configuration bits.
//   SYNC_BITS_31_24 - Sync word.
//   SYNC_BITS_23_16 - Sync word.
//   SYNC_BITS_15_8 - Sync word.
//   SYNC_BITS_7_0 - Sync word.
//   SYNC_CONFIG2 - Sync Word configuration bits.
*/
#define RF_SYNC_CONFIG_6 0x11, 0x11, 0x06, 0x00, 0x01, 0xD0, 0xE6, 0x00, 0x00, 0x00

/*
// Set properties:           RF_PKT_CRC_CONFIG_7
// Number of properties:     7
// Group ID:                 0x12
// Start ID:                 0x00
// Default values:           0x00, 0x01, 0x08, 0xFF, 0xFF, 0x00, 0x00,
// Descriptions:
//   PKT_CRC_CONFIG - Select a CRC polynomial and seed.
//   PKT_WHT_POLY_15_8 - 16-bit polynomial value for the PN Generator (e.g., for Data Whitening)
//   PKT_WHT_POLY_7_0 - 16-bit polynomial value for the PN Generator (e.g., for Data Whitening)
//   PKT_WHT_SEED_15_8 - 16-bit seed value for the PN Generator (e.g., for Data Whitening)
//   PKT_WHT_SEED_7_0 - 16-bit seed value for the PN Generator (e.g., for Data Whitening)
//   PKT_WHT_BIT_NUM - Selects which bit of the LFSR (used to generate the PN / data whitening sequence) is used as the output bit for data scrambling.
//   PKT_CONFIG1 - General configuration bits for transmission or reception of a packet.
*/
#define RF_PKT_CRC_CONFIG_7 0x11, 0x12, 0x07, 0x00, 0x05, 0x01, 0x08, 0xFF, 0xFF, 0x00, 0x02

/*
// Set properties:           RF_PKT_LEN_12
// Number of properties:     12
// Group ID:                 0x12
// Start ID:                 0x08
// Default values:           0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PKT_LEN - Configuration bits for reception of a variable length packet.
//   PKT_LEN_FIELD_SOURCE - Field number containing the received packet length byte(s).
//   PKT_LEN_ADJUST - Provides for adjustment/offset of the received packet length value (in order to accommodate a variety of methods of defining total packet length).
//   PKT_TX_THRESHOLD - TX FIFO almost empty threshold.
//   PKT_RX_THRESHOLD - RX FIFO Almost Full threshold.
//   PKT_FIELD_1_LENGTH_12_8 - Unsigned 13-bit Field 1 length value.
//   PKT_FIELD_1_LENGTH_7_0 - Unsigned 13-bit Field 1 length value.
//   PKT_FIELD_1_CONFIG - General data processing and packet configuration bits for Field 1.
//   PKT_FIELD_1_CRC_CONFIG - Configuration of CRC control bits across Field 1.
//   PKT_FIELD_2_LENGTH_12_8 - Unsigned 13-bit Field 2 length value.
//   PKT_FIELD_2_LENGTH_7_0 - Unsigned 13-bit Field 2 length value.
//   PKT_FIELD_2_CONFIG - General data processing and packet configuration bits for Field 2.
*/
#define RF_PKT_LEN_12 0x11, 0x12, 0x0C, 0x08, 0x2A, 0x01, 0xFE, 0x30, 0x30, 0x00, 0x01, 0x06, 0xA2, 0x00, 0x3F, 0x02

/*
// Set properties:           RF_PKT_FIELD_2_CRC_CONFIG_12
// Number of properties:     12
// Group ID:                 0x12
// Start ID:                 0x14
// Default values:           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PKT_FIELD_2_CRC_CONFIG - Configuration of CRC control bits across Field 2.
//   PKT_FIELD_3_LENGTH_12_8 - Unsigned 13-bit Field 3 length value.
//   PKT_FIELD_3_LENGTH_7_0 - Unsigned 13-bit Field 3 length value.
//   PKT_FIELD_3_CONFIG - General data processing and packet configuration bits for Field 3.
//   PKT_FIELD_3_CRC_CONFIG - Configuration of CRC control bits across Field 3.
//   PKT_FIELD_4_LENGTH_12_8 - Unsigned 13-bit Field 4 length value.
//   PKT_FIELD_4_LENGTH_7_0 - Unsigned 13-bit Field 4 length value.
//   PKT_FIELD_4_CONFIG - General data processing and packet configuration bits for Field 4.
//   PKT_FIELD_4_CRC_CONFIG - Configuration of CRC control bits across Field 4.
//   PKT_FIELD_5_LENGTH_12_8 - Unsigned 13-bit Field 5 length value.
//   PKT_FIELD_5_LENGTH_7_0 - Unsigned 13-bit Field 5 length value.
//   PKT_FIELD_5_CONFIG - General data processing and packet configuration bits for Field 5.
*/
#define RF_PKT_FIELD_2_CRC_CONFIG_12 0x11, 0x12, 0x0C, 0x14, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_PKT_FIELD_5_CRC_CONFIG_12
// Number of properties:     12
// Group ID:                 0x12
// Start ID:                 0x20
// Default values:           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PKT_FIELD_5_CRC_CONFIG - Configuration of CRC control bits across Field 5.
//   PKT_RX_FIELD_1_LENGTH_12_8 - Unsigned 13-bit RX Field 1 length value.
//   PKT_RX_FIELD_1_LENGTH_7_0 - Unsigned 13-bit RX Field 1 length value.
//   PKT_RX_FIELD_1_CONFIG - General data processing and packet configuration bits for RX Field 1.
//   PKT_RX_FIELD_1_CRC_CONFIG - Configuration of CRC control bits across RX Field 1.
//   PKT_RX_FIELD_2_LENGTH_12_8 - Unsigned 13-bit RX Field 2 length value.
//   PKT_RX_FIELD_2_LENGTH_7_0 - Unsigned 13-bit RX Field 2 length value.
//   PKT_RX_FIELD_2_CONFIG - General data processing and packet configuration bits for RX Field 2.
//   PKT_RX_FIELD_2_CRC_CONFIG - Configuration of CRC control bits across RX Field 2.
//   PKT_RX_FIELD_3_LENGTH_12_8 - Unsigned 13-bit RX Field 3 length value.
//   PKT_RX_FIELD_3_LENGTH_7_0 - Unsigned 13-bit RX Field 3 length value.
//   PKT_RX_FIELD_3_CONFIG - General data processing and packet configuration bits for RX Field 3.
*/
#define RF_PKT_FIELD_5_CRC_CONFIG_12 0x11, 0x12, 0x0C, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_PKT_RX_FIELD_3_CRC_CONFIG_9
// Number of properties:     9
// Group ID:                 0x12
// Start ID:                 0x2C
// Default values:           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PKT_RX_FIELD_3_CRC_CONFIG - Configuration of CRC control bits across RX Field 3.
//   PKT_RX_FIELD_4_LENGTH_12_8 - Unsigned 13-bit RX Field 4 length value.
//   PKT_RX_FIELD_4_LENGTH_7_0 - Unsigned 13-bit RX Field 4 length value.
//   PKT_RX_FIELD_4_CONFIG - General data processing and packet configuration bits for RX Field 4.
//   PKT_RX_FIELD_4_CRC_CONFIG - Configuration of CRC control bits across RX Field 4.
//   PKT_RX_FIELD_5_LENGTH_12_8 - Unsigned 13-bit RX Field 5 length value.
//   PKT_RX_FIELD_5_LENGTH_7_0 - Unsigned 13-bit RX Field 5 length value.
//   PKT_RX_FIELD_5_CONFIG - General data processing and packet configuration bits for RX Field 5.
//   PKT_RX_FIELD_5_CRC_CONFIG - Configuration of CRC control bits across RX Field 5.
*/
#define RF_PKT_RX_FIELD_3_CRC_CONFIG_9 0x11, 0x12, 0x09, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_PKT_CRC_SEED_31_24_4
// Number of properties:     4
// Group ID:                 0x12
// Start ID:                 0x36
// Default values:           0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PKT_CRC_SEED_31_24 - 32-bit seed value for the 32-bit CRC engine
//   PKT_CRC_SEED_23_16 - 32-bit seed value for the 32-bit CRC engine
//   PKT_CRC_SEED_15_8 - 32-bit seed value for the 32-bit CRC engine
//   PKT_CRC_SEED_7_0 - 32-bit seed value for the 32-bit CRC engine
*/
#define RF_PKT_CRC_SEED_31_24_4 0x11, 0x12, 0x04, 0x36, 0x00, 0x00, 0x00, 0x00


/*
// Set properties:           RF_MODEM_AGC_CONTROL_1
// Number of properties:     1
// Group ID:                 0x20
// Start ID:                 0x35
// Default values:           0xE0,
// Descriptions:
//   MODEM_AGC_CONTROL - Miscellaneous control bits for the Automatic Gain Control (AGC) function in the RX Chain.
*/
#define RF_MODEM_AGC_CONTROL_1 0x11, 0x20, 0x01, 0x35, 0xE0


/*
// Set properties:           RF_MODEM_RSSI_CONTROL_3
// Number of properties:     3
// Group ID:                 0x20
// Start ID:                 0x4C
// Default values:           0x01, 0x00, 0x40,
// Descriptions:
//   MODEM_RSSI_CONTROL - Control of the averaging modes and latching time for reporting RSSI value(s).
//   MODEM_RSSI_CONTROL2 - RSSI Jump Detection control.
//   MODEM_RSSI_COMP - RSSI compensation value.
*/
#define RF_MODEM_RSSI_CONTROL_3 0x11, 0x20, 0x03, 0x4C, 0x09, 0x1C, 0x40

/*
// Set properties:           RF_MODEM_RSSI_MUTE_1
// Number of properties:     1
// Group ID:                 0x20
// Start ID:                 0x57
// Default values:           0x00,
// Descriptions:
//   MODEM_RSSI_MUTE - Configures muting of the RSSI to avoid false RSSI interrupts.
*/
#define RF_MODEM_RSSI_MUTE_1 0x11, 0x20, 0x01, 0x57, 0x00

/*
// Set properties:           RF_MATCH_VALUE_1_12
// Number of properties:     12
// Group ID:                 0x30
// Start ID:                 0x00
// Default values:           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   MATCH_VALUE_1 - Match value to be compared with the result of logically AND-ing (bit-wise) the Mask 1 value with the received Match 1 byte.
//   MATCH_MASK_1 - Mask value to be logically AND-ed (bit-wise) with the Match 1 byte.
//   MATCH_CTRL_1 - Enable for Packet Match functionality, and configuration of Match Byte 1.
//   MATCH_VALUE_2 - Match value to be compared with the result of logically AND-ing (bit-wise) the Mask 2 value with the received Match 2 byte.
//   MATCH_MASK_2 - Mask value to be logically AND-ed (bit-wise) with the Match 2 byte.
//   MATCH_CTRL_2 - Configuration of Match Byte 2.
//   MATCH_VALUE_3 - Match value to be compared with the result of logically AND-ing (bit-wise) the Mask 3 value with the received Match 3 byte.
//   MATCH_MASK_3 - Mask value to be logically AND-ed (bit-wise) with the Match 3 byte.
//   MATCH_CTRL_3 - Configuration of Match Byte 3.
//   MATCH_VALUE_4 - Match value to be compared with the result of logically AND-ing (bit-wise) the Mask 4 value with the received Match 4 byte.
//   MATCH_MASK_4 - Mask value to be logically AND-ed (bit-wise) with the Match 4 byte.
//   MATCH_CTRL_4 - Configuration of Match Byte 4.
*/
#define RF_MATCH_VALUE_1_12 0x11, 0x30, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

// AUTOMATICALLY GENERATED CODE!
// DO NOT EDIT/MODIFY BELOW THIS LINE!
// --------------------------------------------

#ifndef FIRMWARE_LOAD_COMPILE
#define RADIO_CONFIGURATION_DATA_ARRAY { \
        SI446X_PATCH_CMDS, \
        0x07, RF_POWER_UP, \
        0x08, RF_GPIO_PIN_CFG, \
        0x06, RF_GLOBAL_XO_TUNE_2, \
        0x05, RF_GLOBAL_CONFIG_1, \
        0x06, RF_INT_CTL_ENABLE_2, \
        0x08, RF_FRR_CTL_A_MODE_4, \
        0x0D, RF_PREAMBLE_TX_LENGTH_9, \
        0x0A, RF_SYNC_CONFIG_6, \
        0x0B, RF_PKT_CRC_CONFIG_7, \
        0x10, RF_PKT_LEN_12, \
        0x10, RF_PKT_FIELD_2_CRC_CONFIG_12, \
        0x10, RF_PKT_FIELD_5_CRC_CONFIG_12, \
        0x0D, RF_PKT_RX_FIELD_3_CRC_CONFIG_9, \
        0x08, RF_PKT_CRC_SEED_31_24_4, \
        0x10, 0x11, RF_MODEM_MOD_TYPE_12_433_NR, \
        0x05, 0x11, RF_MODEM_FREQ_DEV_0_1_433_NR, \
        0x10, 0x11, RF_MODEM_TX_RAMP_DELAY_12_433_NR, \
        0x10, 0x11, RF_MODEM_BCR_NCO_OFFSET_2_12_433_NR, \
        0x07, 0x11, RF_MODEM_AFC_LIMITER_1_3_433_NR, \
        0x05, RF_MODEM_AGC_CONTROL_1, \
        0x10, 0x11, RF_MODEM_AGC_WINDOW_SIZE_12_433_NR, \
        0x0C, 0x11, RF_MODEM_RAW_CONTROL_8_433_NR, \
        0x07, RF_MODEM_RSSI_CONTROL_3, \
        0x06, 0x11, RF_MODEM_RAW_SEARCH2_2_433, \
        0x06, 0x11, RF_MODEM_SPIKE_DET_2_433_NR, \
        0x05, RF_MODEM_RSSI_MUTE_1, \
        0x09, 0x11, RF_MODEM_DSA_CTRL1_5_433_NR, \
        0x10, 0x11, RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_433_NR, \
        0x10, 0x11, RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_433_NR, \
        0x10, 0x11, RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_433_NR, \
        0x08, 0x11, RF_PA_MODE_4_433_NR, \
        0x0B, 0x11, RF_SYNTH_PFDCP_CPFF_7_433_NR, \
        0x10, RF_MATCH_VALUE_1_12, \
        0x0C, 0x11, RF_FREQ_CONTROL_INTE_8_433_NR, \
        0x00 \
 }
#else
#define RADIO_CONFIGURATION_DATA_ARRAY { 0 }
#endif

#endif

