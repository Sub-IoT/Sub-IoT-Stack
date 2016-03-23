/*
 * si4460_registers.h
 *
 *  Created on: Dec 11, 2015
 *      Author: MaartenWeyn
 */

#ifndef FRAMEWORK_HAL_CHIPS_SI4460_EZRADIODRV_SI4460_REGISTERS_H_
#define FRAMEWORK_HAL_CHIPS_SI4460_EZRADIODRV_SI4460_REGISTERS_H_

/*
// Set properties:           RF_MODEM_01
// Number of properties:     12
// Group ID:                 0x20
// Start ID:                 0x00
// Descriptions:
//   MODEM_MOD_TYPE - Selects the type of modulation. In TX mode, additionally selects the source of the modulation.
//   MODEM_MAP_CONTROL - Controls polarity and mapping of transmit and receive bits.
//   MODEM_DSM_CTRL - Miscellaneous control bits for the Delta-Sigma Modulator (DSM) in the PLL Synthesizer.
//   MODEM_DATA_RATE_2 - Unsigned 24-bit value used to determine the TX data rate
//   MODEM_DATA_RATE_1 - Unsigned 24-bit value used to determine the TX data rate
//   MODEM_DATA_RATE_0 - Unsigned 24-bit value used to determine the TX data rate
//   MODEM_TX_NCO_MODE_3 - TX Gaussian filter oversampling ratio and Byte 3 of unsigned 26-bit TX Numerically Controlled Oscillator (NCO) modulus.
//   MODEM_TX_NCO_MODE_2 - TX Gaussian filter oversampling ratio and Byte 3 of unsigned 26-bit TX Numerically Controlled Oscillator (NCO) modulus.
//   MODEM_TX_NCO_MODE_1 - TX Gaussian filter oversampling ratio and Byte 3 of unsigned 26-bit TX Numerically Controlled Oscillator (NCO) modulus.
//   MODEM_TX_NCO_MODE_0 - TX Gaussian filter oversampling ratio and Byte 3 of unsigned 26-bit TX Numerically Controlled Oscillator (NCO) modulus.
//   MODEM_FREQ_DEV_2 - 17-bit unsigned TX frequency deviation word.
//   MODEM_FREQ_DEV_1 - 17-bit unsigned TX frequency deviation word.
*/

#define RF_MODEM_MOD_TYPE_12_433_LR 0x20, 0x0C, 0x00, 0x03, 0x00, 0x07, 0x05, 0xDC, 0x00, 0x05, 0x8C, 0xBA, 0x80, 0x00, 0x01
#define RF_MODEM_MOD_TYPE_12_433_NR 0x20, 0x0C, 0x00, 0x03, 0x00, 0x07, 0x10, 0xF4, 0x3C, 0x09, 0x8C, 0xBA, 0x80, 0x00, 0x0F
#define RF_MODEM_MOD_TYPE_12_433_HR 0x20, 0x0C, 0x00, 0x03, 0x00, 0x07, 0x32, 0xDC, 0xDC, 0x09, 0x8C, 0xBA, 0x80, 0x00, 0x0D
#define RF_MODEM_MOD_TYPE_12_868_LR 0x20, 0x0C, 0x00, 0x03, 0x00, 0x07, 0x05, 0xDC, 0x00, 0x05, 0x8C, 0xBA, 0x80, 0x00, 0x00
#define RF_MODEM_MOD_TYPE_12_868_NR 0x20, 0x0C, 0x00, 0x03, 0x00, 0x07, 0x10, 0xF4, 0x3C, 0x09, 0x8C, 0xBA, 0x80, 0x00, 0x07
#define RF_MODEM_MOD_TYPE_12_868_HR 0x20, 0x0C, 0x00, 0x03, 0x00, 0x07, 0x32, 0xDC, 0xDC, 0x09, 0x8C, 0xBA, 0x80, 0x00, 0x06

/*
// Set properties:           RF_MODEM_FREQ_DEV_0_1
// Number of properties:     1
// Group ID:                 0x20
// Start ID:                 0x0C
// Descriptions:
//   MODEM_FREQ_DEV_0 - 17-bit unsigned TX frequency deviation word.
*/
#define RF_MODEM_FREQ_DEV_0_1_433_LR 0x20, 0x01, 0x0C, 0x83
#define RF_MODEM_FREQ_DEV_0_1_433_NR 0x20, 0x01, 0x0C, 0xC1
#define RF_MODEM_FREQ_DEV_0_1_433_HR 0x20, 0x01, 0x0C, 0x21
#define RF_MODEM_FREQ_DEV_0_1_868_LR 0x20, 0x01, 0x0C, 0xC2
#define RF_MODEM_FREQ_DEV_0_1_868_NR 0x20, 0x01, 0x0C, 0xE0
#define RF_MODEM_FREQ_DEV_0_1_868_HR 0x20, 0x01, 0x0C, 0x90

