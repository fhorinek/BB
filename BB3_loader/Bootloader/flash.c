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
#include "lib/esp-flasher/flasher.h"

extern CRC_HandleTypeDef hcrc;

bool flash_loop()
{
	FIL update_file;
	bool firmware_updated = false;

	//Flash new FW
	uint8_t res = f_open(&update_file, UPDATE_FILE, FA_READ);
	if (res == FR_OK)
	{
		INFO("Update file found on sd card!");
		file_header_t hdr;

		UINT br;

		f_read(&update_file, &hdr, sizeof(hdr), &br);

		INFO("file build number %lX", hdr.build_number);
		INFO("device build number %lX", nvm->app.build_number);

		if (hdr.build_number != nvm->app.build_number || 1)
		{
			gfx_draw_status(GFX_STATUS_UPDATE, "Checking the file");
			gfx_draw_progress(0);

			flasher_ret_t ret = check_update_file(&update_file);

			if (ret != flasher_ok)
			{
				gfx_draw_status(GFX_STATUS_ERROR, "Update file not valid");
				button_confirm(BT3);
			}
			else
			{
			    if (file_exists(SKIP_STM_FILE))
			    {
			        INFO("Skipping STM programming");
			        firmware_updated = true;
			    }
			    else
			    {
                    Bootloader_Init();

                    nvm_data_t nvm_temp;
                    memcpy(&nvm_temp, (uint8_t *)NVM_ADDR, sizeof(nvm_data_t));

                    gfx_draw_status(GFX_STATUS_UPDATE, "Erasing STM");
                    gfx_draw_progress(0);

                    Bootloader_Erase();

                    gfx_draw_status(GFX_STATUS_UPDATE, "STM memory");
                    gfx_draw_progress(0);

                    Bootloader_FlashBegin(APP_ADDRESS);

                    //skip app header
                    f_lseek(&update_file, sizeof(file_header_t));

                    chunk_header_t chunk;
                    f_read(&update_file, &chunk, sizeof(chunk_header_t), &br);

                    bool write_error = false;

                    uint32_t pos = 0;
                    uint8_t buff[WORK_BUFFER_SIZE];

                    while (chunk.size > pos)
                    {
                        uint32_t to_read = chunk.size - pos;
                        if (to_read > WORK_BUFFER_SIZE)
                            to_read = WORK_BUFFER_SIZE;

                        f_read(&update_file, buff, to_read, &br);

                        pos += br;

                        for (uint16_t j = 0; j < br; j += 16)
                        {
                            if (Bootloader_FlashNext((uint32_t *)(((uint8_t *)buff) + j)) != BL_OK)
                            {
                                write_error = true;
                                break;
                            }
                        }
                        gfx_draw_progress(f_tell(&update_file) / (float)f_size(&update_file));
                    }

                    if (write_error)
                    {
                        gfx_draw_status(GFX_STATUS_ERROR, "Writing STM failed!");
                        button_confirm(BT3);
                    }
                    else
                    {
                        //store fw info
                        Bootloader_FlashBegin(NVM_ADDR);

                        nvm_temp.app.size = chunk.size;
                        nvm_temp.app.crc = chunk.crc;
                        nvm_temp.app.build_number = hdr.build_number;

                        for (uint32_t i = 0; i < sizeof(nvm_data_t); i += 16)
                        {
                            Bootloader_FlashNext((uint32_t *)(((uint8_t *)&nvm_temp) + i));
                        }

                        firmware_updated = true;
                    }

                    Bootloader_FlashEnd();
			    }

		        if (file_exists(SKIP_ESP_FILE))
		        {
		            INFO("Skipping ESP programming");
		        }
		        else
		        {
		            if (esp_flash_write_file(&update_file) != flasher_ok)
		            {
		                gfx_draw_status(GFX_STATUS_ERROR, "Writing ESP failed!");

		                button_confirm(BT3);

		                firmware_updated = false;
		            }
		        }
			}
		}


		f_close(&update_file);

		if (!file_exists(KEEP_FW_FILE))
		    f_unlink(UPDATE_FILE);
	}

	return firmware_updated;
}

bool flash_verify()
{
	return (Bootloader_VerifyChecksum() == BL_OK);
}
