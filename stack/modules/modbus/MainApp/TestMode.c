/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/MainApp/TestMode.c $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	API for the test mode according to ED&A products \n
*/

/*-- Includes --*/
#include "assert.h"

#include "TestMode.h"

/*-- Local definitions --*/

/*-- Local types --*/
/*! \cond *//* Local types shouldn't be documented */

/*! \endcond *//* End of local types */


/*-- internal functions --*/


/*-- private data --*/
static bool s_bTestMode = false;

/*-- public functions --*/

/*!
*	Enables or disables test mode
*
*	\param[in]		value		state of testmode
*/
void TestMode_SetTestMode(bool enable)
{
    s_bTestMode = enable;
}

/*!
*	Returns if testmode is active or not
*
*	\return		state of testmode
*/
bool TestMode_GetTestMode(void)
{
	return s_bTestMode;
}

/*!
*	Make test mode work
*/
void TestMode_DoWork(void)
{

}

/*-- Private functions --*/
