////////////////////////////////////////////////////////////////////////////////
// system.c
//

#include <jendefs.h>				// JN516x
#include <AppHardwareApi.h>			// JN516x

#include "utils.h"					// ToCoNetUtils

#include "system.h"

PUBLIC void delay_us(uint32 t)
{
	vWait(t * 178 / 100);
}

PUBLIC void zero_memory(void* destination, size_t size)
{
	uint8* p = destination;
	while (size--)
	{
		*p++ = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
