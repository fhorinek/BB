/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "page_settings.h"
#include "pages.h"
#include "widget_list.h"
#include "page_edit.h"

#include "gui/gui_list.h"

REGISTER_TASK_I(page_settings,
		char page_name[PAGE_NAME_LEN + 1];

		lv_obj_t * name_entry;
);

void page_settings_set_page_name(char * name)
{
	strncpy(local->page_name, name, sizeof(local->page_name));

	gui_list_textbox_set_value(local->name_entry, name);
}

static void page_setting_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_LEFT);

	if (event == LV_EVENT_CLICKED)
	{
        if (index == 1)
        {
            gui_switch_task(&gui_page_edit, LV_SCR_LOAD_ANIM_MOVE_LEFT);
            page_edit_set_page_name(local->page_name);
        }

	}
}


static lv_obj_t * page_settings_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Page settings", page_setting_cb);

	local->name_entry = gui_list_textbox_add_entry(list, "Name", "", PAGE_NAME_LEN);
    gui_list_text_add_entry(list, "Edit layout");


	return list;
}
