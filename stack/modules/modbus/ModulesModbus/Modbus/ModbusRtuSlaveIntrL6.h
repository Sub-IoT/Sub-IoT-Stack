/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/Modules/Modbus/ModbusRtuSlaveIntrL6.h $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	Modbus RTU Slave interrupt callbacks\n
*/

#ifndef MODBUSRTUSLAVEINTRL6_H__
#define MODBUSRTUSLAVEINTRL6_H__


/*-- Includes --*/
#include "emtype.h"
/*-- Public definitions --*/

#define MODBUS_COM_INTERFACES		1u

/*-- Public types --*/

/*-- Public functions --*/
void ModbusRtuSlaveIntrL6_Reset( void);
void ModbusRtuSlaveIntrL6_Open( uint8_t SlaveId);
void ModbusRtuSlaveIntrL6_WorkerThread( void);
void ModbusRtuSlaveIntrL6_InstanceWorker(uint32_t instanceID);
return_t ModbusRtuSlaveIntrL6_TxHandler(uint32_t modbusInstance);
void ModbusRtuSlaveIntrL6_ErrorHandler(uint32_t modbusInstance);
void ModbusRtuSlaveIntrL6_RxHandler( uint16_t RxCharStatus, uint32_t modbusInstance);
void ModbusRtuSlaveIntrL6_TimerHandler( void);

#endif /* !MODBUSRTUSLAVEINTRL6_H__ */




