/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/Modules/Modbus/ModbusRtuSlaveIntrL6.c $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
*
* \par Description:
*	Modbus/RTU slave protocol, transport layer\n
*	Interrupt-driven modbus communication, using no coml1 buffers \n
*	Currently supports only one instance \n
*/

/*-- Includes --*/

#include "OsintfL1.h"
#include "ComL1.h" /* For com port defines */
#include "emmacro.h"
#include "Crc16.h"
#include "ModbusRtuSlaveIntrL6.h"
#include "ModbusRtuSlaveIntrL6-fif.h"
#include "log.h"
#include "debug.h"

/*-- Local definitions --*/
/*! \cond *//* Local definitions shouldn't be documented */

/** Application dependent definitions **/

#define M_BUFLEN    128u

#ifdef M_DEBUG
	/* can be active only for debug builds! */
	/* log received chars and times? */
	#define N_M_MODBUS_LOGRX

	/* count diagnostics? */
	#define M_MODBUS_DIAGNOSTICS
#endif

#ifdef M_MODBUS_LOGRX
	#define LOG_BUFFER_SIZE		100u
#endif

#define M_STATE_RX       0u
#define M_STATE_PROCESS  1u
#define M_STATE_DELAY    2u
#define M_STATE_TX       3u


/*! \endcond *//* End of local definitions */

/*-- Local types --*/
/*! \cond *//* Local types shouldn't be documented */


typedef struct
{
    uint8_t      SlaveNr;        /* modbus slave addres */
    uint8_t       Port;           /* serial port number */
    uint32_t      State;          /* state, see M_STATE_.. */
    uint8_t       aBuf[M_BUFLEN]; /* query/response buffer */
    uint32_t      RxCount;        /* number of bytes received */
    uint32_t      TxCount;        /* number of bytes transmitted */
    uint32_t      TxSize;         /* number of bytes to transmit */
    uint32_t      SyncTimeout;    /* timeout that detects end-of-frame */
    uint32_t      ResponseDelay;  /* delay before sending response */
    uint32_t      Time;           /* timer */
    /* RxCount and Time are changed and checked in diferent interrupt routines,
    *  therefore they must be changed or checked in one atomic operation!
    */
	
#ifdef M_MODBUS_DIAGNOSTICS
	struct
	{
		uint32_t NotForMe;
		uint32_t InvalidCrc;
		uint32_t RxBufOfl;
		uint32_t TooShort;
	} Diagnostics;
#endif

#ifdef M_MODBUS_LOGRX
	struct
	{
		uint8_t Ch;
		uint32_t Systime;
	} aLogRx[LOG_BUFFER_SIZE];
	
	uint8_t LogIndex;
#endif
} tpModbus;

/*! \endcond *//* End of local types */

/*-- Local data --*/
static tpModbus Modbus[MODBUS_COM_INTERFACES];

 
/*-- Private prototypes --*/
static void InstanceWorker(uint32_t instanceID);
static void InstanceTimerHandler(uint8_t instanceID);

/*-- Public functions --*/

/*!
*	Reset/init this module
*/
void ModbusRtuSlaveIntrL6_Reset( void)
{
	for (uint8_t idx = 0u; idx < MODBUS_COM_INTERFACES; idx++)
	{
		/* set default modbus frame end timeout */
		Modbus[idx].SyncTimeout = 4u;

		/* set default delay before responding */
		/* is to make sure that other slaves on a multidrop network */
		/* will detect the end of the query message */
		Modbus[idx].ResponseDelay = 1u;
	}
	
	Modbus[0].Port = M_COM_MODBUS;
}

/*!
*	Open the modbus slave
*
*	\param[in]		SlaveNr		Address of modbus slave
*/
void ModbusRtuSlaveIntrL6_Open(uint8_t SlaveId)
{
	for (uint32_t idx = 0u; idx < MODBUS_COM_INTERFACES; idx++)
	{
		/* set modbus slave address */
		Modbus[idx].SlaveNr = SlaveId;
		
		/* complete open */
		ModbusRtuSlaveIntrL6_Fif_Open(Modbus[idx].Port, idx);
	}
}

/*!
*	The worker thread of this module
*/
void ModbusRtuSlaveIntrL6_WorkerThread(void)
{
	static uint32_t currentModbus = 0u;
	
	InstanceWorker(currentModbus);
	
	currentModbus++;
	if (currentModbus >= MODBUS_COM_INTERFACES)
	{
		currentModbus = 0u;
	}
}

