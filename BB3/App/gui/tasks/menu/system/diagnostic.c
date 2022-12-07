/*
 * advenced.c
 *
 *  Created on: Sep 27, 2021
 *      Author: horinek
 */

#include "diagnostic.h"
#include "system.h"
#include "gui/gui_list.h"
#include "gui/dialog.h"

#include "fc/fc.h"
#include "drivers/nvm.h"
#include "fc/imu.h"


REGISTER_TASK_I(diagnostic);


static void clear_debug_file_dialog_cb(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        red_unlink(DEBUG_FILE);
    }
}


static bool clear_debug_file_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        dialog_show("Confirm", _("Clear debug file"), dialog_yes_no, clear_debug_file_dialog_cb);
    }

    return true;
}

static void diagnostic_crash_dialog_cb(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        config_set_bool(&config.debug.crash_dump, true);
        bsod_msg("Creating diagnostic report");
    }
}


static bool diagnostic_crash_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        dialog_show(_("Confirm"), _("This will stop the device and create diagnostic report.\n\nWe will trigger crash to generate the report.\n\nThe crash report will be available on your strato via USB massstorage."), dialog_yes_no, diagnostic_crash_dialog_cb);
    }

    return true;
}

lv_obj_t * diagnostic_init(lv_obj_t * par)
{
    lv_obj_t * list = gui_list_create(par, _("Diagnostics"), &gui_system, NULL);

    gui_list_note_add_entry(list, _("You do not need to do anything here, unless you are instructed by support."), LIST_NOTE_COLOR);

    gui_list_auto_entry(list, _("Create diagnostic report"), CUSTOM_CB, diagnostic_crash_cb);
    gui_list_auto_entry(list, _("Crash reports"), &config.debug.crash_dump, NULL);
    gui_list_auto_entry(list, _("Auto crash upload"), &config.debug.crash_reporting, NULL);
    gui_list_auto_entry(list, _("Debug to file"), &config.debug.use_file, NULL);
    gui_list_auto_entry(list, _("Clear debug.log"), CUSTOM_CB, clear_debug_file_cb);

    return list;
}
