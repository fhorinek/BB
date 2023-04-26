/*
 * flash.cc
 *
 *  Created on: Jun 19, 2020
 *      Author: horinek
 */

#include "flash.h"

#include "gfx.h"

#include "lib/stm32-bootloader/bootloader.h"
#include "lib/esp-flasher/flasher.h"

#include "nvm.h"

extern CRC_HandleTypeDef hcrc;

bool flash_loop()
{
	bool firmware_updated = false;
	bool keep_file = false;

	//Flash new FW
//    uint8_t res = f_open(&update_file, UPDATE_FILE, FA_READ);

    int32_t update_file = red_open(UPDATE_FILE, RED_O_RDONLY);
	if (update_file > 0)
	{
		INFO("Update file found on sd card!");
		file_header_t hdr;

		int32_t br;
		int32_t bw;

//		f_read(&update_file, &hdr, sizeof(hdr), &br);
		red_read(update_file, &hdr, sizeof(hdr));

		INFO("file build number %lu.%u.%u", hdr.build_number, hdr.build_testing, hdr.build_release);
		INFO("device build number %lu.%u.%u", nvm->app.build_number, nvm->app.build_testing, nvm->app.build_release);

        gfx_draw_status(GFX_STATUS_UPDATE, "Checking the file");
        gfx_draw_progress(0);

        flasher_ret_t ret = check_update_file(update_file);

        if (ret != flasher_ok)
        {
            gfx_draw_status(GFX_STATUS_ERROR, "Update file not valid");
            button_confirm(BT3);
        }
        else
        {
            gfx_draw_progress(0);

            //Flash STM firmware
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

                Bootloader_Erase();

                gfx_draw_status(GFX_STATUS_UPDATE, "STM memory");
                gfx_draw_progress(0);

                Bootloader_FlashBegin(APP_ADDRESS);

                //skip app header
                red_lseek(update_file, sizeof(file_header_t), RED_SEEK_SET);

                chunk_header_t chunk;
                for (;;)
                {
                    ASSERT(red_read(update_file, &chunk, sizeof(chunk)) >= 0);

                    if (chunk.addr == CHUNK_STM_ADDR)
                        break;

                    //skip chunk
                    red_lseek(update_file, flasher_aligned(chunk.size), RED_SEEK_CUR);
                }

                bool write_error = false;

                uint32_t pos = 0;
                uint8_t buff[WORK_BUFFER_SIZE];

                while (chunk.size > pos)
                {
                    uint32_t to_read = chunk.size - pos;
                    if (to_read > WORK_BUFFER_SIZE)
                        to_read = WORK_BUFFER_SIZE;

                    br = red_read(update_file, buff, to_read);

                    pos += br;

                    for (uint16_t j = 0; j < br; j += 16)
                    {
                        if (Bootloader_FlashNext((uint32_t *)(((uint8_t *)buff) + j)) != BL_OK)
                        {
                            write_error = true;
                            break;
                        }
                    }
                    gfx_draw_progress(red_lseek(update_file, 0, RED_SEEK_CUR) / (float)file_size(update_file));
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
                    nvm_temp.app.build_testing = hdr.build_testing;
                    nvm_temp.app.build_release = hdr.build_release;

                    for (uint32_t i = 0; i < sizeof(nvm_data_t); i += 16)
                    {
                        Bootloader_FlashNext((uint32_t *)(((uint8_t *)&nvm_temp) + i));
                    }

                    firmware_updated = true;
                }

                Bootloader_FlashEnd();
            }

            //Extract firmware files
            if (file_exists(SKIP_FS_FILE))
            {
                INFO("Skipping file assets");
            }
            else
            {
                char cwd[256];


                clear_dir(ASSET_PATH);

                red_mkdir(SYSTEM_PATH);
                red_mkdir(ASSET_PATH);

                strcpy(cwd, ASSET_PATH);

                gfx_draw_status(GFX_STATUS_UPDATE, "Copying assets");

                //rewind
                red_lseek(update_file, sizeof(file_header_t), RED_SEEK_SET);

                chunk_header_t chunk;
                chunk_header_t last_chunk;

                uint8_t level = 0;

                for (;;)
                {
                    br = red_read(update_file, &chunk, sizeof(chunk));
                    ASSERT(br >= 0);

                    //EOF check
                    if (br == 0)
                        break;

                    if (!(chunk.addr & CHUNK_FS_MASK))
                    {
                        //skip chunk
                        red_lseek(update_file, flasher_aligned(chunk.size), RED_SEEK_CUR);
                        continue;
                    }

                    uint32_t mask = chunk.addr & CHUNK_FS_MASK;
                    uint8_t clevel = (chunk.addr & 0x00FF0000) >> 16;
                    //uint16_t cindex = chunk.addr & 0x0000FFFF;

                    if (level < clevel) //into last created dir
                    {
                        sprintf(cwd + strlen(cwd), "/%s", last_chunk.name);
                        level = clevel;
                    }
                    else while (level > clevel) //out of the current dir
                    {
                        char * last_slash = strrchr(cwd, '/');
                        ASSERT(last_slash != NULL);
                        *last_slash = 0;
                        level--;
                    }

                    char fname[256 + 32];
                    snprintf(fname, sizeof(fname), "%s/%s", cwd, chunk.name);

                    if (mask == CHUNK_DIR_TYPE)
                    {
                        INFO("creating dir %s", fname);
                        red_mkdir(fname);
                        gfx_draw_progress(red_lseek(update_file, 0, RED_SEEK_CUR) / (float)file_size(update_file));
                    }

                    if (mask == CHUNK_FILE_TYPE)
                    {
                        int32_t af;

                        INFO("creating file %s", fname);
//                        if (f_open(&af, fname, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
                        if ((af = red_open(fname, RED_O_CREAT | RED_O_WRONLY)) > 0)
                        {
                            uint32_t pos = 0;
                            uint8_t buff[WORK_BUFFER_SIZE];

                            while (chunk.size > pos)
                            {
                                uint32_t to_read = chunk.size - pos;
                                if (to_read > WORK_BUFFER_SIZE)
                                    to_read = WORK_BUFFER_SIZE;

//                                f_read(&update_file, buff, to_read, &br);
                                br = red_read(update_file, buff, to_read);
//                                gfx_draw_progress(f_tell(&update_file) / (float)f_size(&update_file));
                                gfx_draw_progress(red_lseek(update_file, 0, RED_SEEK_CUR) / (float)file_size(update_file));

                                pos += br;
//                                f_write(&af, buff, br, &bw);
                                bw = red_write(af, buff, br);
                                ASSERT(br == bw);
                            }

                            red_close(af);

                            //if not size is not aligned
                            uint8_t diff = flasher_aligned(chunk.size) - chunk.size;
                            if (diff)
                            {
//                                f_lseek(&update_file, f_tell(&update_file) + diff);
                                red_lseek(update_file, diff, RED_SEEK_CUR);
                            }
                        }
                        else
                        {
                            WARN("Could not create file %s", fname);
//                            f_lseek(&update_file, f_tell(&update_file) + flasher_aligned(chunk.size));
                            red_lseek(update_file, flasher_aligned(chunk.size), RED_SEEK_CUR);
                        }
                    }

                    memcpy(&last_chunk, &chunk, sizeof(chunk));
                }

            }


            //Flash ESP firmware
            if (file_exists(SKIP_ESP_FILE))
            {
                INFO("Skipping ESP programming");
            }
            else
            {
                if (esp_flash_write_file(update_file) != flasher_ok)
                {
                    gfx_draw_status(GFX_STATUS_ERROR, "Writing ESP failed!");

                    button_confirm(BT3);

                    firmware_updated = false;
                    keep_file = true;
                }
            }
        }


        red_close(update_file);

		if (!file_exists(KEEP_FW_FILE) && !keep_file)
		    red_unlink(UPDATE_FILE);
	}

	return firmware_updated;
}

bool flash_verify()
{
	return (Bootloader_VerifyChecksum() == BL_OK);
}
