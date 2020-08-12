/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "settings.h"

#include "gnss.h"
#include "pages.h"
#include "fanet.h"
#include "display.h"

#include "../gui_list.h"

void setting_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_pages, GUI_SW_LEFT_RIGHT);

	if (event == LV_EVENT_CLICKED)
	{
		if (index == 0)
			gui_switch_task(&gui_gnss, GUI_SW_RIGHT_LEFT);
		if (index == 1)
			gui_switch_task(&gui_fanet, GUI_SW_RIGHT_LEFT);
		if (index == 2)
			gui_switch_task(&gui_display, GUI_SW_RIGHT_LEFT);
	}

}


lv_obj_t * settings_init(lv_obj_t * par)
{
	DBG("settings init");

	lv_obj_t * list = gui_list_create(par, "Device settings", setting_cb);

	gui_list_add_text_entry(list, "GNSS");
	gui_list_add_text_entry(list, "FANET");
	gui_list_add_text_entry(list, "Display");

	return list;
}

void settings_loop()
{

}

bool settings_stop()
{
	return true;
}

gui_task_t gui_settings =
{
	settings_init,
	settings_loop,
	settings_stop
};
