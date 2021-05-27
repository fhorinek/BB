
#include "sd.h"
#include "fatfs.h"

void sd_init()
{
	INFO("Mounting SD");
	uint8_t res =  f_mount(&SDFatFS, SDPath, true);
	if (res != FR_OK)
	{
		DBG(" Error mounting SD = %u", res);

		return;
	}

	f_mkdir(PATH_CONFIG_DIR);
	f_mkdir(PATH_PAGES_DIR);
    f_mkdir(PATH_PILOT_DIR);
    f_mkdir(PATH_PROFILE_DIR);
    f_mkdir(PATH_VARIO_DIR);

    f_mkdir(PATH_SYSTEM_DIR);
    f_mkdir(PATH_FW_DIR);
    if (f_mkdir(PATH_TEMP_DIR) == FR_EXIST)
        clear_dir(PATH_TEMP_DIR);

}

void sd_deinit()
{
	f_mount(NULL, SDPath, true);
}
