/* \file
 *
 * This file spefifies the error codes used throughout the framework and the HAL
 * These have been adopted from TinyOS 2.1.0
 **/
#ifndef ERRORS_H
#define ERRORS_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/* \brief All possible error codes
 *
 */
enum {

  /* \brief Operation completed successfully, no errors occurred
   *
   */
  SUCCESS        =  0,
  
  /* \brief Generic condition: backwards compatible
   *
   */
  FAIL           =  1,
  
  /* \brief Parameter passed in was too big, or outside the expected range
   *
   */
  ESIZE          =  2,
  
  /* \brief Operation cancelled by a call.
   *
   */
  ECANCEL        =  3,
  
  /* \brief Subsystem is not active
   *
   */
  EOFF           =  4,
  
  /* \brief The underlying system is busy; retry later
   *
   */
  EBUSY          =  5,
  
  /* \brief An invalid parameter was passed
   *
   */
  EINVAL         =  6,
  
  /* \brief A rare and transient failure: can retry
   *
   */
  ERETRY         =  7,
  
  /* \brief Reservation required before usage
   *
   */
  ERESERVE       =  8,
  
  /* \brief The device state you are requesting is already set
   *
   */
  EALREADY       =  9,
 
  /* \brief Memory required not available
   *
   */
  ENOMEM         = 10,
 
  /* \brief A packet was not acknowledged
   *
   */
  ENOACK         = 11,
 
  /* \brief Last enum value
   *
   */
  ELAST          = 11
};


#ifdef __cplusplus
}
#endif //__cplusplus



#endif
