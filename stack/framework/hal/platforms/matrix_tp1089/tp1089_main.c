/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "scheduler.h"
#include "bootstrap.h"
#include "hwgpio.h"
#include "hwuart.h"
#include "hwleds.h"
#include "hwdebug.h"
#include "platform.h"
#include "kl02z_mcu.h"
// TODO #include "em_gpio.h"
#include <assert.h>
#include <MKL02Z4.h>

static void SetSIMRegisters()
{
  // TODO
  SIM->COPC = SIM_COPC_COPT(0);//disable watchdog immediately

  //do the rest in order listed in data sheet

  SIM->SOPT2 = SIM_SOPT2_UART0SRC(0)|SIM_SOPT2_TPMSRC(0);//no clocks enabled  for
                                                        // uart0 or TPM counterClock

  SIM->SOPT4 = ((0 << SIM_SOPT4_TPM1CLKSEL_SHIFT)&SIM_SOPT4_TPM1CLKSEL_MASK)|((0 << SIM_SOPT4_TPM0CLKSEL_SHIFT)&SIM_SOPT4_TPM0CLKSEL_MASK)|((0<<SIM_SOPT4_TPM1CH0SRC_SHIFT)&SIM_SOPT4_TPM1CH0SRC_MASK);//don't care settings

  SIM->SOPT5 = ((0<<SIM_SOPT5_UART0ODE_SHIFT)&SIM_SOPT5_UART0ODE_MASK)|((0<<SIM_SOPT5_UART0RXSRC_SHIFT)&SIM_SOPT5_UART0RXSRC_MASK)|((0<<SIM_SOPT5_UART0TXSRC_SHIFT)&SIM_SOPT5_UART0TXSRC_MASK);//don't cares

  SIM->SOPT7 = ((0<<SIM_SOPT7_ADC0ALTTRGEN_SHIFT)&SIM_SOPT7_ADC0ALTTRGEN_MASK)|((0<<SIM_SOPT7_ADC0PRETRGSEL_SHIFT)&SIM_SOPT7_ADC0PRETRGSEL_MASK)|((0<<SIM_SOPT7_ADC0TRGSEL_SHIFT)&SIM_SOPT7_ADC0TRGSEL_MASK);//don't cares

  SIM->SCGC4 = ((1<<SIM_SCGC4_SPI0_SHIFT)&SIM_SCGC4_SPI0_MASK)|((0<<SIM_SCGC4_CMP_SHIFT)&SIM_SCGC4_CMP_MASK)|((0<<SIM_SCGC4_UART0_SHIFT)&SIM_SCGC4_UART0_MASK)|((1<<SIM_SCGC4_I2C1_SHIFT)&SIM_SCGC4_I2C1_MASK)|((1<<SIM_SCGC4_I2C0_SHIFT)&SIM_SCGC4_I2C0_MASK);//enable the I2C module

  SIM->SCGC5 = ((1<<SIM_SCGC5_PORTB_SHIFT)&SIM_SCGC5_PORTB_MASK)|((1<<SIM_SCGC5_PORTA_SHIFT)&SIM_SCGC5_PORTA_MASK)|((0<<SIM_SCGC5_LPTMR_SHIFT)&SIM_SCGC5_LPTMR_MASK);//enable both ports

  SIM->SCGC6 = ((0<<SIM_SCGC6_ADC0_SHIFT)&SIM_SCGC6_ADC0_MASK)|((0<<SIM_SCGC6_TPM1_SHIFT)&SIM_SCGC6_TPM1_MASK)|((0<<SIM_SCGC6_TPM0_SHIFT)&SIM_SCGC6_TPM0_MASK)|((0<<SIM_SCGC6_FTF_SHIFT)&SIM_SCGC6_FTF_MASK);//don't cares

  SIM->CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0)|SIM_CLKDIV1_OUTDIV4(4);//clk configured for 4MHz core, .8 MHz Bus Clk (fastest allowable in BLPI mode)

  //this is unchanged from default, do not alter. If we need to save power we can turn off flash, but this has caused hard faults. we need to move the interrupt vectors out of flash to do this SIM_FCFG1 = ((0<<SIM_FCFG1_FLASHDOZE_SHIFT)&SIM_FCFG1_FLASHDOZE_MASK)|((0<<SIM_FCFG1_FLASHDIS_SHIFT)&SIM_FCFG1_FLASHDIS_MASK);//flash disabled to conserve power, other settings are don't cares, TODO:verify flash size setting is unimportant

  //SIM_FCFG2 = SIM_FCFG2 only has maxaddr0, which is read only.

  //SIM_SRVCOP, we do not need to reset timer, it is  disabled already.

  return;
}

