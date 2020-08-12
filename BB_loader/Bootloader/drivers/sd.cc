
#include "../drivers/sd.h"

#include "../debug.h"
#include "fatfs.h"

bool sd_detect()
{
	return GpioRead(SDMMC1_CDET) == LOW;
}

void sd_init()
{
	INFO("Init SD");
	GpioSetDirection(SDMMC1_SW_EN, OUTPUT);
	GpioWrite(SDMMC1_SW_EN, HIGH);
}

bool sd_mount()
{
	INFO("Mounting SD");
	uint8_t res =  f_mount(&SDFatFS, SDPath, 1);
	if (res != FR_OK)
	{
		return false;
		DBG(" Error mounting SD = %u", res);
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


void sd_deinit()
{
	GpioSetDirection(SDMMC1_SW_EN, OUTPUT);
	GpioWrite(SDMMC1_SW_EN, HIGH);
}
