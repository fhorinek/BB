
#include "../drivers/sd.h"

#include "../debug.h"
#include "lib/littlefs/lfs.h"

#include "gfx.h"

#define SD_DMA_TIMEOUT					300

#define SD_DEFAULT_BLOCK_SIZE 512
#define LFS_BLOCK_SIZE   64
#define BUFF_SIZE   (SD_DEFAULT_BLOCK_SIZE * LFS_BLOCK_SIZE)
#define BLOCK_SIZE  BUFF_SIZE
#define LA_SIZE 8192

#undef DBG
#define DBG(...)
//#undef INFO
//#define INFO(...)

bool sd_wait()
{
    uint32_t start = HAL_GetTick();
    while (hsd1.State == HAL_SD_STATE_BUSY)
    {
        if (HAL_GetTick() - start > SD_DMA_TIMEOUT)
        {
            ERR("Wait timeout");
            return false;
        }
    }

    return true;
}

int sd_card_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	uint8_t ret;
	uint8_t cnt = 0;
	uint32_t addr = (block * LFS_BLOCK_SIZE)  + (off / SD_DEFAULT_BLOCK_SIZE);
	size /= SD_DEFAULT_BLOCK_SIZE;

//	INFO("Read adr %08lX blk %u off %u siz %u", addr, block, off, size);


	uint8_t bad_ret;
	uint32_t bad_error_code;

	do
	{
	    ret = HAL_SD_ReadBlocks_DMA(&hsd1, (uint8_t *)buffer, addr, size);
        DBG("HAL_SD_ReadBlocks_DMA %p %08X %u, ret = %u", buffer, addr, size, ret);
        if (ret != HAL_OK)
        {
            bad_ret = ret;
            bad_error_code = hsd1.ErrorCode;
        }
		cnt++;
		if (cnt > 10)
		{
	  		ERR("Read fail %08lX %u %u, ret = %u, %X", addr, size, cnt, ret, bad_error_code);
	  		return -1;
		}
	}
	while (ret != HAL_OK);

	if (cnt > 1)
	{
		WARN("Read problem %08lX %u %u %u %X", addr, bad_ret, size, cnt, bad_error_code);
	}

    if (!sd_wait())
    {
        return -2;
    }

	return 0;
}

int sd_card_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    uint8_t ret;
    uint8_t cnt = 0;
    uint32_t addr = (block * LFS_BLOCK_SIZE)  + (off / SD_DEFAULT_BLOCK_SIZE);
    size /= SD_DEFAULT_BLOCK_SIZE;

//    INFO("Write adr %08lX blk %u off %u siz %u", addr, block, off, size);
    uint8_t bad_ret;
    uint32_t bad_error_code;

    do
    {
        ret = HAL_SD_WriteBlocks_DMA(&hsd1, (uint8_t*) buffer, addr, size);
        if (ret != HAL_OK)
        {
            bad_ret = ret;
            bad_error_code = hsd1.ErrorCode;
        }
        cnt++;
        if (cnt > 10)
        {
            ERR("Write fail %08lX %u %u %X", addr, size, cnt, bad_error_code);
            return -1;
        }
    }
    while (ret != HAL_OK);

    if (cnt > 1)
    {
        WARN("Write problem %08lX %u %u %u %X", addr, bad_ret, size, cnt, bad_error_code);
    }

    if (!sd_wait())
    {
        return -2;
    }

    return 0;
}

int sd_card_erase(const struct lfs_config *c, lfs_block_t block)
{
    return 0;

//	uint32_t addr_start = block * LFS_BLOCK_SIZE;
//	uint32_t addr_end = addr_start + LFS_BLOCK_SIZE;
//
//	INFO("Erase %X - %X", addr_start, addr_end);
//
//    if (!sd_wait())
//    {
//        return -2;
//    }
//
//    HAL_StatusTypeDef res = HAL_SD_Erase(&hsd1, addr_start, addr_end);
//
//    if (!sd_wait())
//    {
//        return -2;
//    }
//
//    return (res == HAL_OK) ? 0 : -1;
}

int sd_card_sync(const struct lfs_config *c)
{
    return 0;
}

struct lfs_config lfs_cfg;
lfs_t lfs;



uint8_t __aligned(4) read_buff[BUFF_SIZE];
uint8_t __aligned(4) prog_buff[BUFF_SIZE];
uint8_t __aligned(16) la_buff[LA_SIZE / sizeof(uint8_t)];

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
    lfs_cfg.read_size = BUFF_SIZE;
    lfs_cfg.prog_size = BUFF_SIZE;
    lfs_cfg.cache_size = BUFF_SIZE;
    lfs_cfg.block_size = BUFF_SIZE;
    lfs_cfg.block_count = card_info.BlockNbr / LFS_BLOCK_SIZE;

    lfs_cfg.read_buffer = read_buff;
    lfs_cfg.prog_buffer = prog_buff;

    lfs_cfg.block_cycles = 500;

    lfs_cfg.lookahead_size = LA_SIZE;
    lfs_cfg.lookahead_buffer = la_buff;

    HAL_Delay(100);

//    for (uint32_t i = 0; i < card_info.BlockNbr; i++)
//        sd_card_read(&lfs_cfg, i, 0, buff, 512);
//
//    for (uint16_t i = 0; i < sizeof(buff); i++)
//        buff[i] = i % 255;
//
//    for (uint32_t i = 0; i < card_info.BlockNbr; i+=100)
//    {
//        sd_card_read(&lfs_cfg, i, 0, buff, 512);
//        sd_card_prog(&lfs_cfg, i, 0, buff, 512);
//        if (i % 1000000 == 0)
//        {
//            INFO("%u/%u %0.2f%%", i, card_info.BlockNbr, (i * 100.0) / (float)card_info.BlockNbr);
//        }
//        break;
//    }



//    while(1);
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




    int err = lfs_mount(&lfs, &lfs_cfg);

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
		ERR(" Error mounting SD = %d", err);
		return false;
	}

	return true;
}

void sd_unmount()
{
	DBG("Unmounting SD");
	lfs_unmount(&lfs);
}


