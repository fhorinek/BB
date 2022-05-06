/*
 * sd_format.c
 *
 *  Created on: May 4, 2022
 *      Author: horinek
 */

#include "sd_format.h"

#include "gui/tasks/blank.h"
#include "gui/dialog.h"
#include "gui_thread.h"
#include "gui/tasks/page/pages.h"

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

void sd_format_worker(void * param)
{
    (void)param;

    char path[PATH_LEN];

    FATFS * fs = malloc(sizeof(FATFS));
    FRESULT res = f_mount(fs, "", true);

    if (res == FR_OK)
    {
        ram_file_t * first = malloc(sizeof(ram_file_t));
        first->path = NULL;
        first->data = NULL;

        dialog_set_text("Storing config");

        if (res == FR_OK)
        {
            strcpy(path, PATH_ASSET_DIR);
            ram_file_t * last = files_to_ram(first, path);

            strcpy(path, PATH_CONFIG_DIR);
            last = files_to_ram(last, path);

            strcpy(path, PATH_LOGS_DIR);
            files_to_ram(last, path);

            f_mount(NULL, "", 0);
        }
        free(fs);

        dialog_set_text("Formating storage");
        INFO("Formating storage to RED");
        if (red_format("") == 0)
        {
            if(red_mount("") == 0)
            {
                dialog_set_text("Restoring config");
                files_from_ram(first->next);
            }
            else
            {
                dialog_show("Error", "Unable to mount RED volume", dialog_confirm, NULL);
            }
        }
        else
        {
            dialog_show("Error", "Unable to format storage", dialog_confirm, NULL);
        }
    }
    else
    {
        dialog_show("Error", "Unable to mount FatFs volume", dialog_confirm, NULL);
    }

    dialog_close();

    osSemaphoreRelease(lock);
    osSemaphoreDelete(lock);

    vTaskDelete(NULL);
}

void sd_format_cb(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        dialog_show("Migrating", "please wait", dialog_progress, NULL);
        dialog_progress_spin();
        xTaskCreate((TaskFunction_t)sd_format_worker, "sd_format_worker", 1024 * 2, NULL, 24, NULL);
    }
    else
    {
        //reboot to bl
        system_reboot_bl();
    }

}

void sd_format_dialog()
{
    lock = osSemaphoreNew(1, 0, NULL);
    vQueueAddToRegistry(lock, "sd_lock");

    gui_startup_task = &gui_blank;
    start_thread(thread_gui);
    osSemaphoreAcquire(gui.lock, WAIT_INF);
    osSemaphoreRelease(gui.lock);

    dialog_show("Warning", "This firmware is using new method to store data.\n\n"
            "We need to reformat the storage.\n\n"
            "Your IGC files, maps and AGL will be lost.\n\n"
            "If you need to backup them, press cancel.\n\n"
            "Your configuration will be preserved\n\n"
            "Continue?\n", dialog_yes_no, sd_format_cb);

    osSemaphoreAcquire(lock, WAIT_INF);
}
