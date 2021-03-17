/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "settings.h"

#include "gui/tasks/page/pages.h"

#include "gnss/gnss.h"
#include "wifi/wifi.h"
#include "fanet.h"
#include "display.h"
#include "development.h"
#include "bluetooth.h"
#include "calibration.h"
#include "datetime/datetime.h"

#include "gui/gui_list.h"

REGISTER_TASK_I(settings);

void setting_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

	if (event == LV_EVENT_CLICKED)
	{
		if (index == 0)
			gui_switch_task(&gui_gnss, LV_SCR_LOAD_ANIM_MOVE_LEFT);

		if (index == 1)
			gui_switch_task(&gui_fanet, LV_SCR_LOAD_ANIM_MOVE_LEFT);

		if (index == 2)
			gui_switch_task(&gui_display, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 3)
            gui_switch_task(&gui_bluetooth, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 4)
            gui_switch_task(&gui_wifi, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 5)
            gui_switch_task(&gui_datetime, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 6)
            gui_switch_task(&gui_development, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 7)
            gui_switch_task(&gui_calibration, LV_SCR_LOAD_ANIM_MOVE_LEFT);
	}

}


lv_obj_t * settings_init(lv_obj_t * par)
{
	DBG("settings init");

	lv_obj_t * list = gui_list_create(par, "Device settings", setting_cb);

	gui_list_text_add_entry(list, "GNSS");
	gui_list_text_add_entry(list, "FANET");
    gui_list_text_add_entry(list, "Display");
    gui_list_text_add_entry(list, "Bluetooth");
    gui_list_text_add_entry(list, "Wi-Fi");
    gui_list_text_add_entry(list, "Time & date");
    gui_list_text_add_entry(list, "Development");
    gui_list_text_add_entry(list, "Calibration");

	return list;
}
