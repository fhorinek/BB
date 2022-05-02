/*
 * bootloader.c
 *
 *  Created on: Oct 13, 2021
 *      Author: horinek
 */

#include "bootloader.h"

#include "drivers/nvm.h"

typedef struct
{
	uint32_t build_number;
	uint32_t size;
	uint32_t reserved;
	uint32_t crc;
} bootloader_header_t;

bootloader_res_t bootloader_update(char * path)
{
	//TODO: check batery voltage before

	INFO("Checking the bootloader file %s", path);

	if (file_exists(path))
	{
	    int32_t f = red_open(path, RED_O_RDONLY);

		if (f < 0)
		{
			ERR("Unable to open update file");
			red_close(f);
			return bl_file_invalid;
		}

		bootloader_header_t hdr;

		if (red_read(f, &hdr, sizeof(hdr)) != sizeof(hdr))
		{
			ERR("Unexpected end while getting head");
			red_close(f);
			return bl_file_invalid;
		}

        if (nvm->bootloader == hdr.build_number)
        {
            ERR("Already up-to-date");
            red_close(f);
            return bl_same_version;
        }

		if (file_size(f) != hdr.size + sizeof(hdr))
		{
			ERR("Unexpected file size");
			red_close(f);
			return bl_file_invalid;
		}

        //reset crc unit
        __HAL_CRC_DR_RESET(&hcrc);
        uint32_t pos = 0;
        uint32_t crc = 0;

		#define WORK_BUFFER_SIZE    (1024 * 16)
        uint8_t * buff = ps_malloc(WORK_BUFFER_SIZE);
        while (hdr.size > pos)
        {
            uint32_t to_read = hdr.size  - pos;
            if (to_read > WORK_BUFFER_SIZE)
                to_read = WORK_BUFFER_SIZE;

            int32_t br = red_read(f, buff, to_read);

            if (br <= 0)
            {
                ERR("Unexpected eof at %lu", pos);
                red_close(f);
                ps_free(buff);
                return bl_file_invalid;
            }

            crc = HAL_CRC_Accumulate(&hcrc, (uint32_t *)buff, br);
            pos += br;
        }


        crc ^= 0xFFFFFFFF;

        if (crc != hdr.crc)
        {
            ERR("CRC fail");
            red_close(f);
            return bl_file_invalid;
        }

		INFO("File is ok");

	    HAL_FLASH_Unlock();
	    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1);
	    red_lseek(f, sizeof(hdr), RED_SEEK_SET);

		INFO("Erasing bootloader");
	    FLASH_EraseInitTypeDef pEraseInit;
	    pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
	    pEraseInit.Sector = 0;
	    pEraseInit.NbSectors = 32;
	    pEraseInit.Banks = FLASH_BANK_1;

	    uint32_t sector_error;
	    HAL_FLASHEx_Erase(&pEraseInit, &sector_error);

		INFO("Programming bootloader");
	    pos = 0;
        while (hdr.size > pos)
        {
            uint32_t to_read = hdr.size - pos;
            if (to_read > WORK_BUFFER_SIZE)
                to_read = WORK_BUFFER_SIZE;

            int32_t br = red_read(f, buff, to_read);

            ASSERT(br > 0);

            for (uint16_t j = 0; j < br; j += 16)
            {
            	HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, FLASH_BASE + pos + j, (uint32_t)buff + j);
            }

            pos += br;
        }

        nvm_update_bootloader(hdr.build_number);

        INFO("Programming done");
        HAL_FLASH_Lock();
        ps_free(buff);
        red_close(f);
		return bl_update_ok;
	}
	else
	{
		ERR("Unable to locate the file");
		return bl_file_not_found;
	}
}

