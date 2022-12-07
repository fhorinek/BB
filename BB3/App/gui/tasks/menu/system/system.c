/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */



#include "calibration/calib_menu.h"
#include "diagnostic.h"
#include "gui/gui_list.h"

#include "datetime/datetime.h"
#include "display.h"
#include "info.h"
#include "firmware.h"
#include "units.h"

#include "../settings.h"

#include "drivers/rev.h"
#include "gui/dialog.h"

REGISTER_TASK_I(system);

static void restore_dialog_cb(uint8_t res, void * data)
{
	if (res == dialog_res_yes)
		config_restore_factory();
}

static bool restore_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        dialog_show(_("Warning"), _("Do you want to restore factory settings?"), dialog_yes_no, restore_dialog_cb);
    }

    return true;
}


lv_obj_t * system_init(lv_obj_t * par)
{
    help_set_base("System");

    lv_obj_t * list = gui_list_create(par, _("System"), &gui_settings, NULL);

    gui_list_auto_entry(list, _("Time & date"), NEXT_TASK, &gui_datetime);
    gui_list_auto_entry(list, _("Display"), NEXT_TASK, &gui_display);
    gui_list_auto_entry(list, _("Units"), NEXT_TASK, &gui_units);
    gui_list_auto_entry(list, _("Advanced"), NEXT_TASK, &gui_advanced);

    char rev_str[20];
    rev_get_sw_string(rev_str);
    lv_obj_t * fw = gui_list_info_add_entry(list, _("Firmware"), rev_str);
    gui_config_entry_add(fw, NEXT_TASK, &gui_firmware);

    gui_list_auto_entry(list, _("Device info"), NEXT_TASK, &gui_info);
    gui_list_auto_entry(list, "Diagnostic", NEXT_TASK, &gui_diagnostic);

    gui_list_auto_entry(list, _("Restore factory settings"), CUSTOM_CB, restore_cb);

    return list;
}