/*!
*	Received a byte via interrupt
*
*	\param[in]		RxCharStatus	Received byte
*/
void ModbusRtuSlaveIntrL6_RxHandler( uint16_t RxCharStatus, uint32_t modbusInstance)
{
#ifdef M_MODBUS_LOGRX
    /* optional logging of received characters and timestamps */
    Modbus[modbusInstance].aLogRx[Modbus[modbusInstance].LogIndex].Ch = (uint8_t)RxCharStatus;
    Modbus[modbusInstance].aLogRx[Modbus[modbusInstance].LogIndex].Systime = TimersL7_GetSystemTime();
    if( ++Modbus[modbusInstance].LogIndex >= LOG_BUFFER_SIZE)
	{
		Modbus[modbusInstance].LogIndex = 0u;
	}
#endif

    /* are we in receiving state? */
    if( Modbus[modbusInstance].State == M_STATE_RX)
    {
        
        /* room left? */
        if( Modbus[modbusInstance].RxCount < M_BUFLEN)
        {
            /* store byte */
            Modbus[modbusInstance].aBuf[Modbus[modbusInstance].RxCount] = (uint8_t)RxCharStatus;
            /* inc RxCount and reset Timer in one atomic operation */
			saveInterruptState_t x = OSSaveInterruptState() ;
			OSDisableInterrupts() ;
            ++Modbus[modbusInstance].RxCount;
            
            Modbus[modbusInstance].Time = 0u;
		    OSRestoreInterruptState(x);

        }
        else
        {
            /* no room */
#ifdef M_MODBUS_DIAGNOSTICS
            ++Modbus[modbusInstance].Diagnostics.RxBufOfl;
#endif
            /* reset timer, so we cleanly wait for the end of the message */
            Modbus[modbusInstance].Time = 0u;
        }
    }
}

/*!
*	Callback for lower layer TX IRQ
*
*	\param[in]	modbusInstance			instance id
*
*	\retval		E_ERR_NOERROR		No error
*	\retval		E_ERR_NOLEFT		No more bytes to send
*/
return_t ModbusRtuSlaveIntrL6_TxHandler(uint32_t modbusInstance)
{
    return_t ReturnCode = E_ERR_NOERROR;

    /* are we in transmission state? */
    if( Modbus[modbusInstance].State == M_STATE_TX)
    {
        /* transmission done */
        /* back to receive */
        Modbus[modbusInstance].RxCount = 0u;
        Modbus[modbusInstance].State = M_STATE_RX;
        ReturnCode = E_ERR_NOLEFT;
    }
    else
    {
        /* not transmitting */
        ReturnCode = E_ERR_NOLEFT;
    }

    return ReturnCode;
}

/*!
*	Callback for lower layer Error IRQ
*
*	\param[in]	modbusInstance			instance id
*/
void ModbusRtuSlaveIntrL6_ErrorHandler(uint32_t modbusInstance)
{
	/* error detected */
	/* back to receive */
	Modbus[modbusInstance].RxCount = 0u;
	Modbus[modbusInstance].State = M_STATE_RX;
}

/*!
*	Timer callback function
*	Must be called once every 1ms
*/
void ModbusRtuSlaveIntrL6_TimerHandler(void)
{
	for (uint8_t idx = 0u; idx < MODBUS_COM_INTERFACES; idx++)
	{
		InstanceTimerHandler(idx);
	}
}

/*-- Private functions --*/

/*!
*	Timer callback function
*	Must be called once every 1ms
*/
static void InstanceTimerHandler(uint8_t instanceID)
{
    saveInterruptState_t SaveInterrupt;
    uint32_t AtomicTime;
    uint32_t AtomicRxCount;
    
    /* our time +1ms */
    /* and get RxCount and Timer in one atomic operation */
    SaveInterrupt = OSSaveInterruptState();
    OSDisableInterrupts();

    ++Modbus[instanceID].Time;
    AtomicTime = Modbus[instanceID].Time;
    AtomicRxCount = Modbus[instanceID].RxCount;

    OSRestoreInterruptState( SaveInterrupt);

    /* are we in receiving state? */
    if( Modbus[instanceID].State == M_STATE_RX)
    {
        /* got bytes received and have modbus frame end timeout? */
        if( (AtomicRxCount >= 1u) && (AtomicTime >= Modbus[instanceID].SyncTimeout))
        {
            /* got minimum frame length? */
            if( AtomicRxCount >= 8u)
            {
                
                /* is it my slave number? */
                /* must check it here, fast, because if this message is not for us we must quickly be ready to receive the next */
                if(
				   (Modbus[instanceID].aBuf[0] == Modbus[instanceID].SlaveNr)
					|| (Modbus[instanceID].aBuf[0] == 0u)
				)
                {
                    log_print_string("my slave number");
                    /* go process message */
                    /* reset timer for response delay */
                    Modbus[instanceID].Time = 0u;
					
					if (Modbus[instanceID].RxCount == 0u)
					{
						assert(false);
					}
                    Modbus[instanceID].State = M_STATE_PROCESS;
                }
                else
                {
                    /* not for me */
#ifdef M_MODBUS_DIAGNOSTICS
                    ++Modbus[instanceID].Diagnostics.NotForMe;
#endif
                    /* flush */
                    Modbus[instanceID].RxCount = 0u;
                }
            }
            else
            {
                /* too short */
#ifdef M_MODBUS_DIAGNOSTICS
                ++Modbus[instanceID].Diagnostics.TooShort;
#endif
                /* flush */
                Modbus[instanceID].RxCount = 0u;
            }
        }
    }
}

