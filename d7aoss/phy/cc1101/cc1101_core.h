/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 *  	armin@otheruse.nl
 */

#ifndef RF1A_H
#define RF1A_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


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
    uint8_t rcctrl1;          // RC Oscillator Configuration
    uint8_t rcctrl0;          // RC Oscillator Configuration
    uint8_t fstest;           // Frequency Synthesizer Calibration Control
    uint8_t ptest;            // Production Test
    uint8_t agctest;          // AGC Test
    uint8_t test2;            // Various Test Settings
    uint8_t test1;            // Various Test Settings
    uint8_t test0;            // Various Test Settings
    uint8_t partnum;          // Chip ID
    uint8_t version;          // Chip ID
    uint8_t freqest;          // Frequency Offset Estimate from Demodulator
    uint8_t lqi;              // Demodulator Estimate for Link Quality
    uint8_t rssi;             // Received Signal Strength Indication
    uint8_t marcstate;        // Main Radio Control State Machine State
    uint8_t wortime1;         // High Byte of WOR Time
    uint8_t wortime0;         // Low Byte of WOR Time
    uint8_t pktstatus;        // Current GDOx Status and Packet Status
    uint8_t vco_vc_dac;       // Current Setting from PLL Calibration Module
    uint8_t txbytes;          // Underflow and Number of Bytes
    uint8_t rxbytes;          // Overflow and Number of Bytes
    uint8_t rcctrl1_status;   // Last RC Oscillator Calibration Result
    uint8_t rcctrl0_status;   // Last RC Oscillator Calibration Result
} RF_SETTINGS;

uint8_t Strobe(unsigned char strobe);
void ResetRadioCore(void);
void WriteRfSettings(RF_SETTINGS *pRfSettings);

uint8_t ReadSingleReg(uint8_t addr);
void WriteSingleReg(uint8_t addr, uint8_t value);
void ReadBurstReg(uint8_t addr, uint8_t* buffer, uint8_t count);
void WriteBurstReg(uint8_t addr, uint8_t* buffer, uint8_t count);
void WriteSinglePATable(uint8_t value);
void WriteBurstPATable(uint8_t* buffer, uint8_t count);

uint8_t ReadPartNum( void );
uint8_t ReadVersion( void );

#ifdef __cplusplus
}
#endif

#endif /* RF1A_H */
