
#include "sd.h"
#include "../debug.h"
#include "fatfs.h"

void sd_init()
{
	INFO("Mounting SD");
	uint8_t res =  f_mount(&SDFatFS, SDPath, true);
	if (res != FR_OK)
	{
		DBG(" Error mounting SD = %u", res);
	}
}

void sd_deinit()
{
	f_mount(NULL, SDPath, true);
}
