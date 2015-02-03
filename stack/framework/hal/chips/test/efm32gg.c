#include "efm32gg.h"
#if defined(DEBUG) && defined (DEBUG_EFM) && defined(EFM32GG990F1024)
char const* config="Debug defined, Debug EFM defined, EFM spec defined";
#elif defined(EFM32GG990F1024)
char const* config="EFM Spec defined";
#elif defined(DEBUG) && defined (DEBUG_EFM)
char const* config="Debug defined, Debug EFM defined";
#else
char const* config="No Configuration defined";
#endif