/*
// Set properties:           RF_MODEM_TX_RAMP_DELAY
// Number of properties:     12
// Group ID:                 0x20
// Start ID:                 0x18
// Descriptions:
//   MODEM_TX_RAMP_DELAY - TX ramp-down delay setting.
//   MODEM_MDM_CTRL - MDM control.
//   MODEM_IF_CONTROL - Selects Fixed-IF, Scaled-IF, or Zero-IF mode of RX Modem operation.
//   MODEM_IF_FREQ_2 - the IF frequency setting (an 18-bit signed number).
//   MODEM_IF_FREQ_1 - the IF frequency setting (an 18-bit signed number).
//   MODEM_IF_FREQ_0 - the IF frequency setting (an 18-bit signed number).
//   MODEM_DECIMATION_CFG1 - Specifies three decimator ratios for the Cascaded Integrator Comb (CIC) filter.
//   MODEM_DECIMATION_CFG0 - Specifies miscellaneous parameters and decimator ratios for the Cascaded Integrator Comb (CIC) filter.
//   MODEM_DECIMATION_CFG2 - Specifies miscellaneous decimator filter selections.
//   MODEM_IFPKD_THRESHOLDS -
//   MODEM_BCR_OSR_1 - RX BCR/Slicer oversampling rate (12-bit unsigned number).
//   MODEM_BCR_OSR_0 - RX BCR/Slicer oversampling rate (12-bit unsigned number).
*/
#define RF_MODEM_TX_RAMP_DELAY_12_433_LR 0x20, 0x0C, 0x18, 0x01, 0x00, 0x08, 0x03, 0x80, 0x00, 0x30, 0x20, 0x00, 0xE8, 0x00, 0xA9
#define RF_MODEM_TX_RAMP_DELAY_12_433_NR 0x20, 0x0C, 0x18, 0x01, 0x00, 0x08, 0x03, 0x80, 0x00, 0x10, 0x20, 0x00, 0xE8, 0x00, 0x75
#define RF_MODEM_TX_RAMP_DELAY_12_433_HR 0x20, 0x0C, 0x18, 0x01, 0x00, 0x08, 0x03, 0x80, 0x00, 0x00, 0x20, 0x00, 0xE8, 0x00, 0x4E
#define RF_MODEM_TX_RAMP_DELAY_12_868_LR 0x20, 0x0C, 0x18, 0x01, 0x00, 0x08, 0x03, 0xC0, 0x00, 0x20, 0x20, 0x00, 0xE8, 0x01, 0x53
#define RF_MODEM_TX_RAMP_DELAY_12_868_NR 0x20, 0x0C, 0x18, 0x01, 0x00, 0x08, 0x03, 0xC0, 0x00, 0x10, 0x20, 0x00, 0xE8, 0x00, 0x75
#define RF_MODEM_TX_RAMP_DELAY_12_868_HR 0x20, 0x0C, 0x18, 0x01, 0x00, 0x08, 0x03, 0xC0, 0x00, 0x00, 0x20, 0x00, 0xE8, 0x00, 0x4E


