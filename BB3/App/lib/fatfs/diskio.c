/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "common.h"
#include "drivers/sd.h"

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    if (HAL_SD_GetCardState(&hsd1) == HAL_SD_CARD_TRANSFER)
    {
        return FR_OK;
    }

    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    MX_SDMMC1_SD_Init();
    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	return (sd_read_blocks((uint32_t *)buff, sector, count) == HAL_OK) ? RES_OK : RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    return sd_write_blocks(buff, sector, count) == HAL_OK ? RES_OK : RES_ERROR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	HAL_SD_CardInfoTypeDef CardInfo;

    switch (cmd)
    {
        /* Make sure that no pending write process */
        case CTRL_SYNC:
            res = RES_OK;
        break;

            /* Get number of sectors on the disk (DWORD) */
        case GET_SECTOR_COUNT:
            HAL_SD_GetCardInfo(&hsd1, &CardInfo);
            *(DWORD*) buff = CardInfo.LogBlockNbr;
            res = RES_OK;
        break;

            /* Get R/W sector size (WORD) */
        case GET_SECTOR_SIZE:
            HAL_SD_GetCardInfo(&hsd1, &CardInfo);
            *(WORD*) buff = CardInfo.LogBlockSize;
            res = RES_OK;
        break;

            /* Get erase block size in unit of sector (DWORD) */
        case GET_BLOCK_SIZE:
            HAL_SD_GetCardInfo(&hsd1, &CardInfo);
            *(DWORD*) buff = 1;
            res = RES_OK;
        break;

        default:
            res = RES_PARERR;
    }

    return res;
}

