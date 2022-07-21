/*
 * sd_format.c
 *
 *  Created on: May 4, 2022
 *      Author: horinek
 */

#include <gui/sd_migrate.h>
#include "gui/tasks/blank.h"
#include "gui/dialog.h"
#include "gui_thread.h"
#include "gui/tasks/page/pages.h"
#include "etc/bootloader.h"

#include "drivers/nvm.h"


#include "lib/fatfs/ff.h"

extern gui_task_t * gui_startup_task;
static osSemaphoreId_t lock;

typedef struct _ram_file_s
{
     struct _ram_file_s * parent;
     struct _ram_file_s * next;
     char * path;
     uint32_t size;
     uint8_t * data;
} ram_file_t;

static void migrate_fail_cb(uint8_t res, void * data)
{
    system_reboot_hard();
}

static bool fatfs_file_exists(char * path)
{
    FILINFO fno;
    return (f_stat(path, &fno) == FR_OK);
}

static FIL * f = NULL;

void fatfs_bl_clean()
{
	if (f != NULL)
	{

	}
}

static bootloader_res_t fatfs_bootloader_update(char * path)
{
	FIL f;
	UINT br;

	INFO("Checking the bootloader file %s", path);

	if (fatfs_file_exists(path))
	{
		if (f_open(&f, path, FA_READ) != FR_OK)
		{
			ERR("Unable to open update file");
			return bl_file_invalid;
		}

		bootloader_header_t hdr;
		f_read(&f, &hdr, sizeof(hdr), &br);

		if (br != sizeof(hdr))
		{
			ERR("Unexpected end while getting head");
			f_close(&f);
			return bl_file_invalid;
		}

		if (f_size(&f) != hdr.size + sizeof(hdr))
		{
			ERR("Unexpected file size");
			f_close(&f);
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

            ASSERT(f_read(&f, buff, to_read, &br) == FR_OK);

            if (br == 0)
            {
                ERR("Unexpected eof at %lu", pos);
                fatfs_bl_clean();
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
            f_close(&f);
            return bl_file_invalid;
        }

		INFO("File is ok");

	    HAL_FLASH_Unlock();
	    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1);
	    f_lseek(&f, sizeof(hdr));

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

            f_read(&f, buff, to_read, &br);

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
        f_close(&f);
		return bl_update_ok;
	}
	else
	{
		ERR("Unable to locate the file");
		return bl_file_not_found;
	}
}


void ram_file_free(ram_file_t * p)
{
    if (p->path != NULL)
        free(p->path);

    if (p->data != NULL)
        ps_free(p->data);

    free(p);
}

ram_file_t * files_to_ram(ram_file_t * prev, char * path)
{
    DIR dir;
    static FILINFO fno;
    static char buff[PATH_LEN];

    ram_file_t * last = prev;

    INFO("Listing %s", path);

    FRESULT res = f_opendir(&dir, path);
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);

            if (res != FR_OK || fno.fname[0] == 0)
                break;

            if (fno.fattrib & AM_DIR)
            {
                uint16_t i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                last = files_to_ram(last, path);
                path[i] = 0;
            }
            else
            {
                snprintf(buff, sizeof(buff), "%s/%s", path, fno.fname);
                INFO("Storing %s", buff);

                FIL f;
                UINT br;

                FRESULT res = f_open(&f, buff, FA_READ);

                if (res == FR_OK)
                {
                    ram_file_t * actual = malloc(sizeof(ram_file_t));
                    if (actual != NULL)
                    {
                        actual->next = NULL;
                        actual->path = malloc(strlen(buff) + 1);
                        actual->size = f_size(&f);

                        if (actual->path != NULL)
                        {
                            strcpy(actual->path, buff);

                            if (actual->size > 0)
                            {
                                actual->data = ps_malloc(actual->size);

                                if (actual->data != NULL)
                                {
                                    res = f_read(&f, actual->data, actual->size, &br);

                                    if (res == FR_OK && br == actual->size)
                                    {
                                        INFO("Readed and stored %lu", actual->size);
                                        last->next = actual;
                                        last = actual;
                                    }
                                    else
                                    {
                                        ERR("Error in read size is %lu, readed %lu, res %u", actual->size, br, res);
                                        ram_file_free(actual);
                                    }
                                }
                                else
                                {
                                    free(actual->path);
                                    free(actual);
                                    ERR("Unable to allocate ram_file->data");
                                }
                            }
                            else
                            {
                                actual->data = NULL;
                                INFO(" Empty, stored");
                                last->next = actual;
                                last = actual;
                            }
                        }
                        else
                        {
                            free(actual);
                            ERR("Unable to allocate ram_file->name");
                        }
                    }
                    else
                    {
                        ERR("Unable to allocate ram_file");
                    }

                    f_close(&f);
                }
            }
        }
        f_closedir(&dir);
    }
    return last;
}