/*
// Set properties:           RF_MODEM_BCR_NCO_OFFSET_2
// Number of properties:     12
// Group ID:                 0x20
// Start ID:                 0x24
// Descriptions:
//   MODEM_BCR_NCO_OFFSET_2 - RX BCR NCO offset value (an unsigned 22-bit number).
//   MODEM_BCR_NCO_OFFSET_1 - RX BCR NCO offset value (an unsigned 22-bit number).
//   MODEM_BCR_NCO_OFFSET_0 - RX BCR NCO offset value (an unsigned 22-bit number).
//   MODEM_BCR_GAIN_1 - The unsigned 11-bit RX BCR loop gain value.
//   MODEM_BCR_GAIN_0 - The unsigned 11-bit RX BCR loop gain value.
//   MODEM_BCR_GEAR - RX BCR loop gear control.
//   MODEM_BCR_MISC1 - Miscellaneous control bits for the RX BCR loop.
//   MODEM_BCR_MISC0 - Miscellaneous RX BCR loop controls.
//   MODEM_AFC_GEAR - RX AFC loop gear control.
//   MODEM_AFC_WAIT - RX AFC loop wait time control.
//   MODEM_AFC_GAIN_1 - Sets the gain of the PLL-based AFC acquisition loop, and provides miscellaneous control bits for AFC functionality.
//   MODEM_AFC_GAIN_0 - Sets the gain of the PLL-based AFC acquisition loop, and provides miscellaneous control bits for AFC functionality.
*/
#define RF_MODEM_BCR_NCO_OFFSET_2_12_433_LR 0x20, 0x0C, 0x24, 0x03, 0x06, 0x55, 0x03, 0x08, 0x02, 0x00, 0x00, 0x00, 0x12, 0x80, 0x61
#define RF_MODEM_BCR_NCO_OFFSET_2_12_433_NR 0x20, 0x0C, 0x24, 0x04, 0x60, 0x43, 0x02, 0x6E, 0x02, 0x00, 0x00, 0x00, 0x12, 0x82, 0x30
#define RF_MODEM_BCR_NCO_OFFSET_2_12_433_HR 0x20, 0x0C, 0x24, 0x06, 0x90, 0x6A, 0x07, 0xFF, 0x02, 0x00, 0x00, 0x00, 0x23, 0x8D, 0x21
#define RF_MODEM_BCR_NCO_OFFSET_2_12_868_LR 0x20, 0x0C, 0x24, 0x01, 0x83, 0x2B, 0x01, 0x83, 0x02, 0x00, 0x00, 0x00, 0x12, 0x80, 0x30
#define RF_MODEM_BCR_NCO_OFFSET_2_12_868_NR 0x20, 0x0C, 0x24, 0x04, 0x60, 0x43, 0x02, 0x6E, 0x02, 0x00, 0x00, 0x00, 0x12, 0x81, 0x18
#define RF_MODEM_BCR_NCO_OFFSET_2_12_868_HR 0x20, 0x0C, 0x24, 0x06, 0x90, 0x6A, 0x07, 0xFF, 0x02, 0x00, 0x00, 0x00, 0x23, 0x86, 0x90


/*
// Set properties:           RF_MODEM_AFC_LIMITER_1_3
// Number of properties:     3
// Group ID:                 0x20
// Start ID:                 0x30
// Default values:           0x00, 0x40, 0xA0,
// Descriptions:
//   MODEM_AFC_LIMITER_1 - Set the AFC limiter value.
//   MODEM_AFC_LIMITER_0 - Set the AFC limiter value.
//   MODEM_AFC_MISC - Specifies miscellaneous AFC control bits.
*/
#define RF_MODEM_AFC_LIMITER_1_3_433_LR 0x20, 0x03, 0x30, 0x04, 0x11, 0xA0
#define RF_MODEM_AFC_LIMITER_1_3_433_NR 0x20, 0x03, 0x30, 0x02, 0x80, 0xA0
#define RF_MODEM_AFC_LIMITER_1_3_433_HR 0x20, 0x03, 0x30, 0x00, 0x9E, 0xA0
#define RF_MODEM_AFC_LIMITER_1_3_868_LR 0x20, 0x03, 0x30, 0x07, 0x5A, 0xA0
#define RF_MODEM_AFC_LIMITER_1_3_868_NR 0x20, 0x03, 0x30, 0x02, 0x80, 0xA0
#define RF_MODEM_AFC_LIMITER_1_3_868_HR 0x20, 0x03, 0x30, 0x00, 0x9E, 0xA0


