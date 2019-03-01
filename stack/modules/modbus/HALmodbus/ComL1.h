/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/HAL/ComL1.h $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
* 
* \par Description:
*	Driver for com ports (uarts)\n
*/

#ifndef COML1_H__
#define COML1_H__

/*-- includes --*/
#include "emtype.h"
#include "assert.h"
#include "stm32_device.h"
#include "stm32_common_mcu.h"
/*-- public definitions --*/

/* Parity options */
#define M_PARITY_NONE		0u
#define M_PARITY_EVEN		1u
#define M_PARITY_ODD		2u


#define M_COM_MODBUS            0u
#define M_COM_NPORTS    		1u



/* typedef pointer to receive callback */
typedef void comReceiveCallback_t(uint16_t RxCharStatus, const uint32_t callbaData);

/* typedef pointer to transmitter complete callback */
typedef return_t comTransmitCallback_t(const uint32_t callbaData);

/* typedef pointer to error callback */
typedef void comErrorCallback_t(const uint32_t callbaData);


typedef struct ComL1DeviceData comL1DeviceData_t;

/*-- public functions --*/

#ifdef __cplusplus
extern "C"
{
#endif

void ComL1_Init(void);
comL1DeviceData_t * ComL1_GetPortHandle(uint8_t port);

void ComL1_OpenPort(comL1DeviceData_t *const pThis, const uint32_t ulBaudRate);
void ComL1_Close(comL1DeviceData_t *const pThis);

void ComL1_Transmit(comL1DeviceData_t *const pThis, uint8_t *pData, uint16_t size);
void ComL1_ReceiveByte(comL1DeviceData_t *const pThis);

bool ComL1_IsTxBusy(comL1DeviceData_t const *const pThis);
uint32_t ComL1_GetBaudRate(comL1DeviceData_t const *const pThis);

void ComL1_SetRxCallback(comL1DeviceData_t *const pThis, comReceiveCallback_t *callback, uint32_t callbaData);
void ComL1_SetTxCallback(comL1DeviceData_t *const pThis, comTransmitCallback_t *callback, uint32_t callbaData);
void ComL1_SetErrorCallback(comL1DeviceData_t *const pThis, comErrorCallback_t *callback, uint32_t callbaData);


#ifdef __cplusplus
}
#endif

#endif /* COML1_H__ */ 
