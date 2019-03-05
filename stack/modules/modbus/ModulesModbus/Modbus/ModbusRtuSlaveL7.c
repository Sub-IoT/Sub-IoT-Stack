/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/Modules/Modbus/ModbusRtuSlaveL7.c $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*		Modbus/RTU slave protocol - Layer 7\n
*		Handles message processing, data read/write\n
*		Currently supports only one instance\n
*		Supports function 3: read N holding registers\n
*		Supports function 16: write N holding registers\n
*/


/*-- Includes --*/
#include "TimersL1.h"
#include "ModbusRtuSlaveintrL6.h"
#include "ModbusRtuSlaveL7.h"
#include "emmacro.h"
#include "hwatomic.h"

#include "Testmode.h"
#include "SoftwareVersion.h"
#include "log.h"
 

/*-- Local definitions --*/
/*! \cond *//* Local definitions shouldn't be documented */

/*! \endcond *//* End of local definitions */

/*-- Local types --*/
/*! \cond *//* Local types shouldn't be documented */

/*! \endcond *//* End of local types */

/*-- Local data --*/

/* ModbusRtuSlaveIntr.c does not use com port buffers, it does use a modbus buffer in tpModbus */

/*-- Private prototypes --*/
static return_t ReadRegisters(uint32_t Register, uint32_t Count, uint8_t aData[]);
static return_t WriteRegisters(uint32_t Register, uint32_t Count, uint8_t aData[]);

static uint16_t ReadW16ViaP8BigEndian(uint8_t p[]);
static void WriteW16ViaP8BigEndian(uint8_t p[], uint16_t w);


/*-- Public functions --*/


/*!
*	Performs total reset of the Modbus/RTU (Layer 7 and all layers below)
*/
void ModbusRtuSlaveIntrL7_Reset( void)
{
    ModbusRtuSlaveIntrL6_Reset();

    /* install a 1ms interrupt callback for L6 */
    TimersL1_AddCallBack(&ModbusRtuSlaveIntrL6_TimerHandler, 1u);
}

/*!
*	Open (install) the Modbus/RTU slave
*
*	\param[in]		SlaveNr		Slave address
*/
void ModbusRtuSlaveIntrL7_Open( uint8_t SlaveNr)
{
    ModbusRtuSlaveIntrL6_Open(SlaveNr);
}

/*!
*	Do Modbus/RTU background processing
*/
void ModbusRtuSlaveIntrL7_WorkerThread( void)
{
    ModbusRtuSlaveIntrL6_WorkerThread();
}

/*!
*	Process Layer 6 Modbus request
*	Translates modbus requests to/from application specific variables
*
*	\param[in]		aRequestBuffer		buffer with query message, starts with function code
*	\param[in]		pResponseLength		on entry: size of request/response buffer
*											on exit: length of response message in response buffer
*	\param[in]		Options					Options
*
*	\retval		E_ERR_NOERROR		Request fullfilled
*	\retval		E_ERR_INVARG		IO not present or invalid request
*								Must also store modbus exception code in response buffer!
*/
return_t ModbusRtuSlaveIntrL7_ProcessRequest(uint8_t aRequestBuffer[], uint32_t* pResponseLength, uint32_t Options)
{
    log_print_string("process request\n");
    /* Exit code */
    return_t ErrorCode;
    /* Starting register */
    uint16_t usRegAddress;
    /* Number of registers in request */
    uint16_t usRegCount;

    /* Assume no errors */
    ErrorCode = E_ERR_NOERROR;

    /* Dispatch request */
    switch( aRequestBuffer[0])
    {
    case MDBF_RREGS:
        /* Read 16-bit registers */

        /* Get register address and register count */
        usRegAddress = M_NTOHS( M_TO16BIT(aRequestBuffer[2], aRequestBuffer[1]));
        usRegCount = M_NTOHS( M_TO16BIT(aRequestBuffer[4], aRequestBuffer[3]));

        /* number of registers fits in buffer? */
        if( (usRegCount * 2u) <= (*pResponseLength - 2u))
        {
            /* Echo function code at index 0 */

            /* read */
            ErrorCode = ReadRegisters((uint32_t)usRegAddress, (uint32_t)usRegCount, aRequestBuffer + 1u);

            /* return response frame length */
            *pResponseLength = 2u + (2u * usRegCount);
        }
        else
        {
            /* error */
            ErrorCode = E_ERR_INVARG;
        }

        /* processing ok? or send exception response? */
        if( ErrorCode == E_ERR_NOERROR)
        {
            /* ok */
        }
        else
        {
            /* Respond function code with error flag */
            aRequestBuffer[0] |= (uint8_t)0x80u;
            /* Respond exception code */
            aRequestBuffer[1] = MDBE_INVADDRESS;
            *pResponseLength = 2u;
        }
        break;

    case MDBF_WNREGS:
        /* Write 16-bit registers */

        /* Get register address and register count */
        usRegAddress = M_NTOHS(M_TO16BIT(aRequestBuffer[2], aRequestBuffer[1]));
        usRegCount = M_NTOHS(M_TO16BIT(aRequestBuffer[4], aRequestBuffer[3]));

        /* number of registers valid? */
        if( usRegCount < 127u)
        {
            /* Echo function code at index 0 */
            /* Echo address at positions 1/2 */
            /* Echo count at positions 3/4 */

            /* write */
            ErrorCode = WriteRegisters((uint32_t)usRegAddress, (uint32_t)usRegCount, &aRequestBuffer[6]);
            *pResponseLength = 5u;
        }
        else
        {
            /* error */
            ErrorCode = E_ERR_INVARG;
        }

        /* processing ok? or send exception response? */
        if( ErrorCode == E_ERR_NOERROR)
        {
            /* ok */
        }
        else
        {
            /* Respond function code with error flag */
            aRequestBuffer[0] |= (uint8_t)0x80u;
            /* Respond exception code */
            aRequestBuffer[1] = MDBE_INVADDRESS;
            *pResponseLength = 2u;
        }
        break;
    case MDBF_RD_ADDRS:
      
    default:
        /* invalid function code */
        aRequestBuffer[0] |= (uint8_t)0x80u;                  /* respond  function code with error flag */
        aRequestBuffer[1] = MDBE_INVFUNCTION;               /* respond exception code */
        *pResponseLength = 2u;
        ErrorCode = E_ERR_INVARG;
        break;
    }

    return ErrorCode;
}


