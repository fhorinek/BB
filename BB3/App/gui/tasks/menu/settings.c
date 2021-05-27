/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */

#include <gui/tasks/menu/settings.h>

#include <gui/tasks/menu/bluetooth.h>
#include <gui/tasks/menu/development/development.h>
#include <gui/tasks/menu/fanet.h>
#include <gui/tasks/menu/gnss/gnss.h>
#include <gui/tasks/menu/system/system.h>
#include <gui/tasks/menu/wifi/wifi.h>
#include "gui/tasks/page/pages.h"

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
            gui_switch_task(&gui_bluetooth, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 3)
            gui_switch_task(&gui_wifi, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 4)
            gui_switch_task(&gui_system, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        if (index == 5)
            gui_switch_task(&gui_development, LV_SCR_LOAD_ANIM_MOVE_LEFT);

	}

}


lv_obj_t * settings_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Device settings", setting_cb);

	gui_list_text_add_entry(list, "GNSS");
	gui_list_text_add_entry(list, "FANET");
    gui_list_text_add_entry(list, "Bluetooth");
    gui_list_text_add_entry(list, "Wi-Fi");
    gui_list_text_add_entry(list, "System");
    gui_list_text_add_entry(list, "Development");

	return list;
}
