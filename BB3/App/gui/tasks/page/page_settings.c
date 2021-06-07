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
#include "gui/statusbar.h"
#include "gui/widgets/pages.h"
#include "gui/keyboard.h"
#include "gui/dialog.h"

REGISTER_TASK_I(page_settings,
		char page_name[PAGE_NAME_LEN + 1];

		lv_obj_t * name_entry;

		uint8_t page_index;
);

void page_settings_set_page_name(char * name, uint8_t index)
{
	strncpy(local->page_name, name, sizeof(local->page_name));

	gui_list_textbox_set_value(local->name_entry, name);
	local->page_index = index;
}

void page_settings_delete_cb(uint8_t res, void * data)
{
	if (res == dialog_res_yes)
	{
		page_delete(local->page_name);
		config_set_text(&profile.ui.page[local->page_index], "");
		pages_defragment();

		gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_LEFT);
	}
}

static void page_setting_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_LEFT);

	if (event == LV_EVENT_LEAVE || event == LV_EVENT_APPLY || event == LV_EVENT_FOCUSED)
	{
		if (obj == local->name_entry)
		{
			keyboard_hide();

			char * text = (char *)gui_list_textbox_get_value(local->name_entry);

			if (strcmp(text, local->page_name) == 0)
				return;

			if (strlen(text) == 0)
			{
				gui_list_textbox_set_value(local->name_entry, local->page_name);
				return;
			}

			if (!page_rename(local->page_name, text))
			{
				gui_list_textbox_set_value(local->name_entry, local->page_name);
				statusbar_add_msg(STATUSBAR_MSG_ERROR, "Already exists!");
			}
			else
			{
				config_set_text(&profile.ui.page[local->page_index], text);
			}
		}
	}

	if (event == LV_EVENT_CLICKED)
	{
        if (index == 1)
        {
            gui_switch_task(&gui_page_edit, LV_SCR_LOAD_ANIM_MOVE_LEFT);
            page_edit_set_page_name(local->page_name, local->page_index);
        }

        if (index == 2)
        {
        	if (pages_get_count() < PAGE_MAX_COUNT)
        	{
				//create new page
        		for (uint8_t i = 1; i < 100; i++)
        		{
        			char new_name[16];
        			snprintf(new_name, sizeof(new_name), "new%u", i);
        			if (page_create(new_name))
        			{
        				uint8_t index = pages_get_count();
        				config_set_text(&profile.ui.page[index], new_name);
        				config_set_int(&profile.ui.page_last, index);
        				gui_switch_task(&gui_pages, LV_SCR_LOAD_ANIM_MOVE_LEFT);

        				break;
        			}
        		}
        	}
        }

        if (index == 3)
        {
        	dialog_show("Remove page", "Do you want to remove this page", dialog_yes_no, page_settings_delete_cb);
        }
	}


}


static lv_obj_t * page_settings_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Page settings", page_setting_cb);

	local->name_entry = gui_list_textbox_add_entry(list, "Name", "", PAGE_NAME_LEN);
    gui_list_text_add_entry(list, "Edit layout");

	gui_list_text_add_entry(list, "Add page");
	gui_list_text_add_entry(list, "Remove page");


	return list;
}