/*-- Private functions --*/


/*!
*	Modbus read multiple 16-bit registers.
*	Implements registers 0..
*	returns 0 for all other registers (improves future compatiblity when newer masters read more variables)
*
*	\param[in]		Register	first register to write
*	\param[in]		Count		number of registers to write
*	\param[out]		pData		where to write databytes
*
*	\retval		E_ERR_NOERROR		ok
*	\retval		E_ERR_INVARG		argument error
*/
static return_t ReadRegisters(uint32_t usRegister, uint32_t Count, uint8_t aData[])
{
    log_print_string("read register\n");
    return_t sErrorCode = E_ERR_NOERROR;
    uint16_t Value = 0u;
    uint32_t Idx = 0u;

    /* first byte of response is byte count */
    aData[0] = (uint8_t)(Count * 2u);
    Idx++;

    /* for all requested registers */
    while((Count > 0u) && (sErrorCode == E_ERR_NOERROR))
    {	
        /* get data for this register */
        switch(usRegister)
        {
            
        case MBREG(40001):
            Value = M_VERSION_MAJOR;
            break;
            
        case MBREG(40002):
            Value = M_VERSION_MINOR;
            break;
            
        case MBREG(40003):
            Value = M_VERSION_REVISION;
            break;
    
        case MBREG(40004):
            Value = M_DATA_LEVEL;
            break;
    
        case MBREG(40005):
            Value = M_VARIANT;
            break;
            
        case MBREG(40024):
            Value = TestMode_GetTestMode() ? 1u: 0u;
            break;
        default:
            sErrorCode = E_ERR_INVARG;
            log_print_string("invalidReg\n");
            break;
        }

        /* write data word */
        WriteW16ViaP8BigEndian(&aData[Idx], Value);

        /* next register */
        Idx += 2u;
        ++usRegister;
        --Count;
    }

    return sErrorCode;
}

/*!
*	Modbus write multiple 16-bit registers.
*	Implements registers 100..
*	ignores writes to all other registers (improves future compatiblity when newer masters writes more variables)
*
*	\param[in]		Register	first register to write
*	\param[in]		Count		number of registers to write
*	\param[out]		pData		pointer to databytes
*
*	\retval		E_ERR_NOERROR		ok
*	\retval		E_ERR_INVARG		argument error
*/
static return_t WriteRegisters( uint32_t usRegister, uint32_t Count, uint8_t* pData)
{
    return_t sErrorCode = E_ERR_NOERROR;
    uint16_t usValue;

    /* for all requested registers */
    while( (Count>0) && (sErrorCode==E_ERR_NOERROR))
    {
        /* get data for this register */
        usValue = ReadW16ViaP8BigEndian( pData);

        /* get data for this register */
        switch(usRegister)
        {
        case MBREG(49501u):
            if (usValue == 12349u)
            {
                TestMode_SetTestMode(true);
            }
            else
            {
                TestMode_SetTestMode(false);
            }
            break;

        default:
            sErrorCode = E_ERR_INVARG;
            break;
        }
    

    /* next register */
    pData += 2;
    ++usRegister;
    --Count;
    }

    return( sErrorCode);
}

/*!
*	Read a 16-bit word through a pointer-to-8bit.
*	Assumes the pointer points to a 16-bit word in big endian format.
*	Intended to fix MISRA errors.
*
*	\param[in]		p        Buffer
*
*	\return		The 16-bit word
*/
static uint16_t ReadW16ViaP8BigEndian(uint8_t p[])
{
    uint16_t Result;

    Result = M_TO16BIT(p[0], p[1]);

    return Result;
}

/*!
*	Write a 16-bit word through a pointer-to-8bit.
*	Intended to fix MISRA errors.
*	Outputs the 16-bit word in big endian format.
*
*	\param[in]		p		Buffer
*	\param[in]		w		16-bit word to be written
*/
static void WriteW16ViaP8BigEndian(uint8_t p[], uint16_t w)
{
    p[0] = M_HIGHBYTE(w); /* parasoft-suppress MISRA2004-12_8 "Use library code that is tested thoroughly" */
    p[1] = M_LOWBYTE(w); /* parasoft-suppress MISRA2004-12_8 "Use library code that is tested thoroughly" */
}
