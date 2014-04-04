/**
 ******************************************************************************
 * @file    hw_config.c
 * @author  MCD Application Team
 * @version V3.4.0
 * @date    29-June-2012
 * @brief   Hardware Configuration & Setup
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx.h"
#include "stm32l1xx_usart.h"
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_syscfg.h"
#include "misc.h"

#include "usb_istr.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "hw_config.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ErrorStatus HSEStartUpStatus;
USART_InitTypeDef USART_InitStructure;

uint8_t USART_Rx_Buffer[USART_RX_DATA_SIZE];
uint32_t USART_Rx_ptr_in = 0;
uint32_t USART_Rx_ptr_out = 0;
uint32_t USART_Rx_length = 0;

uint8_t USB_Tx_State = 0;
static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len);
/* Extern variables ----------------------------------------------------------*/

//extern LINE_CODING linecoding;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : Set_System
 * Description    : Configures Main system clocks & power
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void Set_System(void)
{
#if !defined(STM32F10X_CL) && !defined(STM32L1XX_MD) && !defined(STM32L1XX_HD) && !defined(STM32L1XX_MD_PLUS)
	GPIO_InitTypeDef GPIO_InitStructure;
#endif /* STM32F10X_CL && STM32L1XX_XD */  

#if defined(USB_USE_EXTERNAL_PULLUP)
	GPIO_InitTypeDef GPIO_InitStructure;
#endif /* USB_USE_EXTERNAL_PULLUP */ 

	/*!< At this stage the microcontroller clock setting is already configured,
	 this is done through SystemInit() function which is called from startup
	 file (startup_stm32f10x_xx.s) before to branch to application main.
	 To reconfigure the default setting of SystemInit() function, refer to
	 system_stm32f10x.c file
	 */
#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS)
	/* Enable the SYSCFG module clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
#endif /* STM32L1XX_XD */ 

#if !defined(STM32F10X_CL) && !defined(STM32L1XX_MD) && !defined(STM32L1XX_HD) && !defined(STM32L1XX_MD_PLUS)
	/* Enable USB_DISCONNECT GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

	/* Configure USB pull-up pin */
	GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);
#endif /* STM32F10X_CL && STM32L1XX_XD */

#if defined(USB_USE_EXTERNAL_PULLUP)
	/* Enable the USB disconnect GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIO_DISCONNECT, ENABLE);

	/* USB_DISCONNECT used as USB pull-up */
	GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);
#endif /* USB_USE_EXTERNAL_PULLUP */  
}

/*******************************************************************************
 * Function Name  : Set_USBClock
 * Description    : Configures USB Clock input (48MHz)
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void Set_USBClock(void)
{
#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD) || defined(STM32L1XX_MD_PLUS) 
	/* Enable USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

#elif defined(STM32F10X_CL)
	/* Select USBCLK source */
	RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div3 );

	/* Enable the USB clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);

#else 
	/* Select USBCLK source */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

	/* Enable the USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
#endif /* STM32F10X_CL */
}

/*******************************************************************************
 * Function Name  : Enter_LowPowerMode
 * Description    : Power-off system clocks and power while entering suspend mode
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void Enter_LowPowerMode(void)
{
	/* Set the device state to suspend */
	bDeviceState = SUSPENDED;
}

/*******************************************************************************
 * Function Name  : Leave_LowPowerMode
 * Description    : Restores system clocks and power while exiting suspend mode
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo = &Device_Info;

	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0)
			{
		/* Device configured */
		bDeviceState = CONFIGURED;
	}
	else
	{
		bDeviceState = ATTACHED;
	}
}

/*******************************************************************************
 * Function Name  : USB_Interrupts_Config
 * Description    : Configures the USB interrupts
 * Input          : None.
 * Return         : None.
 *******************************************************************************/
void USB_Interrupts_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1 );

#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD) || defined(STM32L1XX_MD_PLUS) 
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#elif defined(STM32F10X_CL) 
	/* Enable the USB Interrupts */
	NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#else
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif /* STM32L1XX_XD */
}

/*******************************************************************************
 * Function Name  : USB_Cable_Config
 * Description    : Software Connection/Disconnection of USB Cable
 * Input          : None.
 * Return         : Status
 *******************************************************************************/
void USB_Cable_Config(FunctionalState NewState)
{
#if defined(STM32L1XX_MD) || defined (STM32L1XX_HD)|| (STM32L1XX_MD_PLUS)
	if (NewState != DISABLE)
	{
		STM32L15_USB_CONNECT;
	}
	else
	{
		STM32L15_USB_DISCONNECT;
	}

#elif defined(STM32F10X_CL)
	if (NewState != DISABLE)
			{
		USB_DevConnect();
	}
	else
	{
		USB_DevDisconnect();
	}

#else
	if (NewState != DISABLE)
	{
		GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
	}
	else
	{
		GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
	}
#endif
}

/*******************************************************************************
 * Function Name  : Handle_USBAsynchXfer.
 * Description    : send data to USB.
 * Input          : None.
 * Return         : none.
 *******************************************************************************/
