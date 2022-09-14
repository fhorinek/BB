
#include <gui/sd_migrate.h>
#include "sd.h"


bool sd_failsafe = false;

osSemaphoreId_t sd_semaphore;
osSemaphoreId_t sd_dma_semaphore;


#define SD_DMA_TIMEOUT       150
#define SD_TIMEOUT           1000

void sd_reinit()
{
    HAL_SD_DeInit(&hsd1);
    osDelay(100);
    MX_SDMMC1_SD_Init();
}

uint8_t sd_read_blocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
    if (sd_failsafe)
    {
        uint8_t status = HAL_SD_ReadBlocks(&hsd1, (uint8_t *)pData, ReadAddr, NumOfBlocks, SD_TIMEOUT);
        FAULT("sd_read_blocks %08X %u = %u", ReadAddr, NumOfBlocks, status);

        return status;
    }

    uint8_t ret = HAL_OK;

//    INFO("sd_read_blocks %08X %u", ReadAddr, NumOfBlocks);

    osSemaphoreAcquire(sd_semaphore, WAIT_INF);

    uint8_t status = HAL_SD_ReadBlocks_DMA(&hsd1, (uint8_t *)pData, ReadAddr, NumOfBlocks);

    if (status != HAL_OK)
    {
        //WARN("Read error %08lX %u ret = %u", ReadAddr, NumOfBlocks, status);

        osSemaphoreRelease(sd_semaphore);
        ret = HAL_ERROR;
    }
    else
    {
        status = osSemaphoreAcquire(sd_dma_semaphore, SD_DMA_TIMEOUT);
        if (status != osOK)
        {
            WARN("Read timeout %08lX %u err = %X", ReadAddr, NumOfBlocks, status);
            sd_reinit();
            ret = HAL_ERROR;
        }
        else
        {
            if (hsd1.ErrorCode != 0)
            {
                WARN("Read dma error %08lX %u err = %X", ReadAddr, NumOfBlocks, hsd1.ErrorCode);
                ret = HAL_ERROR;
            }
        }
    }

    osSemaphoreRelease(sd_semaphore);
    return ret;
}

uint8_t sd_write_blocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{
    static uint8_t err_cnt = 0;

    if (sd_failsafe)
    {
        uint8_t status = HAL_SD_WriteBlocks(&hsd1, (uint8_t *)pData, WriteAddr, NumOfBlocks, SD_TIMEOUT);

        FAULT("sd_write_blocks %08X %u", WriteAddr, NumOfBlocks);
        if (status != HAL_OK)
        {
            FAULT(" error %u %X", status, hsd1.ErrorCode);
        }
        else
        {
            //wait for op to finish
            while(HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER);
        }

        return status;

    }

    uint8_t ret = HAL_OK;

//    INFO("sd_write_blocks %08X %u", WriteAddr, NumOfBlocks);

    osSemaphoreAcquire(sd_semaphore, WAIT_INF);

    uint8_t status = HAL_SD_WriteBlocks_DMA(&hsd1, (uint8_t *)pData, WriteAddr, NumOfBlocks);

    if (status != HAL_OK)
    {
        WARN("Write error %08lX %u ret = %u", WriteAddr, NumOfBlocks, status);

        osSemaphoreRelease(sd_semaphore);
        ret = HAL_ERROR;

        err_cnt++;
        if (err_cnt >= 3)
        {
            sd_reinit();
        }
    }
    else
    {
        err_cnt = 0;

        status = osSemaphoreAcquire(sd_dma_semaphore, SD_DMA_TIMEOUT);
        if (status != osOK)
        {
            WARN("Write timeout %08lX %u err = %X", WriteAddr, NumOfBlocks, status);

            sd_reinit();

            ret = HAL_ERROR;
        }
        else
        {
            if (hsd1.ErrorCode != 0)
            {
                WARN("Write dma error %08lX %u err = %X", WriteAddr, NumOfBlocks, hsd1.ErrorCode);
                ret = HAL_ERROR;
            }
        }
    }

    osSemaphoreRelease(sd_semaphore);
    return ret;
}

void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
    osSemaphoreRelease(sd_dma_semaphore);
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    osSemaphoreRelease(sd_dma_semaphore);
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    osSemaphoreRelease(sd_dma_semaphore);
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
    ERR("HAL_SD: %08X", hsd->ErrorCode);
    osSemaphoreRelease(sd_dma_semaphore);
//  Error_Handler();
}


void sd_init_failsafe()
{
    sd_failsafe = true;

    red_umount("");
    red_uninit();

    MX_SDMMC1_SD_Init();

    red_init();
    int32_t err = red_mount("");
    if (err != 0)
    {
        FAULT("Error failsafe re-mounting, %d", err);
    }
}

void sd_init()
{
    sd_semaphore = osSemaphoreNew(1, 0, NULL);
    sd_dma_semaphore = osSemaphoreNew(1, 0, NULL);

    vQueueAddToRegistry(sd_semaphore, "SD semaphore");
    vQueueAddToRegistry(sd_dma_semaphore, "sd_dma_semaphore");

    MX_SDMMC1_SD_Init();

    osSemaphoreRelease(sd_semaphore);

    red_init();

    int32_t err = red_mount("");

    if (err != 0)
    {
        ERR("Error mounting, %d", err);
        sd_migrate_dialog();
    }

	//create file system structure

	//logs
    red_mkdir(PATH_LOGS_DIR);

    //agl
    red_mkdir(PATH_TOPO_DIR);

    //map
    red_mkdir(PATH_MAP_DIR);

    //config
	red_mkdir(PATH_CONFIG_DIR);

	//config/pilots
    red_mkdir(PATH_PILOT_DIR);
    //config/profiles
    red_mkdir(PATH_PROFILE_DIR);

    //config/pages
	red_mkdir(PATH_PAGES_DIR);

    //config/vario
    red_mkdir(PATH_VARIO_DIR);

    //system
    red_mkdir(PATH_SYSTEM_DIR);
    //system/fw
    red_mkdir(PATH_FW_DIR);
    //system/temp
    if (red_mkdir(PATH_TEMP_DIR) != 0)
        clear_dir(PATH_TEMP_DIR);

    //system/assets
    red_mkdir(PATH_ASSET_DIR);
    //system/assets/defaults
    red_mkdir(PATH_DEFAULTS_DIR);

    //system/cache
    red_mkdir(PATH_CACHE_DIR);
    //system/cache/map
    red_mkdir(PATH_MAP_CACHE_DIR);
    //system/cache/logs
    red_mkdir(PATH_LOG_CACHE_DIR);
}

void sd_deinit()
{
    DBG("Unmounting SD");
    red_umount("");
}
