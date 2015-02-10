#include "hwatomic.h"
#include "em_int.h"

void start_atomic()
{
	INT_Disable();
}

void end_atomic()
{
	INT_Enable();
}
