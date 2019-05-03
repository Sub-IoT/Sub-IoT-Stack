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
#include "hwuart.h"


/*-- Local definitions --*/
/*! \cond *//*	Local definitions shouldn't be documented */


static uart_handle_t* uart;


/*! \endcond *//*	End of local definitions */

/*-- Local types --*/
/*! \cond *//*	Local types shouldn't be documented */

struct ComL1DeviceData
{
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
static void uart_rx_cb(uint8_t data);


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
	//TODO check if enabled
	if (pThis->bIsOpen != false)
	{
		assert(uart_disable(uart));
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
		uart_send_bytes(uart, pData, size);

		pThis->bTxBusy = false;
	
		if (pThis->TransmitIsrHandler != NULL)
		{
			pThis->TransmitIsrHandler(pThis->transmitIsrCallbaData); 
		}

	}	
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
	uart = uart_init(0, baudrate,0);
	/* Start receiving */
	uart_set_rx_interrupt_callback(uart, &uart_rx_cb);
	assert(uart_enable(uart));
  	uart_rx_interrupt_enable(uart);
	//TODO register error callback
	/*
	if (pThis->bTxBusy)
	{
		pThis->bTxBusy = false;	
	}
	pThis->ErrorIsrHandler(pThis->errorIsrCallbaData);
	*/
}

/*!
* GUID: - Callback for uart data
*
* \param[in]		data		byte received from UART
*/
static void uart_rx_cb(uint8_t data)
{
	comL1DeviceData_t *const pThis = &s_aComL1DevicesData[0];
	pThis->receiveByte=data;
	pThis->ReceiveIsrHandler(pThis->receiveByte, pThis->receiveIsrCallbaData);
}
