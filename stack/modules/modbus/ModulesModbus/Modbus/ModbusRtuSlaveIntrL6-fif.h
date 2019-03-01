/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/Modules/Modbus/ModbusRtuSlaveIntrL6-fif.h $
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

#ifndef MODBUSRTUSLAVEINTRL6_FIF_H__
#define MODBUSRTUSLAVEINTRL6_FIF_H__


/*-- Includes --*/

/*-- Public definitions --*/

/*-- Public types --*/

/*-- Public functions --*/
void ModbusRtuSlaveIntrL6_Fif_Open(uint8_t comPort, uint32_t modbusInstance);
void ModbusRtuSlaveIntrL6_Fif_Transmit(uint8_t comPort, uint8_t *pData, uint16_t size);

return_t ModbusRtuSlaveIntrL6_Fif_ProcessRequest(uint8_t aBuf[], uint32_t* pLen, uint32_t options);

#endif /* !MODBUSRTUSLAVEINTRL6_FIF_H__ */