/*!
*	The worker thread of this instance
*/
static void InstanceWorker(uint32_t instanceID)
{
    uint16_t MyCrc;
    uint32_t ResponseLen;

    switch( Modbus[instanceID].State)
    {
    case M_STATE_RX:
        /* nothing to do here */
        break;

    case M_STATE_PROCESS:
        /* received a message */
        /* check CRC */
        CalcCrc( Modbus[instanceID].aBuf, (uint16_t)(Modbus[instanceID].RxCount - 2u), &MyCrc);
        if(
		   (M_HIGHBYTE(MyCrc) == Modbus[instanceID].aBuf[Modbus[instanceID].RxCount - 2u])
			&& (M_LOWBYTE(MyCrc) == Modbus[instanceID].aBuf[Modbus[instanceID].RxCount - 1u])
			) /* parasoft-suppress MISRA2004-12_8 "Use library code that is tested thoroughly" */
        {
            /* process received message, generate response */
            /*  pass buffer but skip slaveaddress */
            /*  maximum response size minus slaveaddress minus CRC */
            /*  options not used */
            ResponseLen = M_BUFLEN - 1u - 2u;
            (void)ModbusRtuSlaveIntrL6_Fif_ProcessRequest(&Modbus[instanceID].aBuf[1u], &ResponseLen, 0u);
            /* now the response is in the buffer */
			
			if (Modbus[instanceID].aBuf[0] == 0u)
			{
				/* don't send response because of broadcast */
				
				/* back to receive */
				Modbus[instanceID].RxCount = 0u;
				Modbus[instanceID].State = M_STATE_RX;
			}
			else
			{
				/* include the slave address */
				++ResponseLen;
				
				/* generate CRC */
				CalcCrc( Modbus[instanceID].aBuf, ResponseLen, &MyCrc);
				
				/* append CRC to response */
				Modbus[instanceID].aBuf[ResponseLen] = M_HIGHBYTE(MyCrc); /* parasoft-suppress MISRA2004-12_8 "Use library code that is tested thoroughly" */
				Modbus[instanceID].aBuf[ResponseLen + 1u] = M_LOWBYTE(MyCrc); /* parasoft-suppress MISRA2004-12_8 "Use library code that is tested thoroughly" */
				
				/* prepare to transmit response */
				/* number of bytes in TxSize */
				/* this implementation is limited to 255 bytes max while the standard says 'must not exceed 256 bytes' */
				Modbus[instanceID].TxSize = (uint8_t)(ResponseLen + 2u);
				Modbus[instanceID].State = M_STATE_DELAY;
			}
        }
        else
        {
            /* invalid CRC */
#ifdef M_MODBUS_DIAGNOSTICS
            ++Modbus[instanceID].Diagnostics.InvalidCrc;
#endif
            /* flush */
            Modbus[instanceID].RxCount = 0u;
            Modbus[instanceID].State = M_STATE_RX;
        }
        break;

    case M_STATE_DELAY:
        /* delay before sending response */
        if( Modbus[instanceID].Time >= Modbus[instanceID].ResponseDelay)
        {
            /* first set TxCount and State, then call ModbusIntrInitiateTx(),
            *  because ModbusIntrInitiateTx() might generate an ModbusIntrTxHandler() interrupt before it returns!
            */
            uint16_t txCount = Modbus[instanceID].TxSize;
            Modbus[instanceID].TxCount = 0u;
            Modbus[instanceID].State = M_STATE_TX;
            ModbusRtuSlaveIntrL6_Fif_Transmit(Modbus[instanceID].Port, Modbus[instanceID].aBuf, txCount);
        }
        break;

    case M_STATE_TX:
        /* nothing to do here */
        break;

    default:
        /* invalid state? restart */
        /* reset RxCount and Timer in one atomic operation */
        OSDisableInterrupts();
        Modbus[instanceID].RxCount = 0u;
        Modbus[instanceID].Time = 0u;
        OSEnableInterrupts();
        Modbus[instanceID].State = M_STATE_RX;
        break;
    }
}
