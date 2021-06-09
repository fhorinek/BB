
#include "../drivers/sd.h"

#include "../debug.h"
#include "fatfs.h"

#include "gfx.h"

#define DISK_NAME	"Strato"

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


