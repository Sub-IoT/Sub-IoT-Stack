/*!
*	\file
*	$HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/HAL/ComL1.c $
*	$LastChangedRevision: 3 $
*
*	$Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
*	$LastChangedBy: geert $
*
*	\par Company:
*	E.D.&A.\n
*
*	\par Description:
*	Driver for com ports (uarts)\n
*/

/*-- Includes --*/

#include <string.h>
#include "debug.h"
#include "log.h"
#include "ComL1.h"
#include "stm32_device.h"
#include "hwatomic.h"

/*-- Suppress Parasoft rules --*/

/*-- MISRA rules --*/
/*
* Description: "Integral type 'unsigned int' should not be converted to pointer to object type 'RTC_TypeDef *'"
*/
/* parasoft suppress item MISRA2012-RULE-11_4 reason "Conversion in external library (STM32 Cube)"*/

/*
* Description: "Constant used as the right-hand operand of a shift operator shall be limited"
*/
/* parasoft suppress item MISRA2012-RULE-12_2 reason "Shift in external library (STM32 Cube)"*/

/*
* Description: "Implicit conversion between signed and unsigned type shall not be used"
*/
/* parasoft suppress item MISRA2004-10_1_a reason "Conversion in external library (STM32 Peripheral library)" */

/*
* Description: "Cast from 'xxx' type to xxx' type is not allowed"
*/
/* parasoft suppress item MISRA2004-11_3_b reason "Cast in external library (STM32 Peripheral library)" */

/*-- Local definitions --*/
/*! \cond *//*	Local definitions shouldn't be documented */
#define M_UART_MODBUS						(USART2)
#define M_UART_MODBUS_CLK_ENABLE()			__HAL_RCC_USART2_CLK_ENABLE() /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for portability" */

#define M_UART_MODBUS_FORCE_RESET()			(__HAL_RCC_USART2_FORCE_RESET()) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for portability" */
#define M_UART_MODBUS_RELEASE_RESET()		(__HAL_RCC_USART2_RELEASE_RESET()) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for portability" */

/* Definition for UART_MODBUS Pins */
#define M_UART_MODBUS_TX_PIN				(GPIO_PIN_2)
#define M_UART_MODBUS_TX_GPIO_PORT			(GPIOA)
//#define M_UART_MODBUS_TX_AF					(GPIO_AF1_USART2)
#define M_UART_MODBUS_RX_PIN				(GPIO_PIN_3)
#define M_UART_MODBUS_RX_GPIO_PORT			(GPIOA)
//#define M_UART_MODBUS_RX_AF					(GPIO_AF1_USART2)

#define M_UART_MODBUS_IRQN					(USART2_IRQn)

DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* Also check for interrupt handler name */
/* Also check peripheral clock configuration */


/*! \endcond *//*	End of local definitions */

/*-- Local types --*/
/*! \cond *//*	Local types shouldn't be documented */

struct ComL1DeviceData
{
	UART_HandleTypeDef uartHandle;
	
	bool bIsOpen;
	uint32_t BaudRate;
	bool bTxBusy;
	
	uint8_t receiveByte;
	
	comReceiveCallback_t *ReceiveIsrHandler;
	uint32_t receiveIsrCallbaData;
	
	comTransmitCallback_t *TransmitIsrHandler;
	uint32_t transmitIsrCallbaData;
	
	comErrorCallback_t *ErrorIsrHandler;
	uint32_t errorIsrCallbaData;
};

/*! \endcond *//*	End of local types */

/*-- Private prototypes --*/
static void InitComPort(comL1DeviceData_t *const pThis, const uint32_t baudrate);
static uint8_t ConvertUartHandleToComPort(UART_HandleTypeDef const * const uartHandle);

/* Prototype for interrupt handler */
void USART2_IRQHandler(void);

/*-- Local data --*/
static comL1DeviceData_t s_aComL1DevicesData[M_COM_NPORTS];

/*-- Public functions --*/

