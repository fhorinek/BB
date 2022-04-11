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

typedef struct
{
    lv_obj_t *status_bar_progress_handle;
} upload_crash_context_t;

void upload_crash_callback(uint8_t status, upload_slot_t *slot)
{
    upload_crash_context_t *context = (upload_crash_context_t*) slot->context;
    switch (status)
    {
        case (UPLOAD_SLOT_PROGRESS):
            {
            if (context->status_bar_progress_handle == NULL)
            {
                context->status_bar_progress_handle = statusbar_msg_add(STATUSBAR_MSG_PROGRESS, "Uploading crash report");
            }

            ASSERT(slot->file_size > 0);
            ASSERT(slot->transmitted_size  <= slot->file_size);
            uint8_t progress = (slot->transmitted_size * 100) / slot->file_size;
            statusbar_msg_update_progress(context->status_bar_progress_handle, progress);

            break;
        }

        case (UPLOAD_SLOT_COMPLETE):
            {
            INFO("Uploading crash report finished: %s", slot->file_path);
            statusbar_msg_add(STATUSBAR_MSG_INFO, "Crash report sent");

            uint8_t result = FR_OK;//f_unlink(slot->file_path);
            if (result == FR_OK)
            {
                upload_crash_reports_schedule();
            }
            else
            {
                // Don't schedule on failure to avoid endless loop (retry on next WiFi connect)
                ERR("Failed to delete crash bundle: %u", result);
            }

            break;
        }
        case (UPLOAD_SLOT_NO_CONNECTION):
            {
            // Skip status bar update — silently retry on next WiFi connect
            WARN("Uploading crash report failed: no connection");
            break;
        }
        case (UPLOAD_SLOT_FAILED):
            {
            statusbar_msg_add(STATUSBAR_MSG_WARN, "Failed to send crash report");
            WARN("Uploading crash report failed: upload failed");
            break;
        }
        case (UPLOAD_SLOT_NO_SLOT):
            {
            // Skip status bar update — silently retry on next WiFi connect
            WARN("Uploading crash report failed: no slot");
            break;
        }
        case (UPLOAD_SLOT_TIMEOUT):
            {
            statusbar_msg_add(STATUSBAR_MSG_WARN, "Failed to send crash report");
            WARN("Uploading crash report failed: timeout");
            break;
        }
    }

    if (status != UPLOAD_SLOT_PROGRESS)
    {
        if (context->status_bar_progress_handle != NULL)
        {
            statusbar_msg_close(context->status_bar_progress_handle);
        }
        free(context);
    }
}

upload_slot_t* upload_crash_report(char *bundle_file)
{
    INFO("Uploading crash report");

    upload_crash_context_t *context = (upload_crash_context_t *) malloc(sizeof(upload_crash_context_t));
    context->status_bar_progress_handle = NULL;

    return esp_http_upload(config_get_text(&config.debug.crash_reporting_url), bundle_file, upload_crash_callback, context);
}

void upload_crash_reports(void *arg)
{
    DIR dir;
    FILINFO fno;

    DBG("Looking crash reports...");

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