/*
// Set properties:           RF_MODEM_AGC_WINDOW_SIZE_12
// Number of properties:     12
// Group ID:                 0x20
// Start ID:                 0x38
// Default values:           0x11, 0x10, 0x10, 0x0B, 0x1C, 0x40, 0x00, 0x00, 0x2B, 0x0C, 0xA4, 0x03,
// Descriptions:
//   MODEM_AGC_WINDOW_SIZE - Specifies the size of the measurement and settling windows for the AGC algorithm.
//   MODEM_AGC_RFPD_DECAY - Sets the decay time of the RF peak detectors.
//   MODEM_AGC_IFPD_DECAY - Sets the decay time of the IF peak detectors.
//   MODEM_FSK4_GAIN1 - Specifies the gain factor of the secondary branch in 4(G)FSK ISI-suppression.
//   MODEM_FSK4_GAIN0 - Specifies the gain factor of the primary branch in 4(G)FSK ISI-suppression.
//   MODEM_FSK4_TH1 - 16 bit 4(G)FSK slicer threshold.
//   MODEM_FSK4_TH0 - 16 bit 4(G)FSK slicer threshold.
//   MODEM_FSK4_MAP - 4(G)FSK symbol mapping code.
//   MODEM_OOK_PDTC - Configures the attack and decay times of the OOK Peak Detector.
//   MODEM_OOK_BLOPK - Configures the slicing reference level of the OOK Peak Detector.
//   MODEM_OOK_CNT1 - OOK control.
//   MODEM_OOK_MISC - Selects the detector(s) used for demodulation of an OOK signal, or for demodulation of a (G)FSK signal when using the asynchronous demodulator.
*/
#define RF_MODEM_AGC_WINDOW_SIZE_12_433_LR 0x20, 0x0C, 0x38, 0x11, 0x25, 0x25, 0x80, 0x1A, 0x40, 0x00, 0x00, 0x29, 0x0C, 0xA4, 0x23
#define RF_MODEM_AGC_WINDOW_SIZE_12_433_NR 0x20, 0x0C, 0x38, 0x11, 0x1A, 0x1A, 0x80, 0x1A, 0x73, 0x33, 0x00, 0x28, 0x0C, 0xA4, 0x23
#define RF_MODEM_AGC_WINDOW_SIZE_12_433_HR 0x20, 0x0C, 0x38, 0x11, 0x11, 0x11, 0x80, 0x1A, 0x20, 0x00, 0x00, 0x28, 0x0C, 0xA4, 0x23
#define RF_MODEM_AGC_WINDOW_SIZE_12_868_LR 0x20, 0x0C, 0x38, 0x11, 0x4A, 0x4A, 0x80, 0x1A, 0x40, 0x00, 0x00, 0x2A, 0x0C, 0xA4, 0x23
#define RF_MODEM_AGC_WINDOW_SIZE_12_868_NR 0x20, 0x0C, 0x38, 0x11, 0x1A, 0x1A, 0x80, 0x1A, 0x73, 0x33, 0x00, 0x28, 0x0C, 0xA4, 0x23
#define RF_MODEM_AGC_WINDOW_SIZE_12_868_HR 0x20, 0x0C, 0x38, 0x11, 0x11, 0x11, 0x80, 0x1A, 0x20, 0x00, 0x00, 0x28, 0x0C, 0xA4, 0x23


/*
// Set properties:           RF_MODEM_RAW_CONTROL_8
// Number of properties:     8
// Group ID:                 0x20
// Start ID:                 0x45
// Default values:           0x02, 0x00, 0xA3, 0x02, 0x80, 0xFF, 0x0C, 0x01,
// Descriptions:
//   MODEM_RAW_CONTROL - Defines gain and enable controls for raw / nonstandard mode.
//   MODEM_RAW_EYE_1 - 11 bit eye-open detector threshold.
//   MODEM_RAW_EYE_0 - 11 bit eye-open detector threshold.
//   MODEM_ANT_DIV_MODE - Antenna diversity mode settings.
//   MODEM_ANT_DIV_CONTROL - Specifies controls for the Antenna Diversity algorithm.
//   MODEM_RSSI_THRESH - Configures the RSSI threshold.
//   MODEM_RSSI_JUMP_THRESH - Configures the RSSI Jump Detection threshold.
//   MODEM_RSSI_CONTROL - Control of the averaging modes and latching time for reporting RSSI value(s).
*/
#define RF_MODEM_RAW_CONTROL_8_433_LR 0x20, 0x08, 0x45, 0x03, 0x00, 0x7B, 0x02, 0x00, 0xFF, 0x06, 0x02
#define RF_MODEM_RAW_CONTROL_8_433_NR 0x20, 0x08, 0x45, 0x03, 0x01, 0x3F, 0x02, 0x00, 0xFF, 0x06, 0x02
#define RF_MODEM_RAW_CONTROL_8_433_HR 0x20, 0x08, 0x45, 0x03, 0x00, 0x85, 0x02, 0x00, 0xFF, 0x06, 0x02
#define RF_MODEM_RAW_CONTROL_8_868_LR 0x20, 0x08, 0x45, 0x03, 0x00, 0x3D, 0x02, 0x00, 0xFF, 0x06, 0x02
#define RF_MODEM_RAW_CONTROL_8_868_NR 0x20, 0x08, 0x45, 0x03, 0x01, 0x3F, 0x02, 0x00, 0xFF, 0x06, 0x02
#define RF_MODEM_RAW_CONTROL_8_868_HR 0x20, 0x08, 0x45, 0x03, 0x00, 0x85, 0x02, 0x00, 0xFF, 0x06, 0x02


