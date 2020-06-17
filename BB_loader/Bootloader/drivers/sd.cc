
#include "../drivers/sd.h"

#include "../debug.h"
#include "fatfs.h"

void sd_init()
{
	GpioSetDirection(CDMMC1_SW_EN, OUTPUT);
	GpioWrite(CDMMC1_SW_EN, HIGH);

	INFO("Mounting SD");
	uint8_t res =  f_mount(&SDFatFS, SDPath, 1);
	if (res != FR_OK)
	{
		DBG(" Error mounting SD = %u", res);
	}
}

void sd_deinit()
{
//	GpioSetDirection(CDMMC1_SW_EN, OUTPUT);
//	GpioWrite(CDMMC1_SW_EN, HIGH);

}
