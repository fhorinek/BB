/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "page_settings.h"
#include "pages.h"

#include "../gui_list.h"
#include "../../config/config.h"

REGISTER_TASK_I(page_settings,
		char page_name[PAGE_NAME_LEN + 1];

		lv_obj_t * name_entry;
);

void page_settings_open_page(char * name)
{
	strncpy(local->page_name, name, sizeof(local->page_name));

	gui_list_info_set_value(local->page_name, name);
}

static void page_setting_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_LEFT);

	if (event == LV_EVENT_CLICKED)
	{
//		if (index == 0)
//			gui_switch_task(&gui_gnss, LV_SCR_LOAD_ANIM_MOVE_LEFT);
//		if (index == 1)
//			gui_switch_task(&gui_fanet, LV_SCR_LOAD_ANIM_MOVE_LEFT);
//		if (index == 2)
//			gui_switch_task(&gui_display, LV_SCR_LOAD_ANIM_MOVE_LEFT);
	}

}


static lv_obj_t * page_settings_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Page settings", page_setting_cb);

	local->name_entry = gui_list_textbox_add_entry(list, "Name", "", PAGE_NAME_LEN);
	gui_list_text_add_entry(list, "Modify widgets");
	gui_list_text_add_entry(list, "Add widget");
//	gui_list_text_add_entry(list, "Add widget");
//	gui_list_text_add_entry(list, "Add widget");
//	gui_list_text_add_entry(list, "Add widget");
//	gui_list_text_add_entry(list, "Add widget");
//	gui_list_text_add_entry(list, "Add widget");
//	gui_list_text_add_entry(list, "Add widget");
//	gui_list_text_add_entry(list, "Add widget");
//	gui_list_text_add_entry(list, "Add widget");
//	gui_list_text_add_entry(list, "Add widget");
//	gui_list_text_add_entry(list, "Add widget");

	return list;
}
