
#include "../drivers/sd.h"

#include "../debug.h"
#include "lib/littlefs/lfs.h"

#include "gfx.h"

#define SD_DMA_TIMEOUT					150

int sd_card_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	uint8_t ret;
	uint8_t cnt = 0;
	uint32_t addr = block * c->block_size + off;
	size /= c->block_size;

	do
	{
	    ret = HAL_SD_ReadBlocks_DMA(&hsd1, (uint8_t *)buffer, addr, size);
		cnt++;
		if (cnt > 10)
		{
	  		ERR("Read fail %08lX %u %u", addr, size, cnt);
	  		return -1;
		}
	}
	while (ret != HAL_OK);

	if (cnt > 1)
	{
		WARN("Read problem %08lX %u %u", addr, size, cnt);
	}

    uint32_t start = HAL_GetTick();
    while (hsd1.State == HAL_SD_STATE_BUSY)
    {
        if (HAL_GetTick() - start > SD_DMA_TIMEOUT)
        {
            ERR("Read timeout %08lX %u", addr, size);
            return -2;
        }
    };


	return 0;
}

int sd_card_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint8_t ret;
    uint8_t cnt = 0;
    uint32_t addr = block * c->block_size + off;
    size /= c->block_size;

    do
    {
        ret = HAL_SD_WriteBlocks_DMA(&hsd1, (uint8_t*) buffer, addr, size);
        cnt++;
        if (cnt > 10)
        {
            ERR("Write fail %08lX %u %u", addr, size, cnt);
            return -1;
        }
    }
    while (ret != HAL_OK);

    if (cnt > 1)
    {
        WARN("Write problem %08lX %u %u", addr, size, cnt);
    }

    uint32_t start = HAL_GetTick();
    while (hsd1.State == HAL_SD_STATE_BUSY)
    {
        if (HAL_GetTick() - start > SD_DMA_TIMEOUT)
        {
            ERR("Write timeout %08lX %u %u", addr, size, cnt);
            return -2;
        }
    };



    return 0;
}

int sd_card_erase(const struct lfs_config *c, lfs_block_t block)
{
	uint32_t addr_start = block * c->block_size;
	uint32_t addr_end = (block + 1) * c->block_size;

	return (HAL_SD_Erase(&hsd1, addr_start, addr_end) == HAL_OK) ? 0 : -1;
}

int sd_card_sync(const struct lfs_config *c)
{
	return 0;
}

struct lfs_config cfg;
lfs_t lfs;

#define SD_DEFAULT_BLOCK_SIZE 512

void sd_init()
{
    MX_SDMMC1_SD_Init();

    HAL_SD_CardInfoTypeDef CardInfo;
    HAL_SD_GetCardInfo(&hsd1, &CardInfo);

    // block device operations
    cfg.read  = sd_card_read;
    cfg.prog  = sd_card_prog;
    cfg.erase = sd_card_erase;
    cfg.sync  = sd_card_sync;

    // block device configuration
    cfg.read_size = SD_DEFAULT_BLOCK_SIZE;
    cfg.prog_size = SD_DEFAULT_BLOCK_SIZE;
    cfg.cache_size = SD_DEFAULT_BLOCK_SIZE;
    cfg.block_size = CardInfo.BlockSize;
    cfg.block_count = CardInfo.BlockNbr;

    cfg.block_cycles = 500;

    cfg.lookahead_size = 64;
}

void sd_format()
{
	gfx_draw_status(GFX_STATUS_UPDATE, "Formating SD");
	gfx_draw_progress(0);

	lfs_format(&lfs, &cfg);
}

bool sd_mount()
{
	INFO("Mounting SD");

    int8_t err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err)
    {
        lfs_format(&lfs, &cfg);
        err = lfs_mount(&lfs, &cfg);
    }

	if (err)
	{
		DBG(" Error mounting SD = %u", err);
		return false;
	}

	return true;
}

void sd_unmount()
{
	INFO("Unmounting SD");
	lfs_unmount(&lfs);
}


