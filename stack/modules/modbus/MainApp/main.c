/*!
 *  \file
 *  $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/MainApp/main.c $
 *  $LastChangedRevision: 3 $
 *
 *  $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
 *  $LastChangedBy: geert $
 *
 *  \par Company:
 *      E.D.&A.\n
 *
 *  \par Description:
 *      main implementation\n
 */

/*-- Includes --*/
#include "TimersL1.h"
#include "ComL1.h"
#include "ModbusRtuSlaveL7.h"


/*-- Local definitions --*/
/*! \cond *//* Local definitions shouldn't be documented */
/*! \endcond *//* End of local definitions */

/*-- Local types --*/
/*! \cond *//* Local types shouldn't be documented */
/*! \endcond *//* End of local types */

/*-- Local data --*/

/*-- Private prototypes --*/

/*-- Public functions --*/

/*!
 *  Init main module
 */
void main(void)
{
   /*  CpuL1_Init();
    OSIntfL1_Init();
    TimersL1_Init();
    DigIoL1_Init();

	ComL1_OpenPort(ComL1_GetPortHandle(M_COM_MODBUS), 9600u);
	ModbusRtuSlaveIntrL7_Reset();
	ModbusRtuSlaveIntrL7_Open(1u);

    while(1)
    {
        ModbusRtuSlaveIntrL7_WorkerThread();
        //DigIoL1_DoWork();

        OSIntfL1_KickWatchDog();
    } */
}




/*-- Private functions --*/
