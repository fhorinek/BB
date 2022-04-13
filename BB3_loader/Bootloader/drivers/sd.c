
#include "../drivers/sd.h"

#include "../debug.h"
#include "lib/littlefs/lfs.h"

#include "gfx.h"

#define SD_DMA_TIMEOUT					150

#undef DBG
#define DBG(...)

int sd_card_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	uint8_t ret;
	uint8_t cnt = 0;
	uint32_t addr = block;// * c->block_size + off;
	size /= c->block_size;

	ASSERT_MSG(off == 0, "Offset is not 0");

	do
	{
	    ret = HAL_SD_ReadBlocks_DMA(&hsd1, (uint8_t *)buffer, addr, size);
        DBG("HAL_SD_ReadBlocks_DMA %p %08X %u, ret = %u", buffer, addr, size, ret);
		cnt++;
		if (cnt > 10)
		{
	  		ERR("Read fail %08lX %u %u, ret = %u", addr, size, cnt, ret);
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
    uint32_t addr = block;// * c->block_size + off;
    size /= c->block_size;

    ASSERT_MSG(off == 0, "Offset is not 0");

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
	uint32_t addr_start = block;// * c->block_size;
	uint32_t addr_end = (block + 1);// * c->block_size;

	return (HAL_SD_Erase(&hsd1, addr_start, addr_end) == HAL_OK) ? 0 : -1;
}

int sd_card_sync(const struct lfs_config *c)
{
	return 0;
}

struct lfs_config lfs_cfg;
lfs_t lfs;

#define SD_DEFAULT_BLOCK_SIZE 512

void sd_init()
{
    MX_SDMMC1_SD_Init();

    HAL_SD_CardInfoTypeDef card_info;
    HAL_SD_GetCardInfo(&hsd1, &card_info);

    // block device operations
    lfs_cfg.read  = sd_card_read;
    lfs_cfg.prog  = sd_card_prog;
    lfs_cfg.erase = sd_card_erase;
    lfs_cfg.sync  = sd_card_sync;

    // block device configuration
    lfs_cfg.read_size = SD_DEFAULT_BLOCK_SIZE;
    lfs_cfg.prog_size = SD_DEFAULT_BLOCK_SIZE;
    lfs_cfg.cache_size = SD_DEFAULT_BLOCK_SIZE;
    lfs_cfg.block_size = card_info.BlockSize;
    lfs_cfg.block_count = card_info.BlockNbr;

    lfs_cfg.block_cycles = 500;

    lfs_cfg.lookahead_size = 64;
}

void sd_format()
{
	gfx_draw_status(GFX_STATUS_UPDATE, "Formating SD");
	gfx_draw_progress(0);

	lfs_format(&lfs, &lfs_cfg);
}

bool sd_mount()
{
	DBG("Mounting SD");

    int8_t err = lfs_mount(&lfs, &lfs_cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err)
    {
        ERR("Error mounting, formating");
        lfs_format(&lfs, &lfs_cfg);
        err = lfs_mount(&lfs, &lfs_cfg);
    }

	if (err)
	{
		ERR(" Error mounting SD = %u", err);
		return false;
	}

	return true;
}

void sd_unmount()
{
	DBG("Unmounting SD");
	lfs_unmount(&lfs);
}