void files_from_ram(ram_file_t * first)
{
    ram_file_t * actual = first;
    while (actual != NULL)
    {
        char dpath[PATH_LEN];
        char * pos = actual->path;
        pos = actual->path;

        for(;;)
        {
            pos = strchr(pos, '/');
            if (pos == NULL)
                break;

            uint16_t npos = pos - actual->path;
            pos++;
            strncpy(dpath, actual->path, npos);
            dpath[npos] = 0;
            if (!file_exists(dpath))
                red_mkdir(dpath);
        }

        INFO("Writing %s", actual->path);
        int32_t f = red_open(actual->path, RED_O_WRONLY | RED_O_CREAT);
        if (f > 0)
        {
            if (actual->size > 0)
            {
                int32_t bw = red_write(f, actual->data, actual->size);
                if (bw != actual->size)
                    ERR("Unable to write %u != %d, err %d", actual->size, bw, red_errno);
            }
            red_close(f);
        }
        else
        {
            ERR("Unable to create, err %d", red_errno);
        }

        ram_file_t * prev = actual;
        actual = actual->next;
        ram_file_free(prev);
    }

}

void sd_migrate_worker(void * param)
{
    (void)param;

    char path[PATH_LEN];

    FATFS * fs = malloc(sizeof(FATFS));
    FRESULT res = f_mount(fs, "", true);

    if (res == FR_OK)
    {
    	bootloader_res_t bl_res = fatfs_bootloader_update(PATH_BL_FW_AUTO);
    	if (bl_res != bl_update_ok)
    	{
            dialog_show("Error", "Unable to update bootloader.\nRe-install the update file.", dialog_confirm, migrate_fail_cb);
            vTaskSuspend(NULL);
    	}

    	dialog_set_text("Bootloader updated");
    	osDelay(1000);

        ram_file_t first;
        first.path = NULL;
        first.data = NULL;
        first.next = NULL;

        dialog_set_text("Storing config");

        if (res == FR_OK)
        {
            strcpy(path, PATH_ASSET_DIR);
            ram_file_t * last = files_to_ram(&first, path);

            strcpy(path, PATH_CONFIG_DIR);
            last = files_to_ram(last, path);

            strcpy(path, PATH_LOGS_DIR);
            files_to_ram(last, path);

            f_mount(NULL, "", 0);
        }
        free(fs);
        osDelay(1000);

        dialog_set_text("Formating storage");
        INFO("Formating storage to RED");
        if (red_format("") == 0)
        {
            if(red_mount("") == 0)
            {
                dialog_set_text("Restoring config");
                files_from_ram(first.next);
                osDelay(1000);
            }
            else
            {
                dialog_show("Error", "Unable to mount RED volume", dialog_confirm, migrate_fail_cb);
                vTaskSuspend(NULL);
            }
        }
        else
        {
            dialog_show("Error", "Unable to format storage", dialog_confirm, migrate_fail_cb);
            vTaskSuspend(NULL);
        }
    }
    else
    {
        dialog_show("Error", "Unable to mount FatFs volume", dialog_confirm, migrate_fail_cb);
        vTaskSuspend(NULL);
    }

    dialog_close();

    osSemaphoreRelease(lock);
    osSemaphoreDelete(lock);

    vTaskDelete(NULL);
}

void sd_migrate_cb(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
		dialog_show("Migrating", "Updating bootloader", dialog_progress, NULL);
		dialog_progress_spin();
		xTaskCreate((TaskFunction_t)sd_migrate_worker, "sd_migrate", 1024 * 4, NULL, 24, NULL);
	}
    else
    {
        //reboot
        system_reboot_hard();
    }

}

void sd_migrate_dialog()
{
    lock = osSemaphoreNew(1, 0, NULL);
    vQueueAddToRegistry(lock, "sd_lock");

    gui_startup_task = &gui_blank;
    start_thread(thread_gui);

    gui_lock_acquire();
    gui_lock_release();

    dialog_show("Warning", "This firmware is using new method to store data.\n\n"
            "We need to reformat the storage.\n\n"
            "Your IGC files, maps and AGL will be lost.\n\n"
            "If you need to backup them, press cancel.\n\n"
            "Your configuration will be preserved\n\n"
            "Continue?\n", dialog_yes_no, sd_migrate_cb);

    osSemaphoreAcquire(lock, WAIT_INF);
}
