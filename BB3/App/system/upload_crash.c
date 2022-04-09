/*
 * upload_crash.c
 *
 *  Created on: 06.04.2022
 *      Author: simonseyer
 */

#define DEBUG_LEVEL DBG_DEBUG
#include "upload_crash.h"
#include "drivers/esp/upload/slot.h"
#include "drivers/esp/protocol.h"
#include "gui/statusbar.h"
#include "fatfs.h"

void upload_crash_callback(uint8_t res, upload_slot_t * slot)
{
    switch (res)
    {
        case(UPLOAD_SLOT_PROGRESS):
            break;
        case(UPLOAD_SLOT_COMPLETE):
        {
            INFO("Uploading crash report finished: %s", slot->file_path);
            statusbar_msg_add(STATUSBAR_MSG_INFO, "Crash report sent");

            upload_crash_reports_schedule();

            // TODO: Delete file when upload completed
            break;
        }
        case(UPLOAD_SLOT_NO_CONNECTION):
        {
            WARN("Uploading crash report failed: no connection");
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

upload_slot_t * upload_crash_report(char * bundle_file)
{
    char url[128];
    snprintf(url, sizeof(url), "%s/%s", config_get_text(&config.debug.crash_reporting_url), bundle_file);

    INFO("Uploading crash report: %s", url);

    return esp_http_upload(url, bundle_file, ESP_HTTP_CONTENT_TYPE_ZIP, upload_crash_callback);
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

            if (upload_crash_report(fno.fname) != NULL)
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

/**
 * Schedules uploading of crash bundles.
 *
 * Scans for crash_report ZIP files in the root of the SD card.
 * Bundles are uploaded to configured debug.crash_reporting_url.
 * Upload can be disabled by configuring debug.crash_reporting.
 */
void upload_crash_reports_schedule()
{
    if (!config_get_bool(&config.debug.crash_reporting))
        return;

    osTimerId_t timer = osTimerNew(upload_crash_reports, osTimerOnce, NULL, NULL);
    osTimerStart(timer, 8000);
}
