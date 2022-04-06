/*
 * upload_crash.c
 *
 *  Created on: 06.04.2022
 *      Author: simonseyer
 */

#define DEBUG_LEVEL DBG_DEBUG
#include "upload_crash.h"
#include "drivers/esp/upload/slot.h"

void update_info_callback(uint8_t res, upload_slot_t * slot)
{
    switch (res)
    {
        case(UPLOAD_SLOT_PROGRESS):
            break;
        case(UPLOAD_SLOT_COMPLETE):
        {
            INFO("Uploading crash report finished");
            upload_crash_reports_schedule();

            // TODO: Delete file when upload completed
            break;
        }
        case(UPLOAD_SLOT_NO_CONNECTION):
        {
            WARN("Uploading crash report failed: no connection");
            break;
        }
        case(UPLOAD_SLOT_NO_FILE):
        {
            ERR("Uploading crash report failed: no file");
            break;
        }
        case(UPLOAD_SLOT_FAILED):
        {
            WARN("Uploading crash report failed: upload failed");
            break;
        }
        case(UPLOAD_SLOT_NO_SLOT):
        {
            WARN("Uploading crash report failed: no slot");
            break;
        }
        case(UPLOAD_SLOT_TIMEOUT):
        {
            WARN("Uploading crash report failed: timeout");
            break;
        }
    }
}

uint8_t upload_crash_report(char * bundle_file)
{
    INFO("Uploading crash report: %s", bundle_file);

    // TODO: Make URL configurable (with default)
    char url[128];
    snprintf(url, sizeof(url), "http://192.168.0.147/%s", bundle_file);

    return esp_http_post(url, bundle_file, update_info_callback);
}


void upload_crash_reports(void * arg)
{
    DIR dir;
    FILINFO fno;

    DBG("Uploading crash reports...");

    FRESULT res = f_opendir(&dir, "/");
    if (res == FR_OK)
    {
        while (true)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
                break;

            // Check for files that start with PATH_CRASH_BUNDLE
            if ((fno.fattrib & AM_DIR) || strncmp(PATH_CRASH_BUNDLE, fno.fname, strlen(PATH_CRASH_BUNDLE)) != 0)
            {
                continue;
            }

            if (upload_crash_report(fno.fname) != UPLOAD_SLOT_NONE)
            {
                break;
            }

            break; // Only upload one file for now (next scan is triggered when upload finished)
        }

        f_closedir(&dir);
    }
    else
    {
        ERR("Failed to read root directory: %d", res);
    }
}

void upload_crash_reports_schedule()
{
    // TODO: Add option to disable automatic crash reporting
    osTimerId_t timer = osTimerNew(upload_crash_reports, osTimerOnce, NULL, NULL);
    osTimerStart(timer, 8000);
}
