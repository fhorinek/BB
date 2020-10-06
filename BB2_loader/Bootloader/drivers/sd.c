
#include "../drivers/sd.h"

#include "../debug.h"
#include "fatfs.h"



bool sd_mount()
{
	INFO("Mounting SD");
	uint8_t res =  f_mount(&SDFatFS, SDPath, 1);
	if (res != FR_OK)
	{
		DBG(" Error mounting SD = %u", res);
		return false;
	}
	return true;
}

void sd_unmount()
{
	INFO("Unmounting SD");
	uint8_t res =  f_mount(NULL, SDPath, 1);
	if (res != FR_OK)
	{
		DBG(" Error unmounting SD = %u", res);
	}
}