void Handle_USBAsynchXfer(void)
{

	uint16_t USB_Tx_ptr;
	uint16_t USB_Tx_length;

	if (USB_Tx_State != 1)
			{
		if (USART_Rx_ptr_out == USART_RX_DATA_SIZE)
		{
			USART_Rx_ptr_out = 0;
		}

		if (USART_Rx_ptr_out == USART_Rx_ptr_in)
				{
			USB_Tx_State = 0;
			return;
		}

		if (USART_Rx_ptr_out > USART_Rx_ptr_in) /* rollback */
		{
			USART_Rx_length = USART_RX_DATA_SIZE - USART_Rx_ptr_out;
		}
		else
		{
			USART_Rx_length = USART_Rx_ptr_in - USART_Rx_ptr_out;
		}

		if (USART_Rx_length > VIRTUAL_COM_PORT_DATA_SIZE)
		{
			USB_Tx_ptr = USART_Rx_ptr_out;
			USB_Tx_length = VIRTUAL_COM_PORT_DATA_SIZE;

			USART_Rx_ptr_out += VIRTUAL_COM_PORT_DATA_SIZE;
			USART_Rx_length -= VIRTUAL_COM_PORT_DATA_SIZE;
		}
		else
		{
			USB_Tx_ptr = USART_Rx_ptr_out;
			USB_Tx_length = USART_Rx_length;

			USART_Rx_ptr_out += USART_Rx_length;
			USART_Rx_length = 0;
		}
		USB_Tx_State = 1;

#ifdef STM32F10X_CL
		USB_SIL_Write(EP1_IN, &USART_Rx_Buffer[USB_Tx_ptr], USB_Tx_length);
#else
		UserToPMABufferCopy(&USART_Rx_Buffer[USB_Tx_ptr], ENDP1_TXADDR, USB_Tx_length);
		SetEPTxCount(ENDP1, USB_Tx_length);
		SetEPTxValid(ENDP1);
#endif
	}

}

void USB_Send_Data(unsigned char* data, int len) {
	for (int i = 0; i < len; i++) {
		USART_Rx_Buffer[USART_Rx_ptr_in] = data[i];
		USART_Rx_ptr_in++;
		/* To avoid buffer overflow */
		if (USART_Rx_ptr_in == USART_RX_DATA_SIZE)
		{
			USART_Rx_ptr_in = 0;
		}
	}

}

void USB_Send_Character(char data)
{
	USART_Rx_Buffer[USART_Rx_ptr_in] = data;
	USART_Rx_ptr_in++;
	/* To avoid buffer overflow */
	if (USART_Rx_ptr_in == USART_RX_DATA_SIZE)
	{
		USART_Rx_ptr_in = 0;
	}

}

/*******************************************************************************
 * Function Name  : Get_SerialNum.
 * Description    : Create the serial number string descriptor.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Get_SerialNum(void)
{
	uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD) || defined(STM32L1XX_MD_PLUS) 
	Device_Serial0 = *(uint32_t*)(0x1FF80050);
	Device_Serial1 = *(uint32_t*)(0x1FF80054);
	Device_Serial2 = *(uint32_t*)(0x1FF80064);
#else  
	Device_Serial0 = *(__IO uint32_t*) (0x1FFFF7E8);
	Device_Serial1 = *(__IO uint32_t*) (0x1FFFF7EC);
	Device_Serial2 = *(__IO uint32_t*) (0x1FFFF7F0);
#endif /* STM32L1XX_XD */  

	Device_Serial0 += Device_Serial2;

	if (Device_Serial0 != 0)
			{
		IntToUnicode(Device_Serial0, &Virtual_Com_Port_StringSerial[2], 8);
		IntToUnicode(Device_Serial1, &Virtual_Com_Port_StringSerial[18], 4);
	}
}

/*******************************************************************************
 * Function Name  : HexToChar.
 * Description    : Convert Hex 32Bits value into char.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len)
{
	uint8_t idx = 0;

	for (idx = 0; idx < len; idx++)
			{
		if (((value >> 28)) < 0xA)
				{
			pbuf[2 * idx] = (value >> 28) + '0';
		}
		else
		{
			pbuf[2 * idx] = (value >> 28) + 'A' - 10;
		}

		value = value << 4;

		pbuf[2 * idx + 1] = 0;
	}
}
#ifdef STM32F10X_CL
/*******************************************************************************
 * Function Name  : USB_OTG_BSP_uDelay.
 * Description    : provide delay (usec).
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void USB_OTG_BSP_uDelay(const uint32_t usec)
{
	RCC_ClocksTypeDef RCC_Clocks;

	/* Configure HCLK clock as SysTick clock source */
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK );

	RCC_GetClocksFreq(&RCC_Clocks);

	SysTick_Config(usec * (RCC_Clocks.HCLK_Frequency / 1000000));

	SysTick ->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

	while (!(SysTick ->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
		;
}
#endif /* STM32F10X_CL */

#ifndef STM32F10X_CL
/*******************************************************************************
* Function Name  : USB_IRQHandler
* Description    : This function handles USB Low Priority interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_IRQHandler(void)
#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS)
#else
void USB_LP_CAN1_RX0_IRQHandler(void)
#endif
{
  USB_Istr();
}
#endif /* STM32F10X_CL */
/*******************************************************************************
* Function Name  : EVAL_COM1_IRQHandler
* Description    : This function handles EVAL_COM1 global interrupt request.
 Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

#ifdef STM32F10X_CL
/*******************************************************************************
* Function Name  : OTG_FS_IRQHandler
* Description    : This function handles USB-On-The-Go FS global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void OTG_FS_IRQHandler(void)
{
  STM32_PCD_OTG_ISR_Handler();
}
#endif /* STM32F10X_CL */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