/*!
* GUID: - Initializes a COM ports
*/
void ComL1_Init(void)
{
	/* zero all */
	(void)memset( s_aComL1DevicesData, 0, sizeof(s_aComL1DevicesData));
}

/*!
* GUID: - Get serial port handle
*
* \param[in] port Number of port to get handle from
*
* \return Handle to serial port
*/
comL1DeviceData_t * ComL1_GetPortHandle(uint8_t port)
{
	comL1DeviceData_t * p;
	
	/*	valid port number? */
	if( /*	(Port>=0) && */ (port < M_COM_NPORTS))
	{
		p = &s_aComL1DevicesData[port];
	}
	else
	{
		assert(false);
	}
	
	return (p);
}


/*!
* GUID: - Open a serial com port
*
* \param[inout] pThis			 ptr to this port
* \param[in] ulBaudRate		 bits per second
* \param[in] ucData			 5,6,7 or 8 number of databits
* \param[in] ucParity parity
* \param[in] ucStop			 1 or 2 number of stopbits
*/
void ComL1_OpenPort(comL1DeviceData_t *const pThis, const uint32_t ulBaudRate)
{
	pThis->bTxBusy = false;
	
	/* program uart registers */
	/* uart setup */
	InitComPort(pThis, ulBaudRate);
	
	pThis->BaudRate = ulBaudRate;
	
	/* if we get here all was well */
	pThis->bIsOpen = true;
	
	
}

/*!
* GUID: - Closes the port
*
* \param[in,out]		pThis			ptr to this port
*/
void ComL1_Close(comL1DeviceData_t *const pThis)
{
	/* only do this if port is open */
	if (pThis->bIsOpen != false)
	{
		assert(HAL_UART_DeInit(&pThis->uartHandle) == HAL_OK); /* parasoft-suppress MISRA2012-RULE-17_7_a "Check with M_ASSERT" */
		
		pThis->bIsOpen = false;
		pThis->bTxBusy = false;
		
	}
}


/*!
* GUID: - Transmits a number of bytes. Interrupt driven so returns immediately.
*
* \param[in,out]		pThis			ptr to this port
*/
void ComL1_Transmit(comL1DeviceData_t *const pThis, uint8_t *pData, uint16_t size)
{
	if (pThis->bTxBusy == false)
	{
		pThis->bTxBusy = true;
		
		assert(HAL_UART_Transmit_DMA(&pThis->uartHandle, pData, size) == HAL_OK); /* parasoft-suppress MISRA2012-RULE-17_7_a "Check with M_ASSERT" */
		
	}	
}

/*!
*	GUID: - Receive one byte
*
*	\param[in]		pThis ptr to this port
*
*/
void ComL1_ReceiveByte(comL1DeviceData_t *const pThis)
{
	uint8_t temp=HAL_UART_Receive_DMA(&pThis->uartHandle, &pThis->receiveByte, 1u);
	assert(temp == HAL_OK); /* parasoft-suppress MISRA2012-RULE-17_7_a "Check with M_ASSERT" */
}

void DMA1_Channel4_5_6_7_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel4_5_6_7_IRQn 0 */

  /* USER CODE END DMA1_Channel4_5_6_7_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
  /* USER CODE BEGIN DMA1_Channel4_5_6_7_IRQn 1 */

  /* USER CODE END DMA1_Channel4_5_6_7_IRQn 1 */
}


/*!
* GUID: - Set receive interrupt callback handler
*
* \param[in] callback Method to be called for handling received bytes
* \param[in] pCallbaData Data to be passed as parameter to the callback method
*/
void ComL1_SetRxCallback(comL1DeviceData_t *const pThis, comReceiveCallback_t *callback, uint32_t callbaData)
{
	start_atomic();
	
	pThis->ReceiveIsrHandler = callback;
	pThis->receiveIsrCallbaData = callbaData;
	
	end_atomic();
}

