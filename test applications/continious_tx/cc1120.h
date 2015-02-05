#include <stdint.h>

#define MARCSTATE    0x73
#define  PARTNUMBER      0x8F


#define RF_SRES         0x30        // Reset chip.
#define RF_SFSTXON      0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
									// If in RX/TX: Go to a wait state where only the synthesizer is
									// running (for quick RX / TX turnaround).
#define RF_SXOFF        0x32        // Turn off crystal oscillator.
#define RF_SCAL         0x33        // Calibrate frequency synthesizer and turn it off
									// (enables quick start).
#define RF_SRX          0x34        // Enable RX. Perform calibration first if coming from IDLE and
									// MCSM0.FS_AUTOCAL=1.
#define RF_STX          0x35        // In IDLE state: Enable TX. Perform calibration first if
									// MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
									// Only go to TX if channel is clear.
#define RF_SIDLE        0x36        // Exit RX / TX, turn off frequency synthesizer and exit
									// Wake-On-Radio mode if applicable.
#define RF_SAFC         0x37        // Perform AFC adjustment of the frequency synthesizer
#define RF_SWOR         0x38        // Start automatic RX polling sequence (Wake-on-Radio)
#define RF_SPWD         0x39        // Enter power down mode when CSn goes high.
#define RF_SFRX         0x3A        // Flush the RX FIFO buffer.
#define RF_SFTX         0x3B        // Flush the TX FIFO buffer.
#define RF_SWORRST      0x3C        // Reset real time clock.
#define RF_SNOP         0x3D        // No operation. May be used to pad strobe commands to two

typedef struct {
    uint8_t iocfg3;           // GPIO3 IO Pin Configuration
    uint8_t iocfg2;           // GPIO2 IO Pin Configuration
    uint8_t iocfg1;           // GPIO1 IO Pin Configuration
    uint8_t iocfg0;           // GPIO0 IO Pin Configuration
    uint8_t sync_cfg1;        // Sync Word Detection Configuration Reg. 1
    uint8_t deviation_m;      // Frequency Deviation Configuration
    uint8_t modcfg_dev_e;     // Modulation Format and Frequency Deviation Configur..
    uint8_t dcfilt_cfg;       // Digital DC Removal Configuration
    uint8_t preamble_cfg1;    // Preamble Length Configuration Reg. 1
    uint8_t freq_if_cfg;      // RX Mixer Frequency Configuration
    uint8_t iqic;             // Digital Image Channel Compensation Configuration
    uint8_t chan_bw;          // Channel Filter Configuration
    uint8_t mdmcfg1;          // General Modem Parameter Configuration Reg. 1
    uint8_t mdmcfg0;          // General Modem Parameter Configuration Reg. 0
    uint8_t symbol_rate2;     // Symbol Rate Configuration Exponent and Mantissa [1..
    uint8_t symbol_rate1;     // Symbol Rate Configuration Mantissa [15:8]
    uint8_t symbol_rate0;     // Symbol Rate Configuration Mantissa [7:0]
    uint8_t agc_ref;          // AGC Reference Level Configuration
    uint8_t agc_cs_thr;       // Carrier Sense Threshold Configuration
    uint8_t agc_cfg1;         // Automatic Gain Control Configuration Reg. 1
    uint8_t agc_cfg0;         // Automatic Gain Control Configuration Reg. 0
    uint8_t fifo_cfg;         // FIFO Configuration
    uint8_t fs_cfg;           // Frequency Synthesizer Configuration
    uint8_t pkt_cfg2;         // Packet Configuration Reg. 2
    uint8_t pkt_cfg1;         // Packet Configuration Reg. 1
    uint8_t pkt_cfg0;         // Packet Configuration Reg. 0
    uint8_t pa_cfg0;          // Power Amplifier Configuration Reg. 0
    uint8_t if_mix_cfg;       // IF Mix Configuration
    uint8_t toc_cfg;          // Timing Offset Correction Configuration
    uint8_t freq2;            // Frequency Configuration [23:16]
    uint8_t freq1;            // Frequency Configuration [15:8]
    uint8_t fs_dig1;          // Frequency Synthesizer Digital Reg. 1
    uint8_t fs_dig0;          // Frequency Synthesizer Digital Reg. 0
    uint8_t fs_cal1;          // Frequency Synthesizer Calibration Reg. 1
    uint8_t fs_cal0;          // Frequency Synthesizer Calibration Reg. 0
    uint8_t fs_divtwo;        // Frequency Synthesizer Divide by 2
    uint8_t fs_dsm0;          // FS Digital Synthesizer Module Configuration Reg. 0
    uint8_t fs_dvc0;          // Frequency Synthesizer Divider Chain Configuration ..
    uint8_t fs_pfd;           // Frequency Synthesizer Phase Frequency Detector Con..
    uint8_t fs_pre;           // Frequency Synthesizer Prescaler Configuration
    uint8_t fs_reg_div_cml;   // Frequency Synthesizer Divider Regulator Configurat..
    uint8_t fs_spare;         // Frequency Synthesizer Spare
    uint8_t fs_vco0;          // FS Voltage Controlled Oscillator Configuration Reg..
    uint8_t xosc5;            // Crystal Oscillator Configuration Reg. 5
    uint8_t xosc1;            // Crystal Oscillator Configuration Reg. 1
    uint8_t serial_status;    // Serial Status
} RF_SETTINGS;
