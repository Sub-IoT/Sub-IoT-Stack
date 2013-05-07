/*
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 */

#include "system.h"
#include "leds.h"
#include "button.h"
#include "uart.h"
#include "cc430_addresses.h"

//#include "inc/hw_memmap.h"
//#include "driverlib/5xx_6xx/wdt.h"
#include "driverlib/5xx_6xx/tlv.h"
#include "driverlib/5xx_6xx/pmm.h"

//#include "cc430_registers.h"

#include "driverlib/inc/hw_types.h"

u8 tag_id[8];

void clock_init(void);


void PMM_SetStdSVSM(unsigned short svsmh_cfg, u8 Von, u8 Voffon) {
    unsigned short svsmh_reg;
    unsigned short svsml_reg;

    PMMCTL0_H    = 0xA5;
    svsmh_reg       = svsmh_cfg | ((unsigned short)Von << 8) | (unsigned short)Voffon;
    svsml_reg       = SVSMLCTL & 0x070F;
    svsml_reg      |= svsmh_reg & 0x08C0;   // Always disable SVML (useless)
    PMMIFG        = 0;
    PMMRIE        = 0;
    SVSMHCTL   = svsmh_reg;
    SVSMLCTL   = svsml_reg;
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

void PMM_SetVCore (u8 level) {
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

void system_init()
{
	 // Init all ports
	 PADIR = 0xFF;
	 PAOUT = 0x00;
	 PBDIR = 0xFF;
	 PBOUT = 0x00;
	 PCDIR = 0xFF;
	 PCOUT = 0x00;

    system_watchdog_timer_stop();

    //PMM_setVCore(PMMCOREV_2);
    PMM_SetVCore(2);

    PMM_SetStdSVSM(0x8088, 2, 4);

    clock_init();

    led_init();
    button_init();
    uart_init();

    system_get_unique_id(tag_id);
}

void clock_init(void)
{
	UCSCTL1 = 0x0050;
	UCSCTL2 = 0x01F9;
	UCSCTL3 = 0x0020;
	UCSCTL4 = 0x0233;
	UCSCTL5 = 0x0040;
	UCSCTL6 = 0x0100;
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

void system_get_unique_id(unsigned char *tagId)
{
    struct s_TLV_Die_Record * pDIEREC;
    unsigned char bDieRecord_bytes;

    TLV_getInfo(TLV_TAG_DIERECORD,
        0,
        &bDieRecord_bytes,
        (unsigned int **)&pDIEREC
        );

    //unsigned char tagId[8];
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
}
