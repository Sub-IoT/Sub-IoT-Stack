/*!
 *
 * \copyright (C) Copyright 2013 University of Antwerp (http://www.cosys-lab.be) and others.\n
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.\n
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * Contributors:
 * 		maarten.weyn@uantwerpen.be
 *     	glenn.ergeerts@uantwerpen.be
 *
 */

#include "../system.h"
#include "../leds.h"
#include "../button.h"
#include "../uart.h"
#include "cc430_addresses.h"
#include "../../dae/fs.h"
#include "../../d7aoss.h"

#include "pre_init.h"
//#include "inc/hw_memmap.h"
//#include "driverlib/5xx_6xx/wdt.h"
#include "driverlib/5xx_6xx/tlv.h"
//#include "driverlib/5xx_6xx/pmm.h"
#include "driverlib/5xx_6xx/ucs.h"

//#include "cc430_registers.h"

#include "driverlib/inc/hw_types.h"

//uint8_t device_id[8];
//uint8_t virtual_id[2];
uint32_t clock_speed;

 uint8_t vCore_level = 2;
//uint16_t target_clock_speed_kHz = 10000;
//uint16_t target_clock_speed_kHz = 1677216;
static const uint32_t target_clock_speed_Hz = 4194304;
 uint8_t init_IO = 1;

void clock_init(void);


void PMM_SetStdSVSM(unsigned short svsmh_cfg, uint8_t Von, uint8_t Voffon) {
    unsigned short svsmh_reg;
    unsigned short svsml_reg;

    PMMCTL0_H   = 0xA5;
    svsmh_reg   = svsmh_cfg | ((unsigned short)Von << 8) | (unsigned short)Voffon;
    svsml_reg   = SVSMLCTL & 0x070F;
    svsml_reg  |= svsmh_reg & 0x08C0;   // Always disable SVML (useless)
    PMMIFG      = 0;
    PMMRIE      = 0;
    SVSMHCTL    = svsmh_reg;
    SVSMLCTL    = svsml_reg;
    while ((PMMIFG & (SVSMLDLYIFG+SVSMHDLYIFG)) != (SVSMLDLYIFG+SVSMHDLYIFG));

    PMMIFG        = 0;
    PMMRIE        = 0x0130;               //Always enable SVSL reset, SVMH interrupt, SVS/MH Delayed interrupt
    PMMCTL0_H    = 0x00;
}

void SetVCoreUp (unsigned char level)        // Note: change level by one step only
{
  PMMCTL0_H = 0xA5;                         // Open PMM module registers for write access

  SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;     // Set SVS/M high side to new level

  SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;     // Set SVM new Level
  while ((PMMIFG & SVSMLDLYIFG) == 0);      // Wait till SVM is settled (Delay)
  PMMCTL0_L = PMMCOREV0 * level;            // Set VCore to x
  PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);        // Clear already set flags
  if ((PMMIFG & SVMLIFG))
    while ((PMMIFG & SVMLVLRIFG) == 0);     // Wait till level is reached

  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;     // Set SVS/M Low side to new level
  PMMCTL0_H = 0x00;                         // Lock PMM module registers for write access
}

void SetVCoreDown (unsigned char level)
{
  PMMCTL0_H = 0xA5;                         // Open PMM module registers for write access
  SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;     // Set SVS/M Low side to new level
  //Wait until SVM high side and SVM low side is settled
  while (PMMIFG & SVSMLDLYIFG == 0) ;
  //while ((PMM->IFG & SVSMLDLYIFG) == 0);      // Wait till SVM is settled (Delay)
  PMMCTL0_L = (level * PMMCOREV0);          // Set VCore to 1.85 V for Max Speed.
  PMMCTL0_H = 0x00;                         // Lock PMM module registers for write access
}

void PMM_SetVCore (uint8_t level) {
    unsigned char actLevel;

    // Note: change level by one step only
    do {
        actLevel = PMMCTL0_L & PMMCOREV_3;
        if (actLevel < level)
            SetVCoreUp(++actLevel);               // Set VCore (step by step)
        if (actLevel > level)
            SetVCoreDown(--actLevel);             // Set VCore (step by step)
    } while (actLevel != level);
}

void system_init(uint8_t* tx_buffer, uint16_t tx_buffer_size, uint8_t* rx_buffer, uint16_t rx_buffer_size)
{
	system_watchdog_timer_stop();

    PMM_SetVCore(vCore_level);

    PMM_SetStdSVSM(0x8088, 2, 4);

    clock_init();

    if (init_IO)
    {
    	// Init all ports
		PADIR = 0xFF;
		PAOUT = 0x00;
		PBDIR = 0xFF;
		PBOUT = 0x00;
		PCDIR = 0xFF;
		PCOUT = 0x00;

    	led_init();

		#ifdef BUTTONS
    	button_init();
		#endif


		#ifdef UART
    	uart_init();
		#endif

    }

    queue_init_with_header(&tx_queue, tx_buffer, tx_buffer_size, 1, 30);
    queue_init(&rx_queue, rx_buffer, rx_buffer_size, 1);


    system_check_set_unique_id();
}