static void SetPorts()
{

 /* PortA
  * 7 6 5 4 3 2 1 0
  * | | | | | | | |_SWD_CLK
  * | | | | | | |___nRST
  * | | | | | |_____SWD_DIO
  * | | | | |_______SCL(i2c0)
  * | | | |_________SDA(i2C0)
  * | | |___________DCI
  * | |_____________INT
  * |_______________RESET
  *
  * PortB
  * 7 6 5 4 3 2 1 0
  * | | | | | | | |_SCLK
  * | | | | | | |___CSB
  * | | | | | |_____ATEST
  * | | | | |_______D1(red LED)
  * | | | |_________INT
  * | | |___________PWR_TEMP
  * | |_____________DNE - NOT PART OF PACKAGE
  * |_______________DNE - NOT PART OF PACKAGE
  *
  */
	  GPIOA_PCOR |= 0x0080;//clear output BEFORE setting it as an output pin
	  GPIOA_PDDR |= 0x0080;//set as output
	  GPIOA_PDDR &= ~0x0040;//int pin as an input
	  GPIOA_PCOR |= 0x0020;//disable impinj
	  GPIOA_PDDR |= 0x0020;

	  PORTA_PCR3 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(0)|PORT_PCR_MUX(2)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);//I2C0 SCL

	  PORTA_PCR4 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(0)|PORT_PCR_MUX(2)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);//I2C SDA

	  PORTA_PCR6 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(0)|PORT_PCR_MUX(3)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);//NCS TEMP(TMP108 CONNECTION)

	  PORTA_PCR7 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(0)|PORT_PCR_MUX(3)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);//NALERT(TMP108 CONNECTION)

	  PORTA_PCR5 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(0)|PORT_PCR_MUX(1)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);//DCI (IMPINJ POWER)

	  //"Reset" B.5 controls a FET that, when set will signal the power supply to turn off

	  PORTB_PCR0 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(0)|PORT_PCR_MUX(3)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);

	  PORTB_PCR1 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(0)|PORT_PCR_MUX(1)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);

	  PORTB_PCR2 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(0)|PORT_PCR_MUX(1)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);

	  PORTB_PCR3 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(0)|PORT_PCR_MUX(1)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);

	  PORTB_PCR4 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(9)|PORT_PCR_MUX(1)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);

	  PORTB_PCR5 = ((0<<PORT_PCR_ISF_SHIFT)&PORT_PCR_ISF_MASK)|PORT_PCR_IRQC(0)|PORT_PCR_MUX(1)|((0<<PORT_PCR_DSE_SHIFT)&PORT_PCR_DSE_MASK)|((0<<PORT_PCR_PFE_SHIFT)&PORT_PCR_PFE_MASK)|((1<<PORT_PCR_SRE_SHIFT)&PORT_PCR_SRE_MASK)|((0<<PORT_PCR_PE_SHIFT)&PORT_PCR_PE_MASK)|((0<<PORT_PCR_PS_SHIFT)&PORT_PCR_PS_MASK);

	  GPIOA_PCOR |= 0x0020;//clear output BEFORE setting it as an output pin
	  GPIOA_PDDR |= 0x0020;//set as output
	  GPIOA_PDDR &= ~0x0040;//int pin as an input

	  GPIOB_PSOR |= 0x000A;
	  GPIOB_PDDR |= 0x002A;

	  //enable interrupt
	  PORTB_ISFR = 0xFFFF;//clear any/all flags present before enabling
#define  PortB_irq_no           31  // Vector No 47 // TODO
	  NVIC->ICPR[0] = 1 << (PortB_irq_no);//enable the portB interrupt here
	  NVIC->ISER[0] = 1 << (PortB_irq_no);

	  return;
}

/*******************************************************************************
 * \brief       Sets power registers (allows VLPR, VLPS modes)
 * \param 	N/A
 * \return      N/A
 ******************************************************************************/

