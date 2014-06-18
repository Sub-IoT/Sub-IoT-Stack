/*
 * radio_spi_hw.c
 *
 *  Created on: Aug 6, 2012
 *      Author: armin
 */
#include <stdbool.h>

#include <misc.h>
#include <stm32l1xx.h>
#include <stm32l1xx_exti.h>
#include <stm32l1xx_gpio.h>
#include <stm32l1xx_rcc.h>
#include <stm32l1xx_spi.h>
#include <stm32l1xx_syscfg.h>

#include "../../phy/cc1101/cc1101_core.h" // TODO refactor, don't depend on this here
#include "radio_spi_hw.h"
#include "radio_spi_settings.h"

extern void CC1101_ISR (GDOLine gdo);
static GPIO_InitTypeDef GPIO_InitStructure;

void spiInit() {
//	debugprint("Init hardware spi\n");
	SPI_InitTypeDef SPI_InitStructure;
	/* Enable the SPI peripheral */SPIx_ENABLE_CLK(SPIx_CLK, ENABLE);
	/* Enable SCK, MOSI and MISO GPIO clocks */
	RCC_AHBPeriphClockCmd(SPIx_SCK_GPIO_CLK | SPIx_MISO_GPIO_CLK | SPIx_MOSI_GPIO_CLK | SPIx_NSS_GPIO_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

	/* SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
	GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);

	/* SPI  MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin = SPIx_MOSI_PIN;
	GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/* SPI MISO pin configuration */
	GPIO_InitStructure.GPIO_Pin = SPIx_MISO_PIN;
	GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStructure);

	/* SPI NSS pin configuration */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = SPIx_NSS_PIN;
	GPIO_Init(SPIx_NSS_GPIO_PORT, &GPIO_InitStructure);

	// Deselect radio
	SPIx_NSS_GPIO_PORT->BSRRL = SPIx_NSS_PIN;

	/* Map pins to SPIx */
	GPIO_PinAFConfig(SPIx_SCK_GPIO_PORT, SPIx_SCK_SOURCE, SPIx_SCK_AF );
	GPIO_PinAFConfig(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_SOURCE, SPIx_MOSI_AF );
	GPIO_PinAFConfig(SPIx_MISO_GPIO_PORT, SPIx_MISO_SOURCE, SPIx_MISO_AF );

	/* SPI configuration -------------------------------------------------------*/
	SPI_I2S_DeInit(SPIx );
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	/* Initializes the SPI communication */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPIx, &SPI_InitStructure);
//	SPIx->CR1 &= SPI_CR1_CRCEN; // disable CRC calculation
	SPI_Cmd(SPIx, ENABLE);
}

uint8_t spiSendByte(uint8_t data) {
	/*!< Loop while DR register in not empty or spi busy*/
//	while (!(SPIx ->SR & SPI_I2S_FLAG_TXE ))
	while (SPIx ->SR & SPI_I2S_FLAG_BSY )
	;
	/*!< Send byte through the SPI peripheral */SPIx ->DR = data;

	/*!< Wait to receive a byte */
	while (!(SPIx ->SR & SPI_I2S_FLAG_RXNE ))
	;
	/*!< Return the byte read from the SPI bus */
	return (uint8_t) SPIx ->DR;

}

void radioSelect() {
	// Select radio
	SPIx_NSS_GPIO_PORT ->BSRRH = SPIx_NSS_PIN;
	__NOP();
	__NOP();

	// Wait for radio to be ready
	while (SPIx_MISO_GPIO_PORT ->IDR & SPIx_MISO_PIN )
		;
}

void radioDeselect() {
	// deselect radio
	SPIx_NSS_GPIO_PORT ->BSRRL = SPIx_NSS_PIN;
}

void radioDisableGDO0Interrupt() {
	NVIC ->ICER[GDO0_IRQn >> 0x05] =
			(uint32_t) 0x01 << (GDO0_IRQn & (uint8_t) 0x1F);
}

void radioEnableGDO0Interrupt() {
	NVIC ->ISER[GDO0_IRQn >> 0x05] =
			(uint32_t) 0x01 << (GDO0_IRQn & (uint8_t) 0x1F);
}

void radioDisableGDO2Interrupt() {
	NVIC ->ICER[GDO2_IRQn >> 0x05] =
			(uint32_t) 0x01 << (GDO2_IRQn & (uint8_t) 0x1F);
}

void radioEnableGDO2Interrupt() {
	NVIC ->ISER[GDO2_IRQn >> 0x05] =
			(uint32_t) 0x01 << (GDO2_IRQn & (uint8_t) 0x1F);
}
void radioClearInterruptPendingLines() {
	EXTI ->PR = GDO2_EXTI_LINE | GDO0_EXTI_LINE;
}

void radioConfigureInterrupt(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	// GDO0
	RCC_AHBPeriphClockCmd(GDO0_GPIO_CLK, ENABLE);

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	/* Configure pins as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Pin = GDO0_GPIO_PIN;
	GPIO_Init(GDO0_GPIO_PORT, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(GDO0_PORT_SOURCE, GDO0_PIN_SOURCE );

	/* Enable and set EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = GDO0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	radioDisableGDO0Interrupt();

	// Configure interrupts
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Line = GDO0_EXTI_LINE;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);


	// GDO2
	RCC_AHBPeriphClockCmd(GDO2_GPIO_CLK, ENABLE);

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	/* Configure pins as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Pin = GDO2_GPIO_PIN;
	GPIO_Init(GDO2_GPIO_PORT, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(GDO2_PORT_SOURCE, GDO2_PIN_SOURCE );

	/* Enable and set EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = GDO2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
	radioDisableGDO2Interrupt();

	// Configure interrupts
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Line = GDO2_EXTI_LINE;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

void GDO2_IRQHandler(void) {
	if (EXTI ->PR  & GDO2_EXTI_LINE) {
		CC1101_ISR(GDOLine2);
		EXTI ->PR = GDO2_EXTI_LINE;
	}

}

void GDO0_IRQHandler(void) {
	if (EXTI ->PR  & GDO0_EXTI_LINE) {
		CC1101_ISR(GDOLine0);
		EXTI ->PR = GDO0_EXTI_LINE;
	}

}
