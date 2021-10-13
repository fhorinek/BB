/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */



#include "gui/gui_list.h"

#include "datetime/datetime.h"
#include "advanced/advanced.h"
#include "display.h"
#include "info.h"
#include "units.h"

#include "../settings.h"

#include "drivers/rev.h"
#include "drivers/power/pwr_mng.h"
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
        dialog_show("Warning", "Do you want to restore factory settings?", dialog_yes_no, restore_dialog_cb);
    }

    return true;
}

static bool reboot_bt_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
    	if (pwr.charger.charge_port == PWR_CHARGE_NONE)
    	{
    		dialog_show("Bootloader update", "Connect left (power) usb to the charger or computer", dialog_confirm, NULL);
    		return true;
    	}

    	if (pwr.data_port == PWR_DATA_NONE)
    	{
    		dialog_show("Bootloader update", "Connect right (data) usb to the computer", dialog_confirm, NULL);
    		return true;
    	}

        dialog_show("Bootloader update", "Go to:\nstrato.skybean.eu/dfu/\n\nPress Connect Strato\n\nThen press and hold the bottom right (option) button", dialog_bootloader, NULL);
    }

    return true;
}

lv_obj_t * system_init(lv_obj_t * par)
{
    DBG("settings init");

    lv_obj_t * list = gui_list_create(par, "System", &gui_settings, NULL);

    gui_list_auto_entry(list, "Time & date", NEXT_TASK, &gui_datetime);
    gui_list_auto_entry(list, "Display", NEXT_TASK, &gui_display);
    gui_list_auto_entry(list, "Units", NEXT_TASK, &gui_units);
    gui_list_auto_entry(list, "Advanced", NEXT_TASK, &gui_advanced);

    char rev_str[10];
    rev_get_sw_string(rev_str);
    lv_obj_t * info = gui_list_info_add_entry(list, "Device info", rev_str);
    gui_config_entry_add(info, NEXT_TASK, &gui_info);

    gui_list_auto_entry(list, "Restore factory settings", CUSTOM_CB, restore_cb);
    gui_list_auto_entry(list, "Bootloader update", CUSTOM_CB, reboot_bt_cb);

    return list;
}
