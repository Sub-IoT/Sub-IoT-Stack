/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/Modules/Common/emmacro.h $
* $LastChangedRevision: 3 $
*
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
* $LastChangedBy: geert $
*
* \par Company:
*	E.D.&A.\n
* 
* \par Description:
*	Module containing popular macro's\n
*
* \par History:
*	2015-11-30			Created\n
*/

#ifndef EMMACRO_H__
#define EMMACRO_H__

/*-- Compiler settings --*/

#ifdef __IAR_SYSTEMS_ICC__
#endif /* __IAR_SYSTEMS_ICC__ */

/*-- Includes --*/

/*-- Definitions --*/


/* Value for specifying infinite timeout */
#define M_INFINITE                     0xFFFFFFFFUL

/* Define keyword to store data in flash only */
#define M_ROMTYPE					   const

/* Standard function return codes */

enum ERROR_IDS
{
    E_ERR_NOERROR = 0,                 /*!< GUID: - No error */
    E_ERR_NOTREADY = 1,                /*!< GUID: - Not ready, retry later */
    E_ERR_NOLEFT = 2,                  /*!< GUID: - No items left */
    E_ERR_NOSPACE = 3,                 /*!< GUID: - Not enough room available */
    E_ERR_IO = 4,                      /*!< GUID: - I/O operation failed */
    E_ERR_NAK = 5,                     /*!< GUID: - Received negative acknowledgement from partner */
    E_ERR_INVALID = 6,                 /*!< GUID: - Operation or action was invalid (at the moment) */
    E_ERR_INVARG = 7,                  /*!< GUID: - Invalid argument */
    E_ERR_NOTSUPPORTED = 8,            /*!< GUID: - Not supported by module */
    E_ERR_TIMEOUT = 9,                 /*!< GUID: - Timeout */
    E_ERR_UNREACHABLE = 10,            /*!< GUID: - Entered code which should never be reached */
    E_ERR_INITIALIZATION = 11,         /*!< GUID: - Initilisation Error */
    E_ERR_CONFIGURATION = 12           /*!< GUID: - Configuration Error */
};

/* Macro helpers */
#define M_MAKECONCAT(a, b)             a ## b /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro to make use of ## more readable and less error-prone" */ /* parasoft-suppress MISRA2004-19_4 "Macro to use ## cannot be defined differently without introducing unwanted behavior" */ /* parasoft-suppress MISRA2012-RULE-20_10 "This macro increases readability, but use should still be avoided wherever possible" */ /* parasoft-suppress MISRA2004-19_10 "Cannot be enclosed in parentheses as behavior would change" */
#define M_CONCAT(a, b)                 M_MAKECONCAT(a, b) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro to make use of ## more readable and less error-prone" */ /* parasoft-suppress MISRA2004-19_10 "Cannot be enclosed in parentheses as behavior would change" */ /* parasoft-suppress MISRA2004-19_4 "Macro cannot be defined differently without introducing unwanted behavior" */
#define M_MAKESTRING(a)                #a /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro to make use of # more readable and less error-prone" */ /* parasoft-suppress MISRA2004-19_4 "Macro cannot be defined differently without introducing unwanted behavior" */ /* parasoft-suppress MISRA2012-RULE-20_10 "This macro increases readability, but use should still be avoided wherever possible" */
#define M_STRING(a)                    M_MAKESTRING(a) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro to make use of # more readable and less error-prone" */ /* parasoft-suppress MISRA2004-19_4 "Macro cannot be defined differently without introducing unwanted behavior" */

/* Array-length */
#define M_CARRAY_LENGTH(a)             (sizeof(a)/sizeof((a)[0])) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro to get size from definition of array directly (higher readability)" */

/* Minimum & Maximum macro's */
#ifndef M_MIN
#define M_MIN(a, b)                    ( ((a) < (b)) ? (a) : (b) ) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */
#endif
#ifndef M_MAX
#define M_MAX(a, b)                    ( ((a) < (b)) ? (b) : (a) ) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */
#endif

/* Absolute value */
#define M_ABS(a)                       (((a) < 0) ? -(a) : (a)) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */

/* Conversion macros */
#define M_TO32BIT(a,b,c,d)             ( (( (uint32_t) (a) ) << 24U) | (( (uint32_t) (b) ) << 16U) | (( (uint32_t) (c) ) << 8U) | ( (uint32_t) (d) ) ) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */
#define M_TO16BIT(a,b)                 ( (uint16_t)(( (uint16_t) (a) ) << 8U) | ( (uint16_t) (b) ) ) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */
#define M_BYTE(a, num)                 ( (uint8_t) ((a) >> ((num) * 8U)) ) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */

#define M_HIGHBYTE(a)                  (M_BYTE((a), 1U)) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */
#define M_LOWBYTE(a)                   (M_BYTE((a), 0U)) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */

/* macro for taking 1 bit out of a word */
#define M_BITOF(a,b)        (((uint32_t)(a)>>(b))&1u) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */

/*
 *  M_ROUND2() and M_ROUND()
 *
 * Description
 * -----------
 * Macros to round numbers
 *
 * M_ROUND2 = rounds a number up to a power of 2
 * M_ROUND  = rounds a number up to any other number
 *
 *  Parameters
 *  ----------
 *  n     = number to be rounded
 *  pow2  = must be a power of two value
 *  r     = any number
 */

#define M_ROUND2(n,pow2) ( ( (n) + (pow2) - 1u) & ~((pow2) - 1u) ) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */

#define M_ROUND(n,r) ( ( ((n) / (r)) + ( ((n) % (r)) ? 1 : 0) ) * (r) ) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */


/* Endian conversion, converts between this machine and network byte order which is big endian */
#if 1
 /* This is for little endian machines (swap) */
 #define M_HTONS(usNumber) ((((usNumber) & (uint16_t)0xFFU) << 8u) | (((usNumber) & (uint16_t)0xFF00U) >> 8u)) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */
 #define M_NTOHS(usNumber) ((((usNumber) & (uint16_t)0xFFU) << 8u) | (((usNumber) & (uint16_t)0xFF00U) >> 8u)) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */

 #define M_HTONS32(ulNumber) ((((ulNumber) & (uint32_t)0xFFU) << 24u) | (((ulNumber) & (uint32_t)0xFF00U) << 8u) | (((ulNumber) & (uint32_t)0xFF0000U) >> 8u) | (((ulNumber) & (uint32_t)0xFF000000U) >> 24u)) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */
 #define M_NTOHS32(ulNumber) ((((ulNumber) & (uint32_t)0xFFU) << 24u) | (((ulNumber) & (uint32_t)0xFF00U) << 8u) | (((ulNumber) & (uint32_t)0xFF0000U) >> 8u) | (((ulNumber) & (uint32_t)0xFF000000U) >> 24u)) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */
#endif
#if 0
 /* This is for big endian machines (nothing to do) */
 #define M_HTONS(usNumber) (usNumber) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */
 #define M_NTOHS(usNumber) (usNumber) /* parasoft-suppress MISRA2012-DIR-4_9 "Function-like macro for standard algorithm to increase readability" */
#endif



/*-- Public types --*/

/*-- Public functions --*/


/*-- Restore Compiler settings --*/

#ifdef __IAR_SYSTEMS_ICC__
#endif /* __IAR_SYSTEMS_ICC__ */

#endif /* !EMMACRO_H__ */