void clock_init(void)
{
//	UCSCTL1 = DCORSEL_5; // 0x0050
	//UCSCTL1 = DCORSEL_1;
//	UCSCTL2 = 0x01F9;
//	UCSCTL3 = 0x0020;
//	UCSCTL4 = 0x0233;
//	UCSCTL5 = 0x0040;
//	UCSCTL6 = 0x0100;

	//Set DCO FLL reference = REFO
	UCS_clockSignalInit(
			__MSP430_BASEADDRESS_UCS_RF__,
		UCS_FLLREF,
		UCS_REFOCLK_SELECT,
		UCS_CLOCK_DIVIDER_1
		);
	//Set ACLK = REFO
	UCS_clockSignalInit(
			__MSP430_BASEADDRESS_UCS_RF__,
		UCS_ACLK,
		UCS_REFOCLK_SELECT,
		UCS_CLOCK_DIVIDER_1
		);

	//Set Ratio and Desired MCLK Frequency  and initialize DCO
//	UCS_initFLLSettle(
//			__MSP430_BASEADDRESS_UCS_RF__,
//		1000, // 1000 khz
//		31   //  1000 kHz / 32.768 Khz (Crystal)
//		);

	_BIS_SR(SCG0);                  // Disable the FLL control loop
	UCS_initFLLSettle(
			__MSP430_BASEADDRESS_UCS_RF__,
			target_clock_speed_Hz / 1000, // 10000 khz
			(uint16_t) (target_clock_speed_Hz / 32768)   //  10000 kHz / 32.768 Khz  = 305(Crystal)
			);
	_BIC_SR(SCG0);                  // Enable the FLL control loop


	clock_speed = UCS_getSMCLK(__MSP430_BASEADDRESS_UCS_RF__);
	//unsigned long clockValueMCLK = UCS_getMCLK(__MSP430_BASEADDRESS_UCS_RF__);
	//unsigned long clockValueCLK = UCS_getACLK(__MSP430_BASEADDRESS_UCS_RF__);
}

void system_watchdog_timer_stop()
{
    //WDT_hold();
	unsigned char newWDTStatus = WDTCTL_L | WDTHOLD;
    WDTCTL = WDTPW + newWDTStatus;
}

void system_watchdog_timer_start()
{
    //WDT_hold();
	//WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK;
    unsigned char newWDTStatus = WDTCTL_L & ~WDTHOLD;
    WDTCTL = WDTPW + newWDTStatus;
}

void system_watchdog_timer_reset()
{
	//Set Counter Clear bit
	unsigned char newWDTStatus = ( WDTCTL_L | WDTCNTCL );
	WDTCTL = WDTPW + newWDTStatus;
}

void system_watchdog_timer_enable_interrupt()
{
	SFRIE1 |= WDTIE;
}

void system_watchdog_timer_init(unsigned char clockSelect, unsigned char clockDivider)
{
    WDTCTL = WDTPW + WDTCNTCL + WDTTMSEL + clockSelect + clockDivider;
}

void system_watchdog_init(unsigned char clockSelect, unsigned char clockDivider)
{
    WDTCTL = WDTPW + WDTCNTCL + clockSelect + clockDivider;
}

void system_lowpower_mode(unsigned char mode, unsigned char enableInterrupts)
{
    unsigned char registerSetting = 0;
    switch (mode)
    {
        case 1:
            registerSetting = LPM1_bits;
            break;
        case 2:
            registerSetting = LPM2_bits;
            break;
        case 3:
            registerSetting = LPM3_bits;
            break;
        case 4:
            registerSetting = LPM4_bits;
            break;
        case 0:
        default:
            registerSetting = LPM0_bits;
            break;
    }

    if (enableInterrupts)
        registerSetting += GIE;

    __bis_SR_register(registerSetting);
}

void system_check_set_unique_id()
{
	file_handler fh;
	fs_open(&fh, DA_FILE_ID_UID, file_system_user_root, file_system_access_type_read);
	device_id = fs_get_data_pointer(&fh, 0);

	uint8_t i = 0;
	bool device_id_null = true;
	for(;i<8;i++)
	{
		if (device_id[i] != 0x00)
		{
			device_id_null = false;
			break;
		}
	}

	if (device_id_null)
	{
		struct s_TLV_Die_Record * pDIEREC;
		unsigned char bDieRecord_bytes;

		TLV_getInfo(TLV_TAG_DIERECORD,
			0,
			&bDieRecord_bytes,
			(unsigned int **)&pDIEREC
			);

		unsigned char tagId[8];
		unsigned char* pointer = (unsigned char*) &(pDIEREC->wafer_id);
		tagId[0] = pointer[3];
		tagId[1] = pointer[2];
		tagId[2] = pointer[1];
		tagId[3] = pointer[0];
		pointer = (unsigned char*) &(pDIEREC->die_x_position);
		tagId[4] = pointer[1];
		tagId[5] = pointer[0];

		pointer = (unsigned char*) &(pDIEREC->die_y_position);
		tagId[6] = pointer[1];
		tagId[7] = pointer[0];

		fs_open(&fh, DA_FILE_ID_UID, file_system_user_root, file_system_access_type_write);

		fs_write_data(&fh, 0, (uint8_t*) tagId, 8, true);

		//TODO: correct way to find virtual_id -> set by app layer

		//virtual_id[0] = device_id[4] ^ device_id[5];
		//virtual_id[1] = device_id[6] ^ device_id[7];
	}
}
