
#include "../drivers/sd.h"

#include "../debug.h"
#include "fatfs.h"

#include "gfx.h"

#define DISK_NAME	"Strato"

void sd_format()
{
	BYTE work[_MAX_SS];

	gfx_draw_status(GFX_STATUS_UPDATE, "Formating SD");
	gfx_draw_progress(0);

	uint8_t res = f_mkfs(SDPath, FM_FAT32, 0, work, sizeof(work));
	DBG(" f_mkfs = %u", res);
}

void sd_set_disk_label()
{
	//set disk name to Strato
	char label[16];
	DWORD len;
	f_getlabel(SDPath, label, &len);
	len = min(sizeof(label) - 1, len);
	label[len] = 0;
	if (strcmp(DISK_NAME, label) != 0)
	{
		f_setlabel(DISK_NAME);
	}
}

bool sd_mount()
{
	INFO("Mounting SD");

	uint8_t res = f_mount(&SDFatFS, SDPath, 1);
	if (res == FR_NO_FILESYSTEM)
	{
		sd_format();

		res =  f_mount(&SDFatFS, SDPath, 1);
	}

	if (res != FR_OK)
	{
		DBG(" Error mounting SD = %u", res);
		return false;
	}

	sd_set_disk_label();

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


