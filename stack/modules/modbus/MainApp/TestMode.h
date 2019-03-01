/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/MainApp/TestMode.h $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
 *
 * \par Description:
 *  Handles the test mode setup and the loading & saving of test data.\n
*/
#ifndef TESTMODE_H__
#define TESTMODE_H__

/*-- Includes --*/
#include "stdbool.h"
/*-- Public definitions --*/

/*-- public functions --*/

void TestMode_SetTestMode(bool enable);
bool TestMode_GetTestMode(void);

void TestMode_DoWork(void);


#endif /* TESTMODE_H__ */
