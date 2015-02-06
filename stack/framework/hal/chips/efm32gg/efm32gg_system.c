#include "hwsystem.h"
#include "em_system.h"
#include "em_emu.h"
#include <assert.h>

void hw_enter_lowpower_mode(uint8_t mode)
{
    switch(mode)
    {
	case 0:
	{
	    EMU_EnterEM1();
	    break;	    
	}
	case 1:
	{
	    EMU_EnterEM2(true);
	    break;
	}
	case 2:
	{
	    EMU_EnterEM3(true);
	    break;
	}
	case 4:
	{
	    EMU_EnterEM4();
	    break;
	}
	default:
	{
	    assert(0);
	}
    }
}

uint64_t hw_get_unique_id()
{
    return SYSTEM_GetUnique();
}
