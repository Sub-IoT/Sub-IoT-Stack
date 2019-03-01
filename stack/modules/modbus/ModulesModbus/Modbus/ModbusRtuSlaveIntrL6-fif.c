/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/Modules/Modbus/ModbusRtuSlaveIntrL6-fif.c $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	Interrupt-driven modbus communication, using no coml1 buffers\n
*/

/*-- Includes --*/
#include "emtype.h"
#include "emmacro.h"

#include "comL1.h"
#include "ModbusRtuSlaveL7.h"

#include "ModbusRtuSlaveIntrL6.h"
#include "ModbusRtuSlaveIntrL6-fif.h"

/*-- Local definitions --*/
/*! \cond *//* Local definitions shouldn't be documented */

/*! \endcond *//* End of local definitions */

/*-- Local types --*/
/*! \cond *//* Local types shouldn't be documented */

/*! \endcond *//* End of local types */

/*-- Local data --*/

/*-- Private prototypes --*/

static void ModbusRtuSlaveIntrL6_RxHandler_full(uint16_t RxCharStatus, uint32_t callbaData);
static return_t ModbusRtuSlaveIntrL6_TxHandler_full(uint32_t callbaData);
static void ModbusRtuSlaveIntrL6_ErrorHandler_full(uint32_t callbaData);

/*-- Public functions --*/

void ModbusRtuSlaveIntrL6_Fif_Open(uint8_t comPort, uint32_t modbusInstance)
{
	ComL1_SetRxCallback(ComL1_GetPortHandle(comPort), &ModbusRtuSlaveIntrL6_RxHandler_full, modbusInstance);
	ComL1_SetTxCallback(ComL1_GetPortHandle(comPort), &ModbusRtuSlaveIntrL6_TxHandler_full, modbusInstance);
	ComL1_SetErrorCallback(ComL1_GetPortHandle(comPort), &ModbusRtuSlaveIntrL6_ErrorHandler_full, modbusInstance);
}

/*!
*	Initiate transmission, pass first byte to transmit
*/
void ModbusRtuSlaveIntrL6_Fif_Transmit(uint8_t comPort, uint8_t *pData, uint16_t size)
{
	ComL1_Transmit(ComL1_GetPortHandle(comPort), pData, size);
}

/*!
*	Function to call callback on data received
*
*	\param[in]			pucRequestBuffer		pointer to buffer with query message, starts with function code
*	\param[inout]		pusResponseLength		on entry: size of request/response buffer
*												on exit: length of response message in response buffer
*	\param[in]			Options					Options
*
*	\retval				M_ENOERROR				Request fullfilled
*	\retval				M_EINVARG				IO not present or invalid request
*												Must also store modbus exception code in response buffer!
*/
return_t ModbusRtuSlaveIntrL6_Fif_ProcessRequest(uint8_t aBuf[], uint32_t* pLen, uint32_t Options)
{
	return ModbusRtuSlaveIntrL7_ProcessRequest( aBuf, pLen, Options);
}

/*-- Private functions --*/


static void ModbusRtuSlaveIntrL6_RxHandler_full(uint16_t RxCharStatus, uint32_t callbaData)
{
	ModbusRtuSlaveIntrL6_RxHandler(RxCharStatus, callbaData);
}

static return_t ModbusRtuSlaveIntrL6_TxHandler_full(uint32_t callbaData)
{
	return ModbusRtuSlaveIntrL6_TxHandler(callbaData);
}

static void ModbusRtuSlaveIntrL6_ErrorHandler_full(uint32_t callbaData)
{
	ModbusRtuSlaveIntrL6_ErrorHandler(callbaData);
}
