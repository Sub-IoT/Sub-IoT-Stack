/*!
* \file
* $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/Modules/Common/emtype.h $
* $LastChangedRevision: 3 $
* $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
*
* \par Company:
*	E.D.&A.\n
*
* \par GUID:
*	\n
*
* \par Description:
*	Module containing basic type definitions\n
*/

#ifndef EMTYPE_H__
#define EMTYPE_H__

/*-- Includes --*/

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#include <stdbool.h>
#endif

/*-- Compiler settings --*/

#ifdef __IAR_SYSTEMS_ICC__
#endif /* __IAR_SYSTEMS_ICC__ */

/*-- Public definitions --*/

/*-- Public types --*/

typedef char char_t;                   /*!< GUID: - Default character type as defined by the compiler (should be set to unsigned in project settings) */

typedef uint32_t ustime_t;             /*!< GUID: - Time unit in microseconds */
typedef uint32_t mstime_t;             /*!< GUID: - Time unit in milliseconds */

typedef uint_fast8_t return_t;         /*!< GUID: - Type large enough for all return codes and most optimal on architecture for that size */

typedef uint32_t bitfield_t;           /*!< GUID: - Type to be used when working with bitfields */

typedef uint32_t address_t;            /*!< GUID: - Type used to hold address values (not as pointer) */

typedef bool bool_t;

/*-- Type ranges --*/

/*-- Public functions --*/

/*-- Legacy types --*/


/*-- Restore compiler settings --*/

#ifdef __IAR_SYSTEMS_ICC__
#endif /* __IAR_SYSTEMS_ICC__ */

#endif /* !(EMTYPE_H__) */