/*
// Set properties:           RF_MODEM_RAW_SEARCH2_2
// Number of properties:     2
// Group ID:                 0x20
// Start ID:                 0x50
// Default values:           0x00, 0x08,
// Descriptions:
//   MODEM_RAW_SEARCH2 - Defines and controls the search period length for the Moving Average and Min-Max detectors.
//   MODEM_CLKGEN_BAND - Select PLL Synthesizer output divider ratio as a function of frequency band.
*/
#define RF_MODEM_RAW_SEARCH2_2_433 0x20, 0x02, 0x50, 0x84, 0x0A
#define RF_MODEM_RAW_SEARCH2_2_868 0x20, 0x02, 0x50, 0x84, 0x08


/*
// Set properties:           RF_MODEM_SPIKE_DET_2
// Number of properties:     2
// Group ID:                 0x20
// Start ID:                 0x54
// Default values:           0x00, 0x00,
// Descriptions:
//   MODEM_SPIKE_DET - Configures the threshold for (G)FSK Spike Detection.
//   MODEM_ONE_SHOT_AFC - Configures parameters for th e One Shot AFC function and for BCR timing/acquisition.
*/
#define RF_MODEM_SPIKE_DET_2_433_LR 0x20, 0x02, 0x54, 0x03, 0x07
#define RF_MODEM_SPIKE_DET_2_433_NR 0x20, 0x02, 0x54, 0x04, 0x07
#define RF_MODEM_SPIKE_DET_2_433_HR RF_MODEM_SPIKE_DET_2_433_LR
#define RF_MODEM_SPIKE_DET_2_868_LR RF_MODEM_SPIKE_DET_2_433_LR
#define RF_MODEM_SPIKE_DET_2_868_NR RF_MODEM_SPIKE_DET_2_433_NR
#define RF_MODEM_SPIKE_DET_2_868_HR RF_MODEM_SPIKE_DET_2_433_NR


/*
// Set properties:           RF_MODEM_DSA_CTRL1_5
// Number of properties:     5
// Group ID:                 0x20
// Start ID:                 0x5B
// Default values:           0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   MODEM_DSA_CTRL1 - Configures parameters for the Signal Arrival Detection circuit block and algorithm.
//   MODEM_DSA_CTRL2 - Configures parameters for the Signal Arrival Detection circuit block and algorithm.
//   MODEM_DSA_QUAL - Configures parameters for the Eye Opening qualification m ethod of the Signal Arrival Detection algorithm.
//   MODEM_DSA_RSSI - Signal Arrival Detect RSSI Qualifier Config
//   MODEM_DSA_MISC - Miscellaneous detection of signal arrival bits.
*/
#define RF_MODEM_DSA_CTRL1_5_433_LR 0x20, 0x05, 0x5B, 0x42, 0x04, 0x06, 0x78, 0x20
#define RF_MODEM_DSA_CTRL1_5_433_NR 0x20, 0x05, 0x5B, 0x40, 0x04, 0x09, 0x78, 0x20
#define RF_MODEM_DSA_CTRL1_5_433_HR 0x20, 0x05, 0x5B, 0x40, 0x04, 0x04, 0x78, 0x20
#define RF_MODEM_DSA_CTRL1_5_868_LR 0x20, 0x05, 0x5B, 0x42, 0x04, 0x05, 0x78, 0x20
#define RF_MODEM_DSA_CTRL1_5_868_NR 0x20, 0x05, 0x5B, 0x40, 0x04, 0x09, 0x78, 0x20
#define RF_MODEM_DSA_CTRL1_5_868_HR 0x20, 0x05, 0x5B, 0x40, 0x04, 0x04, 0x78, 0x20