/*!
* GUID: - Set transmit interrupt callback handler, function to get next byte
*
* \param[in] Callback Method to be called for handling received bytes
* \param[in] pCallbaData Data to be passed as parameter to the callback method
*/
void ComL1_SetTxCallback(comL1DeviceData_t *const pThis, comTransmitCallback_t *callback, uint32_t callbaData)
{
	start_atomic();
	
	pThis->TransmitIsrHandler = callback;
	pThis->transmitIsrCallbaData = callbaData;
	
	end_atomic();
}

/*!
* GUID: - Set error interrupt callback handler
*
* \param[in] callback Method to be called for handling received bytes
* \param[in] callbaData Data to be passed as parameter to the callback method
*/
void ComL1_SetErrorCallback(comL1DeviceData_t *const pThis, comErrorCallback_t *callback, uint32_t callbaData)
{
	start_atomic();
	
	pThis->ErrorIsrHandler = callback;
	pThis->errorIsrCallbaData = callbaData;
	
	end_atomic();
}

/*-- Private functions --*/



/*!
* GUID: - Initializes the UART (baudrate, wordlength, stop bits, parity)
*
* \param[inout]		pThis			ptr to this port
* \param[in]		baudrate		bits per second
* \param[in]		data			8 or 9 number of databits
* \param[in]		parity			parity
* \param[in]		stopBits		1 or 2 number of stopbits
*/
static void InitComPort(comL1DeviceData_t *const pThis, const uint32_t baudrate)
{
		/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Channel4_5_6_7_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

	if (pThis == &s_aComL1DevicesData[M_COM_MODBUS])
	{
		pThis->uartHandle.Instance = M_UART_MODBUS;
	}
	else
	{
		assert(false);
	}
	
	pThis->uartHandle.Init.BaudRate = baudrate;
	pThis->uartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	pThis->uartHandle.Init.StopBits = UART_STOPBITS_1;
	pThis->uartHandle.Init.Parity = UART_PARITY_NONE;
	pThis->uartHandle.Init.Mode = UART_MODE_TX_RX;	
	pThis->uartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	pThis->uartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
	pThis->uartHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	pThis->uartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	if(HAL_UART_Init(&pThis->uartHandle) != HAL_OK)
	{
		assert(false);
	}
	
	/* Start receiving */
	ComL1_ReceiveByte(pThis);
	
}


/**
* @brief Tx Transfer completed callback
* @param UartHandle: UART handle. 
* @note This example shows a simple way to report end of IT Tx transfer, and 
* you can add your own implementation. 
* @retval None
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle) /* parasoft-suppress MISRA2012-RULE-8_13_a "Prototype defined in CubeMX library" */ /* parasoft-suppress EDnA-217 "Prototype defined in CubeMX library" */
{
	comL1DeviceData_t *const pThis = &s_aComL1DevicesData[ConvertUartHandleToComPort(UartHandle)];
	
	pThis->bTxBusy = false;
	
	if (pThis->TransmitIsrHandler != NULL)
	{
		pThis->TransmitIsrHandler(pThis->transmitIsrCallbaData); /* parasoft-suppress MISRA2012-RULE-17_7_a "Return code is not used" */
	}
}


/**
* @brief Rx Transfer completed callback
* @param UartHandles[port]: UART handle
* @note 
* @retval None
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) /* parasoft-suppress MISRA2012-RULE-8_13_a "Prototype defined in CubeMX library" */ /* parasoft-suppress EDnA-217 "Prototype defined in CubeMX library" */
{
	comL1DeviceData_t *const pThis = &s_aComL1DevicesData[ConvertUartHandleToComPort(UartHandle)];

 	if( pThis->ReceiveIsrHandler != NULL)
	{
	
		pThis->ReceiveIsrHandler(pThis->receiveByte, pThis->receiveIsrCallbaData);
	}
	
	/* Start receiving again */
	ComL1_ReceiveByte(pThis);
}

