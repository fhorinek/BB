
#include "sd.h"
#include "fatfs.h"


#define SD_DMA_TIMEOUT					150

uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
	uint8_t ret;
	uint8_t cnt = 0;
	do
	{
	    ret = HAL_SD_ReadBlocks_DMA(&hsd1, (uint8_t *)pData, ReadAddr, NumOfBlocks);
		cnt++;
		if (cnt > 10)
		{
	  		ERR("Read fail %08lX %u %u", ReadAddr, NumOfBlocks, cnt);
	  		return MSD_ERROR;
		}
	}
	while (ret != HAL_OK);

	if (cnt > 1)
	{
		WARN("Read problem %08lX %u %u", ReadAddr, NumOfBlocks, cnt);
	}

	return MSD_OK;
}

uint8_t BSP_SD_ReadBlocks_DMA_Wait(uint32_t ReadAddr, uint32_t NumOfBlocks)
{
	  uint32_t start = HAL_GetTick();
	  while (hsd1.State == HAL_SD_STATE_BUSY)
	  {
	  	if (HAL_GetTick() - start > SD_DMA_TIMEOUT)
	  	{
	  		ERR("Read timeout %08lX %u", ReadAddr, NumOfBlocks);
	  		return MSD_ERROR;
	  	}
	  };

	  return MSD_OK;
}

uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
	uint8_t ret;
	uint8_t cnt = 0;
	do
	{
	    ret = HAL_SD_WriteBlocks_DMA(&hsd1, (uint8_t *)pData, ReadAddr, NumOfBlocks);
		cnt++;
		if (cnt > 10)
		{
	  		ERR("Write fail %08lX %u %u", ReadAddr, NumOfBlocks, cnt);
	  		return MSD_ERROR;
		}
	}
	while (ret != HAL_OK);

	if (cnt > 1)
	{
		WARN("Write problem %08lX %u %u", ReadAddr, NumOfBlocks, cnt);
	}

  uint32_t start = HAL_GetTick();
  while (hsd1.State == HAL_SD_STATE_BUSY)
  {
  	if (HAL_GetTick() - start > SD_DMA_TIMEOUT)
  	{
  		ERR("Write timeout %08lX %u %u", ReadAddr, NumOfBlocks, cnt);
  		return MSD_ERROR;
  	}
  };

  return MSD_OK;
}

uint8_t BSP_SD_WriteBlocks_DMA_Wait(uint32_t ReadAddr, uint32_t NumOfBlocks)
{
	  uint32_t start = HAL_GetTick();
	  while (hsd1.State == HAL_SD_STATE_BUSY)
	  {
	  	if (HAL_GetTick() - start > SD_DMA_TIMEOUT)
	  	{
	  		ERR("Write timeout %08lX %u", ReadAddr, NumOfBlocks);
	  		return MSD_ERROR;
	  	}
	  };

	  return MSD_OK;
}


void sd_init()
{
	INFO("Mounting SD");
	uint8_t res =  f_mount(&SDFatFS, SDPath, true);
	if (res != FR_OK)
	{
		DBG(" Error mounting SD = %u", res);

		return;
	}

	//create file system structure

	//logs
    f_mkdir(PATH_LOGS_DIR);

    //agl
    f_mkdir(PATH_TOPO_DIR);

    //map
    f_mkdir(PATH_MAP_DIR);


    //config
	f_mkdir(PATH_CONFIG_DIR);

	//configs are in flash memory default will be available
	//config/pilots
    f_mkdir(PATH_PILOT_DIR);
    //config/profiles
    f_mkdir(PATH_PROFILE_DIR);

    //config/pages
	if (f_mkdir(PATH_PAGES_DIR) == FR_OK)
	{
		f_mkdir(PATH_PAGES_DIR "/default");
		copy_dir(PATH_DEFAULTS_DIR "/pages", PATH_PAGES_DIR "/default");
	}

    //config/vario
    if (f_mkdir(PATH_VARIO_DIR) == FR_OK)
    {
    	copy_dir(PATH_DEFAULTS_DIR "/vario", PATH_VARIO_DIR);
    }

    //system
    f_mkdir(PATH_SYSTEM_DIR);
    //system/fw
    f_mkdir(PATH_FW_DIR);
    //system/temp
    if (f_mkdir(PATH_TEMP_DIR) == FR_EXIST)
        clear_dir(PATH_TEMP_DIR);

    //system/cache
    f_mkdir(PATH_CACHE_DIR);
    //system/cache/map
    f_mkdir(PATH_MAP_CACHE_DIR);



}

void sd_deinit()
{
	f_mount(NULL, SDPath, true);
}