/*
// Set properties:           RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12
// Number of properties:     12
// Group ID:                 0x21
// Start ID:                 0x00
// Default values:           0xFF, 0xBA, 0x0F, 0x51, 0xCF, 0xA9, 0xC9, 0xFC, 0x1B, 0x1E, 0x0F, 0x01,
// Descriptions:
//   MODEM_CHFLT_RX1_CHFLT_COE13_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE12_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE11_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE10_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE9_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE8_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE7_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE6_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE5_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE4_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE3_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE2_7_0 - Filter coefficients for the first set of RX filter coefficients.
*/
#define RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_433_LR 0x21, 0x0C, 0x00, 0xFF, 0xC4, 0x30, 0x7F, 0xF5, 0xB5, 0xB8, 0xDE, 0x05, 0x17, 0x16, 0x0C
#define RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_433_NR 0x21, 0x0C, 0x00, 0xCC, 0xA1, 0x30, 0xA0, 0x21, 0xD1, 0xB9, 0xC9, 0xEA, 0x05, 0x12, 0x11
#define RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_433_HR 0x21, 0x0C, 0x00, 0x7E, 0x64, 0x1B, 0xBA, 0x58, 0x0B, 0xDD, 0xCE, 0xD6, 0xE6, 0xF6, 0x00
#define RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_868_LR 0x21, 0x0C, 0x00, 0xCC, 0xA1, 0x30, 0xA0, 0x21, 0xD1, 0xB9, 0xC9, 0xEA, 0x05, 0x12, 0x11
#define RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_868_NR RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_868_LR
#define RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12_868_HR 0x21, 0x0C, 0x00, 0x7E, 0x64, 0x1B, 0xBA, 0x58, 0x0B, 0xDD, 0xCE, 0xD6, 0xE6, 0xF6, 0x00

/*
// Set properties:           RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12
// Number of properties:     12
// Group ID:                 0x21
// Start ID:                 0x0C
// Default values:           0xFC, 0xFD, 0x15, 0xFF, 0x00, 0x0F, 0xFF, 0xC4, 0x30, 0x7F, 0xF5, 0xB5,
// Descriptions:
//   MODEM_CHFLT_RX1_CHFLT_COE1_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE0_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COEM0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COEM1 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COEM2 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COEM3 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE13_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE12_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE11_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE10_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE9_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE8_7_0 - Filter coefficients for the second set of RX filter coefficients.
*/
#define RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_433_LR 0x21, 0x0C, 0x0C, 0x03, 0x00, 0x15, 0xFF, 0x00, 0x00, 0xFF, 0xC4, 0x30, 0x7F, 0xF5, 0xB5
#define RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_433_NR 0x21, 0x0C, 0x0C, 0x0A, 0x04, 0x15, 0xFC, 0x03, 0x00, 0xCC, 0xA1, 0x30, 0xA0, 0x21, 0xD1
#define RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_433_HR 0x21, 0x0C, 0x0C, 0x03, 0x03, 0x15, 0xF0, 0x3F, 0x00, 0x7E, 0x64, 0x1B, 0xBA, 0x58, 0x0B
#define RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_868_LR 0x21, 0x0C, 0x0C, 0x0A, 0x04, 0x15, 0xFC, 0x03, 0x00, 0xCC, 0xA1, 0x30, 0xA0, 0x21, 0xD1
#define RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_868_NR RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_868_LR
#define RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12_868_HR 0x21, 0x0C, 0x0C, 0x03, 0x03, 0x15, 0xF0, 0x3F, 0x00, 0x7E, 0x64, 0x1B, 0xBA, 0x58, 0x0B

/*
// Set properties:           RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12
// Number of properties:     12
// Group ID:                 0x21
// Start ID:                 0x18
// Default values:           0xB8, 0xDE, 0x05, 0x17, 0x16, 0x0C, 0x03, 0x00, 0x15, 0xFF, 0x00, 0x00,
// Descriptions:
//   MODEM_CHFLT_RX2_CHFLT_COE7_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE6_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE5_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE4_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE3_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE2_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE1_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE0_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COEM0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COEM1 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COEM2 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COEM3 - Filter coefficients for the second set of RX filter coefficients.
*/
#define RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_433_LR 0x21, 0x0C, 0x18, 0xB8, 0xDE, 0x05, 0x17, 0x16, 0x0C, 0x03, 0x00, 0x15, 0xFF, 0x00, 0x00
#define RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_433_NR 0x21, 0x0C, 0x18, 0xB9, 0xC9, 0xEA, 0x05, 0x12, 0x11, 0x0A, 0x04, 0x15, 0xFC, 0x03, 0x00
#define RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_433_HR 0x21, 0x0C, 0x18, 0xDD, 0xCE, 0xD6, 0xE6, 0xF6, 0x00, 0x03, 0x03, 0x15, 0xF0, 0x3F, 0x00
#define RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_868_LR 0x21, 0x0C, 0x18, 0xB9, 0xC9, 0xEA, 0x05, 0x12, 0x11, 0x0A, 0x04, 0x15, 0xFC, 0x03, 0x00
#define RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_868_NR RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_868_LR
#define RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12_868_HR 0x21, 0x0C, 0x18, 0xDD, 0xCE, 0xD6, 0xE6, 0xF6, 0x00, 0x03, 0x03, 0x15, 0xF0, 0x3F, 0x00

