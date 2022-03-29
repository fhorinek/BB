#include "sd_diskio.h"
#include "ff.h"

#define SD_DEFAULT_BLOCK_SIZE 512

#define SD_TIMEOUT          (1 * 1000)

static volatile DSTATUS Stat = STA_NOINIT;

static DSTATUS SD_FailSafe_CheckStatus(BYTE lun);
DSTATUS SD_FailSafe_initialize(BYTE);
DSTATUS SD_FailSafe_status(BYTE);
DRESULT SD_FailSafe_read(BYTE, BYTE*, DWORD, UINT);
DRESULT SD_FailSafe_write(BYTE, const BYTE*, DWORD, UINT);
DRESULT SD_FailSafe_ioctl(BYTE, BYTE, void*);

const Diskio_drvTypeDef SD_FailSafe_Driver =
        {
                SD_FailSafe_initialize,
                SD_FailSafe_status,
                SD_FailSafe_read,
                SD_FailSafe_write,
                SD_FailSafe_ioctl,
        };



static DSTATUS SD_FailSafe_CheckStatus(BYTE lun)
{
    Stat = STA_NOINIT;

    if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
    {
        Stat &= ~STA_NOINIT;
    }

    return Stat;
}

DSTATUS SD_FailSafe_initialize(BYTE lun)
{
    Stat = STA_NOINIT;

    if (BSP_SD_Init() == MSD_OK)
    {
        Stat = SD_FailSafe_CheckStatus(lun);
    }

    return Stat;
}

DSTATUS SD_FailSafe_status(BYTE lun)
{
    return SD_FailSafe_CheckStatus(lun);
}

DRESULT SD_FailSafe_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
    DRESULT res;

    for (uint8_t i = 0; i < 10; i++)
    {
//        FAULT("HAL_SD_ReadBlocks, buff = %X %u %u", buff, sector, count);
        res = HAL_SD_ReadBlocks(&hsd1, buff, (uint32_t) (sector), count, SD_TIMEOUT);
        if (res == RES_OK)
            break;

        FAULT("HAL_SD_ReadBlocks, res = %u (%u)", res, i);
    }
    return res;
}

DRESULT SD_FailSafe_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
    DRESULT res;

    for (uint8_t i = 0; i < 10; i++)
    {
//        FAULT("HAL_SD_WriteBlocks, buff = %X %u %u", buff, sector, count);
        res = HAL_SD_WriteBlocks(&hsd1, (uint8_t *) buff, (uint32_t) (sector), count, SD_TIMEOUT);
        if (res == RES_OK)
        {
            //wait to finish
            while(BSP_SD_GetCardState() != MSD_OK);
            break;
        }

        FAULT("HAL_SD_WriteBlocks, res = %u (%u)", res, i);
    }
    return res;
}

DRESULT SD_FailSafe_ioctl(BYTE lun, BYTE cmd, void *buff)
{
    DRESULT res = RES_ERROR;
    BSP_SD_CardInfo CardInfo;

    if (Stat & STA_NOINIT)
        return RES_NOTRDY;

    switch (cmd)
    {
        /* Make sure that no pending write process */
        case CTRL_SYNC:
            res = RES_OK;
        break;

            /* Get number of sectors on the disk (DWORD) */
        case GET_SECTOR_COUNT:
            BSP_SD_GetCardInfo(&CardInfo);
            *(DWORD*) buff = CardInfo.LogBlockNbr;
            res = RES_OK;
        break;

            /* Get R/W sector size (WORD) */
        case GET_SECTOR_SIZE:
            BSP_SD_GetCardInfo(&CardInfo);
            *(WORD*) buff = CardInfo.LogBlockSize;
            res = RES_OK;
        break;

            /* Get erase block size in unit of sector (DWORD) */
        case GET_BLOCK_SIZE:
            BSP_SD_GetCardInfo(&CardInfo);
            *(DWORD*) buff = CardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
            res = RES_OK;
        break;

        default:
            res = RES_PARERR;
    }

    return res;
}

extern Disk_drvTypeDef disk;
static bool sd_failsafe_init = false;

void SD_FailSafe_init()
{
    char path[16];

    if (sd_failsafe_init)
        return;

    //relink callbacks to non DMA
    HAL_SD_Abort_IT(&hsd1);
    HAL_SD_DeInit(&hsd1);

    disk.nbr = 0;
    FATFS_LinkDriver(&SD_FailSafe_Driver, path);

    uint8_t res = f_mount(&SDFatFS, path, true);

    sd_failsafe_init = true;
}

//override the cube default function, so we can use filesystem without RTOS
int ff_req_grant (  /* 1:Got a grant to access the volume, 0:Could not get a grant */
    _SYNC_t sobj    /* Sync object to wait */
)
{
    if (sd_failsafe_init)
        return 1;

    return osSemaphoreAcquire(sobj, _FS_TIMEOUT) == osOK;
}