static void SetPowerRegisters()
{
  SMC->PMPROT = ((1<<SMC_PMPROT_AVLP_SHIFT)|SMC_PMPROT_AVLP_MASK)|((0<<SMC_PMPROT_AVLLS_SHIFT)|SMC_PMPROT_AVLLS_MASK);//allow low power modes to be entered via PMCTRL

  SMC->PMCTRL = SMC_PMCTRL_RUNM(2)|SMC_PMCTRL_STOPM(2);//run in VLPR mode, entering sleep will put us in VLPS mode

  return;
}

/*******************************************************************************
 * \brief       Sets up system clocks, all are disabled except for MCGOUTCLK to allow VLPR and VLPS
 * \param 	N/A
 * \return      N/A
 ******************************************************************************/

static void SetMCGRegisters()
{
  /*
  *  MCGIRCKLK - DISABLED
  *  MCGOUTCLK - 4MHz
  *  MCGFLLCLK - DISABLED
  *  MCGFFCLK - DISABLED
  *  *****FROM SIM SETTINGS*****
  */
  MCG->C1 = MCG_C1_CLKS(1)|MCG_C1_FRDIV(0)|((1<<MCG_C1_IREFS_SHIFT)&MCG_C1_IREFS_MASK)|((0<<MCG_C1_IRCLKEN_SHIFT)&MCG_C1_IRCLKEN_MASK)|((0<<MCG_C1_IREFSTEN_SHIFT)&MCG_C1_IREFSTEN_MASK);

  MCG->C2 = ((0<<MCG_C2_LOCRE0_SHIFT)&MCG_C2_LOCRE0_MASK)|((0<<MCG_C2_FCFTRIM_SHIFT)&MCG_C2_FCFTRIM_MASK)|MCG_C2_RANGE0(0)|((0<<MCG_C2_HGO0_SHIFT)&MCG_C2_HGO0_MASK)|((1<<MCG_C2_EREFS0_SHIFT)&MCG_C2_EREFS0_MASK)|((1<<MCG_C2_LP_SHIFT)&MCG_C2_LP_MASK)|((1<<MCG_C2_IRCS_SHIFT)&MCG_C2_IRCS_MASK);

  MCG->C4 |= ((1<<MCG_C4_DMX32_SHIFT)&MCG_C4_DMX32_MASK)|MCG_C4_DRST_DRS(0);//OR'D TO PRESERVE FACTORY TRIM SETTINGS
  //MCG_C6 RESETS TO THE DESIRED VALUE(CME CLEARED)

  MCG->SC = ((0<<MCG_SC_ATME_SHIFT)&MCG_SC_ATME_MASK)|((1<<MCG_SC_ATMS_SHIFT)&MCG_SC_ATMS_MASK)|((0<<MCG_SC_FLTPRSRV_SHIFT)&MCG_SC_FLTPRSRV_MASK)|MCG_SC_FCRDIV(0);


  return;
}

void __platform_init()
{
    __kl02z_mcu_init();
    SetSIMRegisters();
    SetPorts();
    SetPowerRegisters();
    SetMCGRegisters();
    __gpio_init();

#ifdef USE_CC1101
    //TODO: add calls to hw_gpio_configure_pin for the pins used by the CC1101 driver
    //(and possibly the spi interface)

    // configure the interrupt pins here, since hw_gpio_configure_pin() is MCU specific and not part of the common HAL API
    //TODO hw_gpio_configure_pin(CC1101_GDO0_PIN, true, gpioModeInput, 0); // TODO pull up or pull down to prevent floating
    //hw_gpio_configure_pin(CC1101_GDO2_PIN, true, gpioModeInput, 0) // TODO pull up or pull down to prevent floating // TODO not used for now
#endif
   // __hw_debug_init();

    error_t err;
    // TODO
//    err = hw_gpio_configure_pin(BUTTON0, true, gpioModeInput, 0); assert(err == SUCCESS); // TODO pull up or pull down to prevent floating
//    err = hw_gpio_configure_pin(BUTTON1, true, gpioModeInput, 0); assert(err == SUCCESS); // TODO pull up or pull down to prevent floating
}

void __platform_post_framework_init()
{
}

int main()
{
    //initialise the platform itself
	__platform_init();
    //do not initialise the scheduler, this is done by __framework_bootstrap()
    __framework_bootstrap();
    //initialise platform functionality that depends on the framework
    __platform_post_framework_init();
    scheduler_run();
    return 0;
}