/*
// Set properties:           RF_PA_MODE_4
// Number of properties:     4
// Group ID:                 0x22
// Start ID:                 0x00
// Default values:           0x08, 0x7F, 0x00, 0x5D,
// Descriptions:
//   PA_MODE - Selects the PA operating mode, and selects resolution of PA power adjustment (i.e., step size).
//   PA_PWR_LVL - Configuration of PA output power level.
//   PA_BIAS_CLKDUTY - Configuration of the PA Bias and duty cycle of the TX clock source.
//   PA_TC - Configuration of PA ramping parameters.
*/
#define RF_PA_MODE_4_433_LR 0x22, 0x04, 0x00, 0x18, 0x4F, 0xC0, 0x1D
#define RF_PA_MODE_4_433_NR RF_PA_MODE_4_433_LR
#define RF_PA_MODE_4_433_HR 0x22, 0x04, 0x00, 0x18, 0x4F, 0xC0, 0x1D
#define RF_PA_MODE_4_868_LR RF_PA_MODE_4_433_LR
#define RF_PA_MODE_4_868_NR RF_PA_MODE_4_433_LR
#define RF_PA_MODE_4_868_HR 0x22, 0x04, 0x00, 0x18, 0x4F, 0xC0, 0x3D

/*
// Set properties:           RF_SYNTH_PFDCP_CPFF_7
// Number of properties:     7
// Group ID:                 0x23
// Start ID:                 0x00
// Default values:           0x2C, 0x0E, 0x0B, 0x04, 0x0C, 0x73, 0x03,
// Descriptions:
//   SYNTH_PFDCP_CPFF - Feed forward charge pump current selection.
//   SYNTH_PFDCP_CPINT - Integration charge pump current selection.
//   SYNTH_VCO_KV - Gain scaling factors (Kv) for the VCO tuning varactors on both the integrated-path and feed forward path.
//   SYNTH_LPFILT3 - Value of resistor R2 in feed-forward path of loop filter.
//   SYNTH_LPFILT2 - Value of capacitor C2 in feed-forward path of loop filter.
//   SYNTH_LPFILT1 - Value of capacitors C1 and C3 in feed-forward path of loop filter.
//   SYNTH_LPFILT0 - Bias current of the active amplifier in the feed-forward loop filter.
*/
#define RF_SYNTH_PFDCP_CPFF_7_433_LR 0x23, 0x07, 0x00, 0x2C, 0x0E, 0x0B, 0x04, 0x0C, 0x73, 0x03
#define RF_SYNTH_PFDCP_CPFF_7_433_NR RF_SYNTH_PFDCP_CPFF_7_433_LR
#define RF_SYNTH_PFDCP_CPFF_7_433_HR 0x23, 0x07, 0x00, 0x39, 0x04, 0x0B, 0x05, 0x04, 0x01, 0x03
#define RF_SYNTH_PFDCP_CPFF_7_868_LR RF_SYNTH_PFDCP_CPFF_7_433_LR
#define RF_SYNTH_PFDCP_CPFF_7_868_NR RF_SYNTH_PFDCP_CPFF_7_433_LR
#define RF_SYNTH_PFDCP_CPFF_7_868_HR 0x23, 0x07, 0x00, 0x39, 0x04, 0x0B, 0x05, 0x04, 0x01, 0x03

