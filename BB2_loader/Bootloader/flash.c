/*
 * flash.cc
 *
 *  Created on: Jun 19, 2020
 *      Author: horinek
 */

#include "flash.h"

#include "gfx.h"

#include "fatfs.h"
#include "lib/stm32-bootloader/bootloader.h"

extern CRC_HandleTypeDef hcrc;

bool flash_loop()
{
	FIL update_file;
	bool firmware_updated = false;

	Bootloader_Init();

	//Flash new FW
	uint8_t res = f_open(&update_file, UPDATE_FILE, FA_READ);
	if (res == FR_OK)
	{
		INFO("Update file found on sd card!");
		btl_header_t hdr;
		uint32_t calc_crc;

		UINT br;

		f_read(&update_file, &hdr, sizeof(hdr), &br);

		INFO("file build number %lX", hdr.build_number);
		INFO("device build number %lX", app->build_number);

		if (hdr.build_number != app->build_number || 1)
		{
			gfx_draw_status(GFX_STATUS_UPDATE, "Checking file");

			//reset crc unit
			__HAL_CRC_DR_RESET(&hcrc);

			//verify update file
			uint32_t buff[1024 * 4];
			uint32_t progress = 0;

			while (1)
			{
				progress++;

				f_read(&update_file, buff, sizeof(buff), &br);
				calc_crc = HAL_CRC_Accumulate(&hcrc, buff, br);

				if (br == 0)
					break;

				gfx_draw_progress((progress * sizeof(buff)) / (float)f_size(&update_file));
			}

			calc_crc ^= 0xFFFFFFFF;

			//if header crc is not valid
			//if header file size is different than real size
			//if file size does not fit to memory
			if (calc_crc != hdr.crc
					|| f_size(&update_file) != hdr.size + sizeof(hdr)
					|| Bootloader_CheckSize(hdr.size) != BL_OK)
			{
				gfx_draw_status(GFX_STATUS_ERROR, "Update file not valid");
				HAL_Delay(MSG_DELAY);
			}
			else
			{
				gfx_draw_status(GFX_STATUS_UPDATE, "Erasing memory");

				Bootloader_Erase();

				gfx_draw_status(GFX_STATUS_UPDATE, "Writing memory");

				Bootloader_FlashBegin(APP_ADDRESS);

				//skip app header
				f_lseek(&update_file, sizeof(btl_header_t));

				progress = 0;
				bool write_error = false;

				while(1)
				{
					f_read(&update_file, &buff, sizeof(buff), &br);

					progress++;

					//end of file, write app header
					if (br == 0)
					{
						uint32_t tmp[sizeof(btl_header_t) / sizeof(uint32_t)];

						Bootloader_FlashBegin(APP_HDR);

						f_lseek(&update_file, 0);

						f_read(&update_file, tmp, sizeof(btl_header_t), &br);
						for (uint8_t i = 0; i < sizeof(btl_header_t) / sizeof(uint32_t); i++)
							Bootloader_FlashNext(tmp[i]);

						break;
					}

					for (uint16_t j = 0; j < br / 4; j++)
					{
						if (Bootloader_FlashNext(buff[j]) != BL_OK)
						{
							write_error = true;
							break;
						}
					}

					if (write_error)
					{
						gfx_draw_status(GFX_STATUS_ERROR, "Writing failed!");
						HAL_Delay(MSG_DELAY);
						break;
					}

					gfx_draw_progress((progress * sizeof(buff)) / (float)f_size(&update_file));
				}

				Bootloader_FlashEnd();
				firmware_updated = true;
			}

		}
		f_close(&update_file);
		f_unlink(UPDATE_FILE);
	}

	return firmware_updated;
}

bool flash_verify()
{
	return (Bootloader_VerifyChecksum() == BL_OK);
}
