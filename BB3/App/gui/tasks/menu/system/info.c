/*
 * info.c
 *
 *  Created on: Apr 30, 2021
 *      Author: horinek
 */


#include "gui/gui_list.h"

#include "system.h"

#include "drivers/rev.h"
#include "drivers/esp/download/slot.h"

#include "gui/dialog.h"

REGISTER_TASK_I(info,
    char new_fw[32];
    uint8_t slot_id;
);

void info_update_progress_cb(uint8_t res, void * data)
{
    if (res == dialog_res_cancel)
    {
        esp_http_stop(local->slot_id);
    }
}

void info_update_apply_cb(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        char path[64];

        snprintf(path, sizeof(path), "%s/%s", PATH_FW_DIR, local->new_fw);
        f_unlink(PATH_UPDATE_FILE);
        if (copy_file(path, PATH_UPDATE_FILE))
        {
            system_reboot();
        }
    }
}

void info_update_get_file_cb(uint8_t res, download_slot_t * ds)
{
    gui_lock_acquire();

    INFO("info_update_get_file_cb cb %u", res);

    if (res == DOWNLOAD_SLOT_PROGRESS)
    {
        dialog_progress_set_progress((ds->pos * 100) / ds->size);
    }
    else if (res == DOWNLOAD_SLOT_COMPLETE)
    {
        dialog_close();
        download_slot_file_data_t * data = (download_slot_file_data_t *)ds->data;
        char path[64];

        snprintf(path, sizeof(path), "%s/%s", PATH_FW_DIR, local->new_fw);

        f_unlink(path);
        f_rename(data->name, path);
        f_unlink(data->name);

        dialog_show("Start update process?", "", dialog_yes_no, info_update_apply_cb);
    }
    else
    {
        dialog_downloads_error(res);
    }

    gui_lock_release();
}

void info_update_question_cb(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        char url[128];

        snprintf(url, sizeof(url), "%s/%s/fw/%s", config_get_text(&config.system.server_url), config_get_select_text(&config.system.firmware_channel), local->new_fw);

        local->slot_id = esp_http_get(url, DOWNLOAD_SLOT_TYPE_FILE, info_update_get_file_cb);
        dialog_show("Downloading firmware", "", dialog_progress, info_update_progress_cb);
        dialog_progress_spin();
        dialog_progress_set_subtitle(local->new_fw);
    }
}

void info_update_get_info_cb(uint8_t res, download_slot_t * ds)
{
    gui_lock_acquire();

    if (res == DOWNLOAD_SLOT_COMPLETE)
    {
        dialog_close();

        char msg[64];

        if (read_value(ds->data, "firmware", local->new_fw, sizeof(local->new_fw)))
        {
            char rev_str[10];
            rew_get_sw_string(rev_str);
            if (strncmp(rev_str, local->new_fw, strlen(rev_str)) == 0)
            {
                dialog_show("Up to date", "You are using the latest firmware", dialog_confirm, NULL);
            }
            else
            {
                snprintf(msg, sizeof(msg), "Download firmware %s?", local->new_fw);
                dialog_show("Firmware update", msg, dialog_yes_no, info_update_question_cb);
            }
        }
        else
        {
            dialog_downloads_error(DOWNLOAD_SLOT_NOT_FOUND);
        }
    }
    else
    {
        dialog_downloads_error(res);
    }

    gui_lock_release();
}

void info_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
    if (event == LV_EVENT_CANCEL)
        gui_switch_task(&gui_system, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

    if (event == LV_EVENT_CLICKED)
    {
        if (index == 0)
        {
            char url[128];

            snprintf(url, sizeof(url), "%s/%s/", config_get_text(&config.system.server_url), config_get_select_text(&config.system.firmware_channel));

            local->slot_id = esp_http_get(url, DOWNLOAD_SLOT_TYPE_PSRAM, info_update_get_info_cb);
            dialog_show("Checking for updates", "", dialog_progress, info_update_progress_cb);
            dialog_progress_spin();
        }
    }

}


lv_obj_t * info_init(lv_obj_t * par)
{
    lv_obj_t * list = gui_list_create(par, "Device info", info_cb);

    char rev_str[10];
    char value[32];

    rew_get_sw_string(rev_str);
    snprintf(value, sizeof(value), "Firmware %s", rev_str);
    gui_list_info_add_entry(list, "Check for updates", value);

    snprintf(value, sizeof(value), "%08lX", DEVICE_ID);
    gui_list_info_add_entry(list, "Serial number", value);

    snprintf(value, sizeof(value), "%02X", rev_get_hw());
    gui_list_info_add_entry(list, "Hardware revision", value);


    return list;
}