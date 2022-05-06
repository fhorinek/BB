
#include "../drivers/sd.h"

#include "../debug.h"


#include "gfx.h"

#define SD_DMA_TIMEOUT					(300 / (1000 / TX_TIMER_TICKS_PER_SECOND))

#define SD_DEFAULT_BLOCK_SIZE 512

static TX_SEMAPHORE sd_semaphore;
static TX_SEMAPHORE sd_dma_semaphore;

uint8_t sd_read_blocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
    uint8_t ret = HAL_OK;

//    INFO("BSP_SD_ReadBlocks_DMA %08X %u", ReadAddr, NumOfBlocks);

    tx_semaphore_get(&sd_semaphore, TX_WAIT_FOREVER);

    uint8_t status = HAL_SD_ReadBlocks_DMA(&hsd1, (uint8_t *)pData, ReadAddr, NumOfBlocks);

    if (status != HAL_OK)
    {
        //WARN("Read error %08lX %u ret = %u", ReadAddr, NumOfBlocks, status);

        tx_semaphore_put(&sd_semaphore);
        ret = HAL_ERROR;
    }
    else
    {
        status = tx_semaphore_get(&sd_dma_semaphore, SD_DMA_TIMEOUT);
        if (status != TX_SUCCESS)
        {
            WARN("Read timeout %08lX %u err = %X", ReadAddr, NumOfBlocks, status);
            MX_SDMMC1_SD_Init();
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

    tx_semaphore_put(&sd_semaphore);
    return ret;
}

uint8_t sd_write_blocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks)
{
    uint8_t ret = HAL_OK;

//    INFO("BSP_SD_WriteBlocks_DMA %08X %u", WriteAddr, NumOfBlocks);

    tx_semaphore_get(&sd_semaphore, TX_WAIT_FOREVER);

    uint8_t status = HAL_SD_WriteBlocks_DMA(&hsd1, (uint8_t *)pData, WriteAddr, NumOfBlocks);

    if (status != HAL_OK)
    {
        //WARN("Write error %08lX %u ret = %u", WriteAddr, NumOfBlocks, status);

        tx_semaphore_put(&sd_semaphore);
        ret = HAL_ERROR;
    }
    else
    {
        status = tx_semaphore_get(&sd_dma_semaphore, SD_DMA_TIMEOUT);
        if (status != TX_SUCCESS)
        {
            WARN("Write timeout %08lX %u err = %X", WriteAddr, NumOfBlocks, status);
            MX_SDMMC1_SD_Init();
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

    tx_semaphore_put(&sd_semaphore);
    return ret;
}

void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
    tx_semaphore_put(&sd_dma_semaphore);
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
    tx_semaphore_put(&sd_dma_semaphore);
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
    tx_semaphore_put(&sd_dma_semaphore);
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
    ERR("HAL_SD: %08X", hsd->ErrorCode);
    tx_semaphore_put(&sd_dma_semaphore);
//  Error_Handler();
}



void sd_init()
{
    tx_semaphore_create(&sd_semaphore, "SD semaphore", 0);
    tx_semaphore_create(&sd_dma_semaphore, "SD dma semaphore", 0);

    MX_SDMMC1_SD_Init();

    HAL_SD_CardInfoTypeDef card_info;
    HAL_SD_GetCardInfo(&hsd1, &card_info);

    tx_semaphore_put(&sd_semaphore);

    red_init();
 }

void sd_format()
{
	gfx_draw_status(GFX_STATUS_UPDATE, "Formating SD");
	gfx_draw_progress(0);

	int32_t err = red_format("");
	if (err != 0)
	{
	    ERR("red_format = %d", red_errno);
	}
}

bool sd_mount()
{
	DBG("Mounting SD");




    int32_t err = red_mount("");

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err != 0)
    {
        ERR("Error mounting, formating");
        sd_format();
        err = red_mount("");
    }

	if (err)
	{
		ERR(" Error mounting SD = %d", err);
		return false;
	}

	return true;
}

void sd_unmount()
{
	DBG("Unmounting SD");
	red_umount("");
}