/**
* @brief Error callback
* @param UartHandles[port]: UART handle
* @note
* @retval None
*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle) /* parasoft-suppress EDnA-217 "Prototype defined in CubeMX library" */
{
	comL1DeviceData_t *const pThis = &s_aComL1DevicesData[ConvertUartHandleToComPort(UartHandle)];
	
	/* Disable the TXE and Transmit Complete Interrupt Interrupt */
	/* because Cube library leaves them enabled */
	__HAL_UART_DISABLE_IT(UartHandle, UART_IT_TXE);
	__HAL_UART_DISABLE_IT(UartHandle, UART_IT_TC);

	/* Stop transmitting when busy */
	if (pThis->bTxBusy)
	{
		pThis->bTxBusy = false;	
	}
	
	if( pThis->ErrorIsrHandler != NULL)
	{
		pThis->ErrorIsrHandler(pThis->errorIsrCallbaData);
	}
	
	/* Start receiving again */

	ComL1_ReceiveByte(pThis);
}

/**
* @brief This function handles UART interrupt request. 
* @param None
* @retval None
* @Note This function is redefined in "main.h" and related to DMA 
* used for USART data transmission 
*/
void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&s_aComL1DevicesData[M_COM_MODBUS].uartHandle);
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  const uint8_t port = ConvertUartHandleToComPort(huart);

  if(huart->Instance==M_UART_MODBUS)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    M_UART_MODBUS_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = M_UART_MODBUS_RX_PIN|M_UART_MODBUS_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL; //GPIO_PULLUP?
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; //GPIO_SPEED_FREQ_LOW?
    GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Channel5;
    hdma_usart2_rx.Init.Request = DMA_REQUEST_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      assert(false);
    }

    __HAL_LINKDMA(huart,hdmarx,hdma_usart2_rx);

    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Channel4;
    hdma_usart2_tx.Init.Request = DMA_REQUEST_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      assert(false);
    }

    __HAL_LINKDMA(huart,hdmatx,hdma_usart2_tx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(M_UART_MODBUS_IRQN, 0, 0);
    HAL_NVIC_EnableIRQ(M_UART_MODBUS_IRQN);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
  else
  {
	  assert(false);
  }
  

}



/**
* @brief UART MSP De-Initialization 
* This function frees the hardware resources used in this example:
* - Disable the Peripheral's clock
* - Revert GPIO and NVIC configuration to their default state
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart) /* parasoft-suppress MISRA2012-RULE-8_13_a "Prototype defined in CubeMX library" */
{
	if(huart->Instance==M_UART_MODBUS) 
	{
		/*##-1- Reset peripherals ##################################################*/
		//M_UART_MODBUS_FORCE_RESET(); //needed?
		//M_UART_MODBUS_RELEASE_RESET();//needed?
		/* Peripheral clock disable */
		__HAL_RCC_USART2_CLK_DISABLE();
		
		/*##-2- Disable peripherals and GPIO Clocks #################################*/
		/* Configure UART Tx as alternate function */
		HAL_GPIO_DeInit(M_UART_MODBUS_TX_GPIO_PORT, M_UART_MODBUS_TX_PIN|M_UART_MODBUS_RX_PIN);
		/* USART2 DMA DeInit */
		HAL_DMA_DeInit(huart->hdmarx);
    	HAL_DMA_DeInit(huart->hdmatx);
		
		/*##-3- Disable the NVIC for UART ##########################################*/
		HAL_NVIC_DisableIRQ(M_UART_MODBUS_IRQN);
	}
	else
	{
		assert(false);
	}
}


/*!
*	Convert uart handle to modbus port handle
*	Caution: asserts when com port handle is not found
*
*	\param[in]		uartHandle		Uart handle
*
*	\return	Com port handle
*/
static uint8_t ConvertUartHandleToComPort(UART_HandleTypeDef const * const uartHandle)
{	
	uint8_t idx = 0u;
	for (idx = 0u; idx < M_COM_NPORTS; idx++)
	{
		if (&s_aComL1DevicesData[idx].uartHandle == uartHandle)
		{
			break;
		}
	}

	/* Check if handle is found */
	assert(idx < M_COM_NPORTS);

	return (idx);
}
