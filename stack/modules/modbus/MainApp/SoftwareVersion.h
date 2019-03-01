/*!
 *  \file
 *  $HeadURL: http://visualsvn.edna.local/PD77/PD-77SENSE1/MODBUS_EXAMPLE/MainApp/SoftwareVersion.h $
 *  $LastChangedRevision: 3 $
 *  
 *  $Date: 2019-01-07 15:33:09 +0100 (ma, 07 jan 2019) $
 *  $LastChangedBy: geert $
 *  
 *  \par Company:
 *      E.D.&A.\n
 *
 *  \par Description:
 *      File that holds the software version\n
 */

#ifndef SOFTWARE_VERSION_H__
#define SOFTWARE_VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Major 0..5 */
#define M_VERSION_MAJOR				(2u)

/* Minor 0..99 */
#define M_VERSION_MINOR				(0u)

/* Beta 0..99 */
#define M_VERSION_REVISION			(1u)

#define M_DATA_LEVEL				(1u)

#define M_VARIANT					(1u)

#define M_SOFTWAREVERSION			((M_VERSION_MAJOR*10000u) + (M_VERSION_MINOR*100u) + (M_VERSION_REVISION))


#ifdef __cplusplus
}
#endif

#endif /* !SOFTWARE_VERSION_H__ */





