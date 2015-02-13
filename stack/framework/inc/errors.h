/* \file
 *
 * This file spefifies the error codes used throughout the framework and the HAL
 * These have been adopted from TinyOS 2.1.0, in combination with the ones defined in <errno.h>
 **/
#ifndef ERRORS_H
#define ERRORS_H

#include "types.h"
#include <errno.h>


/* \brief Operation completed successfully, no errors occurred
 *
 */
#define SUCCESS 	0
  
/* \brief Generic failure condition
 *
 */
#define FAIL		5001
  
/* \brief Parameter passed in was too big, or outside the expected range
 *
 */
#define ESIZE		5002
  
/* \brief Operation cancelled by a call.
 *
 */
#define ECANCEL		5003
  
/* \brief Subsystem is not active
 *
 */
#define EOFF		5004
  
/* \brief The underlying system is busy; retry later
 *
 */
//#define   EBUSY	(already defined in errno.h)
  
/* \brief An invalid parameter was passed
 *
 */
//#define EINVAL	(already defined in errno.h)
  
/* \brief A rare and transient failure: can retry
 *
 */
#define ERETRY		5005
  
/* \brief Reservation required before usage
 *
 */
#define ERESERVE	5006
  
/* \brief The device state you are requesting is already set
 *
 */
//#define EALREADY	(already defined in errno.h)
 
/* \brief Memory required not available
 *
 */
//#define ENOMEM	(already defined in errno.h)
 
/* \brief A packet was not acknowledged
 *
 */
#define ENOACK		5007



#endif
