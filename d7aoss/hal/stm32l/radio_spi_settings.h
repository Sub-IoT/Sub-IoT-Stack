/*
 * radio_spi_settings.h
 *
 *  Created on: Mar 12, 2014
 *      Author: armin
 */

#ifndef RADIO_SPI_SETTINGS_H_
#define RADIO_SPI_SETTINGS_H_


#include "stm32l1xx.h"
#include "stm32l1xx_gpio.h"

#define SPIx					SPI2

#define SPIx_IRQn				SPI2_IRQn
#define SPIx_IRQHANDLER			SPI2_IRQHandler
#define SPIx_ENABLE_CLK 		RCC_APB1PeriphClockCmd
#define SPIx_CLK 				RCC_APB1Periph_SPI2

#define SPIx_SCK_GPIO_CLK 		RCC_AHBPeriph_GPIOB
#define SPIx_SCK_GPIO_PORT 		GPIOB
#define SPIx_SCK_SOURCE 		GPIO_PinSource13
#define SPIx_SCK_AF 			GPIO_AF_SPI2
#define SPIx_SCK_PIN 			GPIO_Pin_13

#define SPIx_MOSI_GPIO_CLK 		RCC_AHBPeriph_GPIOB
#define SPIx_MOSI_GPIO_PORT		GPIOB
#define SPIx_MOSI_SOURCE		GPIO_PinSource15
#define SPIx_MOSI_AF			GPIO_AF_SPI2
#define SPIx_MOSI_PIN			GPIO_Pin_15

#define SPIx_MISO_GPIO_CLK 		RCC_AHBPeriph_GPIOB
#define SPIx_MISO_GPIO_PORT		GPIOB
#define SPIx_MISO_SOURCE		GPIO_PinSource14
#define SPIx_MISO_AF			GPIO_AF_SPI2
#define SPIx_MISO_PIN			GPIO_Pin_14

#define SPIx_NSS_GPIO_CLK 		RCC_AHBPeriph_GPIOB
#define SPIx_NSS_GPIO_PORT		GPIOB
#define SPIx_NSS_PIN			GPIO_Pin_12

#define GDO0_GPIO_CLK 			RCC_AHBPeriph_GPIOA
#define GDO0_GPIO_PORT			GPIOA
#define GDO0_PORT_SOURCE		EXTI_PortSourceGPIOA
#define GDO0_PIN_SOURCE			EXTI_PinSource1
#define GDO0_SOURCE				GPIO_PinSource1
#define GDO0_GPIO_PIN			GPIO_Pin_1
#define GDO0_EXTI_LINE			EXTI_Line1
#define GDO0_IRQn				EXTI1_IRQn
#define GDO0_IRQHandler			EXTI1_IRQHandler

#define GDO2_GPIO_CLK 			RCC_AHBPeriph_GPIOA
#define GDO2_GPIO_PORT			GPIOA
#define GDO2_PORT_SOURCE		EXTI_PortSourceGPIOA
#define GDO2_PIN_SOURCE			EXTI_PinSource0
#define GDO2_SOURCE				GPIO_PinSource0
#define GDO2_GPIO_PIN			GPIO_Pin_0
#define GDO2_EXTI_LINE			EXTI_Line0
#define GDO2_IRQn				EXTI0_IRQn
#define GDO2_IRQHandler			EXTI0_IRQHandler


#endif /* RADIO_SPI_SETTINGS_H_ */