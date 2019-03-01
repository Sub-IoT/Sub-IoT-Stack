/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/Modules/Modbus/ModbusRtuSlaveL7.h $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	Modbus/RTU slave protocol - Layer 7 header file\n
*/

#ifndef MODBUSRTU_SLAVEL7_H__
#define MODBUSRTU_SLAVEL7_H__


/*-- Includes --*/
#include "emtype.h"
/*-- Public definitions --*/

#define MDBF_RCOILS         1u
#define MDBF_RINPCOILS      2u
#define MDBF_RREGS          3u
#define MDBF_RINPREGS       4u
#define MDBF_WCOIL          5u
#define MDBF_WREG           6u
#define MDBF_WNCOILS       15u
#define MDBF_WNREGS        16u
#define MDBF_RWREGS        23u

#define MDBF_RD_ADDRS      100u
#define MDBF_WR_ADDRS      101u


#define MDBE_INVFUNCTION    1u
#define MDBE_INVADDRESS     2u
#define MDBE_INVVALUE       3u

/* macro to convert from public address 40001... to offset 0... */
#define MBREG(x)		((x) - 40001u) /* parasoft-suppress MISRA2004-19_7 "Needed for readable, clear and understandable code" */

/*-- Public types --*/

/*-- Public functions --*/
void ModbusRtuSlaveIntrL7_Reset(void);
void ModbusRtuSlaveIntrL7_Open( uint8_t SlaveNr);
void ModbusRtuSlaveIntrL7_WorkerThread( void);
return_t ModbusRtuSlaveIntrL7_ProcessRequest(uint8_t aRequestBuffer[], uint32_t* pResponseLength, uint32_t Options);


#endif /* !MODBUSRTU_SLAVEL7_H__ */
