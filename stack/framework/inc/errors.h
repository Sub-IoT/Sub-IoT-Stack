#ifndef ERRORS_H
#define ERRORS_H

#include "types.h"
typedef uint8_t error_t;

//generic error codes 'borrowed' from tinyos
enum {
  SUCCESS        =  0,
  FAIL           =  1,           // Generic condition: backwards compatible
  ESIZE          =  2,           // Parameter passed in was too big.
  ECANCEL        =  3,           // Operation cancelled by a call.
  EOFF           =  4,           // Subsystem is not active
  EBUSY          =  5,           // The underlying system is busy; retry later
  EINVAL         =  6,           // An invalid parameter was passed
  ERETRY         =  7,           // A rare and transient failure: can retry
  ERESERVE       =  8,           // Reservation required before usage
  EALREADY       =  9,           // The device state you are requesting is already set
  ENOMEM         = 10,           // Memory required not available
  ENOACK         = 11,           // A packet was not acknowledged
  ELAST          = 11            // Last enum value
};




#endif