/*
// Set properties:           RF_FREQ_CONTROL_INTE_8
// Number of properties:     8
// Group ID:                 0x40
// Start ID:                 0x00
// Default values:           0x3C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x20, 0xFF,
// Descriptions:
//   FREQ_CONTROL_INTE - Frac-N PLL Synthesizer integer divide number.
//   FREQ_CONTROL_FRAC_2 - Frac-N PLL fraction number.
//   FREQ_CONTROL_FRAC_1 - Frac-N PLL fraction number.
//   FREQ_CONTROL_FRAC_0 - Frac-N PLL fraction number.
//   FREQ_CONTROL_CHANNEL_STEP_SIZE_1 - EZ Frequency Programming channel step size.
//   FREQ_CONTROL_CHANNEL_STEP_SIZE_0 - EZ Frequency Programming channel step size.
//   FREQ_CONTROL_W_SIZE - Set window gating period (in number of crystal reference clock cycles) for counting VCO frequency during calibration.
//   FREQ_CONTROL_VCOCNT_RX_ADJ - Adjust target count for VCO calibration in RX mode.
*/
#define RF_FREQ_CONTROL_INTE_8_433_LR 0x40, 0x08, 0x00, 0x41, 0x0D, 0x03, 0x26, 0x07, 0xE0, 0x20, 0xFE
#define RF_FREQ_CONTROL_INTE_8_433_NR 0x40, 0x08, 0x00, 0x43, 0x09, 0x6D, 0x7D, 0x3F, 0x04, 0x20, 0xFE
#define RF_FREQ_CONTROL_INTE_8_433_HR 0x40, 0x08, 0x00, 0x43, 0x09, 0x6D, 0x7D, 0x3F, 0x04, 0x20, 0xFE
#define RF_FREQ_CONTROL_INTE_8_868_LR1 0x40, 0x08, 0x00, 0x41, 0x0B, 0x15, 0xA9, 0x03, 0xF0, 0x20, 0xFF
#define RF_FREQ_CONTROL_INTE_8_868_LR2 0x40, 0x08, 0x00, 0x41, 0x0F, 0x01, 0xF8, 0x03, 0xF0, 0x20, 0xFF
#define RF_FREQ_CONTROL_INTE_8_868_NR 0x40, 0x08, 0x00, 0x41, 0x0B, 0x23, 0x72, 0x1F, 0x82, 0x20, 0xFF
#define RF_FREQ_CONTROL_INTE_8_868_HR 0x40, 0x08, 0x00, 0x41, 0x0B, 0x23, 0x72, 0x1F, 0x82, 0x20, 0xFF

/*******************************************************************
							   SYNC WORD
********************************************************************/

//Although the Sync Word byte(s) are transmitted/received in descending order (i.e., Byte 3 first, followed by Byte 2, etc.), each byte is transmitted/received in littleendian fashion (i.e., least significant bit first).
#define RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS0_0  \
  0x11 /* GROUP: Sync                                       */,\
  0x04 /* NUM_PROPS                                         */,\
  0x01 /* START_PROP                                        */,\
  0x67 /* SYNC_BITS,BITS[7:0],BITS[7:0],BITS[7:0],BITS[7:0] */,\
  0x0B /* DATA1                                             */,\
  0x00 /* DATA2                                             */,\
  0x00 /* DATA3                                             */\

#define RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS0_1  \
  0x11 /* GROUP: Sync                                       */,\
  0x04 /* NUM_PROPS                                         */,\
  0x01 /* START_PROP                                        */,\
  0xD0 /* SYNC_BITS,BITS[7:0],BITS[7:0],BITS[7:0],BITS[7:0] */,\
  0xE6 /* DATA1                                             */,\
  0x00 /* DATA2                                             */,\
  0x00 /* DATA3								*/\

#define RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS1_0  \
  0x11 /* GROUP: Sync                                       */,\
  0x04 /* NUM_PROPS                                         */,\
  0x01 /* START_PROP                                        */,\
  0x2F /* SYNC_BITS,BITS[7:0],BITS[7:0],BITS[7:0],BITS[7:0] */,\
  0x19 /* DATA1                                             */,\
  0x00 /* DATA2                                             */,\
  0x00 /* DATA3                                             */\

#define RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS1_1  \
  0x11 /* GROUP: Sync                                       */,\
  0x04 /* NUM_PROPS                                         */,\
  0x01 /* START_PROP                                        */,\
  0x98 /* SYNC_BITS,BITS[7:0],BITS[7:0],BITS[7:0],BITS[7:0] */,\
  0xF4 /* DATA1                                             */,\
  0x00 /* DATA2                                             */,\
  0x00 /* DATA3                                             */\


#endif /* FRAMEWORK_HAL_CHIPS_SI4460_EZRADIODRV_SI4460_REGISTERS_H_ */
