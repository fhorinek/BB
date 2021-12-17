/*
 * info.c
 *
 *  Created on: Apr 30, 2021
 *      Author: horinek
 */


#include "gui/gui_list.h"

#include "system.h"

#include "fc/fc.h"

#include "drivers/rev.h"
#include "drivers/esp/download/slot.h"
#include "drivers/esp/protocol.h"
#include "drivers/power/pwr_mng.h"

#include "gui/dialog.h"
#include "gui/tasks/filemanager.h"
#include "gui/statusbar.h"
#include "gui/statusbar.h"

#include "etc/format.h"
#include "etc/bootloader.h"


REGISTER_TASK_I(info,
    char new_fw[32];
    uint8_t slot_id;
    uint8_t click_cnt;
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
        f_unlink(UPDATE_FILE);
        if (copy_file(path, UPDATE_FILE))
        {
            system_reboot();
        }
    }
}

void info_update_get_file_cb(uint8_t res, download_slot_t * ds)
{
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
        char tmp_path[TEMP_NAME_LEN];

        get_tmp_path(tmp_path, data->tmp_id);

        snprintf(path, sizeof(path), "%s/%s", PATH_FW_DIR, local->new_fw);

        f_unlink(path);
        f_rename(tmp_path, path);
        f_unlink(tmp_path);

        dialog_show("Start update process?", "", dialog_yes_no, info_update_apply_cb);
    }
    else
    {
        dialog_downloads_error(res);
    }
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
	if (res == DOWNLOAD_SLOT_PROGRESS)
		return;

    if (res == DOWNLOAD_SLOT_COMPLETE)
    {
        dialog_close();

        char msg[64];

        if (read_value(ds->data, "firmware", local->new_fw, sizeof(local->new_fw)))
        {
            char rev_str[10];
            rev_get_sw_string(rev_str);
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
}

static bool info_update_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
		char url[128];

		snprintf(url, sizeof(url), "%s/%s/", config_get_text(&config.system.server_url), config_get_select_text(&config.system.firmware_channel));

		local->slot_id = esp_http_get(url, DOWNLOAD_SLOT_TYPE_PSRAM, info_update_get_info_cb);
		dialog_show("Checking for updates", "", dialog_progress, info_update_progress_cb);
		dialog_progress_spin();
    }
    return true;
}

bool manual_install_fm_cb(uint8_t event, char * path)
{
    if (event == FM_CB_SELECT)
    {
        char text[64];
        path = strrchr(path, '/');

        if (path == NULL)
            return true;
        path++;

        snprintf(text, sizeof(text), "Install version %s", path);
        strncpy(local->new_fw, path, sizeof(local->new_fw));
        dialog_show("Start update?", text, dialog_yes_no, info_update_apply_cb);
    }
	return true;
}

static bool manual_install_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		gui_switch_task(&gui_filemanager, LV_SCR_LOAD_ANIM_MOVE_LEFT);
		filemanager_open(PATH_FW_DIR, 0, &gui_info, FM_FLAG_HIDE_DIR, manual_install_fm_cb);

		//supress default handler
		return false;
	}

	return true;
}

static bool info_serial_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
    	local->click_cnt++;
    	if (local->click_cnt > 5)
    	{
    		local->click_cnt = 0;

    		if (file_exists(DEV_MODE_FILE))
    		{
    			f_unlink(DEV_MODE_FILE);
    			statusbar_msg_add(STATUSBAR_MSG_INFO, "Developer mode disabled");
    		}
    		else
    		{
    			touch(DEV_MODE_FILE);
    			statusbar_msg_add(STATUSBAR_MSG_INFO, "Developer mode enabled");
    		}

    	}
    }
    return true;
}

static bool info_serial_release_note_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
    	gui_show_release_note();
    }
    return true;
}


static void update_bl_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        bootloader_res_t res = bootloader_update(PATH_BL_FW_MANUAL);
        switch (res)
        {
            case(bl_update_ok):
                statusbar_msg_add(STATUSBAR_MSG_INFO, "Bootloader successfully updated!");
            break;
            case(bl_same_version):
                statusbar_msg_add(STATUSBAR_MSG_INFO, "Bootloader already up-to-date");
            break;
            case(bl_file_invalid):
                statusbar_msg_add(STATUSBAR_MSG_ERROR, "Update file is corrupted!");
            break;
            case(bl_file_not_found):
                statusbar_msg_add(STATUSBAR_MSG_ERROR, "Update file not found!");
            break;
        }
    }
}

static bool reboot_dfu_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        if (pwr.charger.charge_port == PWR_CHARGE_NONE)
        {
            dialog_show("DFU", "Connect left (power) usb to the charger or computer", dialog_confirm, NULL);
            return true;
        }

        if (pwr.data_port == PWR_DATA_NONE)
        {
            dialog_show("DFU", "Connect right (data) usb to the computer", dialog_confirm, NULL);
            return true;
        }

        dialog_show("DFU", "Go to:\nstrato.skybean.eu/dfu/\n\nPress Connect Strato\n\nThen press and hold the bottom right (option) button", dialog_dfu, NULL);
    }

    return true;
}


lv_obj_t * info_init(lv_obj_t * par)
{
	local->click_cnt = 0;

    lv_obj_t * list = gui_list_create(par, "Device info", &gui_system, NULL);

    char rev_str[10];
    char value[32];
    lv_obj_t * obj;

    gui_list_auto_entry(list, "Check for updates", CUSTOM_CB, info_update_cb);

    rev_get_sw_string(rev_str);
    snprintf(value, sizeof(value), "Firmware ver. %s", rev_str);
    obj = gui_list_info_add_entry(list, "Release note", value);
    gui_config_entry_add(obj, CUSTOM_CB, info_serial_release_note_cb);

    gui_list_auto_entry(list, "Manual firmware install", CUSTOM_CB, manual_install_cb);

    snprintf(value, sizeof(value), "%08lX", rev_get_short_id());
    obj = gui_list_info_add_entry(list, "Serial number", value);
    gui_config_entry_add(obj, CUSTOM_CB, info_serial_cb);

    snprintf(value, sizeof(value), "%02X", rev_get_hw());
    gui_list_info_add_entry(list, "Hardware revision", value);

    snprintf(value, sizeof(value), "%lu", nvm->bootloader);
    gui_list_info_add_entry(list, "Bootloader version", value);

    if (file_exists(DEV_MODE_FILE))
    {
        gui_list_auto_entry(list, "Reboot to DFU", CUSTOM_CB, reboot_dfu_cb);

        if (file_exists(PATH_BL_FW_MANUAL))
        {
            snprintf(value, sizeof(value), "from /%s", PATH_BL_FW_MANUAL);
            obj = gui_list_info_add_entry(list, "Manual bootloader install", value);
            gui_config_entry_add(obj, CUSTOM_CB, update_bl_cb);
        }
    }


    return list;
}
