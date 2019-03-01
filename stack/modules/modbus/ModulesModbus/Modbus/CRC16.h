/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/Modules/Modbus/CRC16.h $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	Modbus/RTU CRC calculator\n
*/

#ifndef CRC16_H__
#define CRC16_H__


/*-- Includes --*/

/*-- Public definitions --*/

/*-- Public types --*/

/*-- Public functions --*/
void CalcCrc( uint8_t aBuffer[], uint32_t Length, uint16_t * pCRC);


#endif /* !CRC16_H__ */
