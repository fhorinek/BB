/*
 * settings.cc
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */


#include "page_autoset.h"

#include "page_settings.h"

#include "gui/gui_list.h"
#include "gui/dialog.h"

REGISTER_TASK_I(page_autoset,
	lv_obj_t * note;

	lv_obj_t * power_on;
	lv_obj_t * take_off;
	lv_obj_t * circle;
	lv_obj_t * glide;
	lv_obj_t * land;

	cfg_entry_t * entry;


	char page_name[PAGE_NAME_LEN + 1];
	uint8_t page_index;


);

bool page_is_set(cfg_entry_t * cfg)
{
	return strcmp(local->page_name, config_get_text(cfg)) == 0;
}


void page_autoset_set_page_name(char * filename, uint8_t index)
{
    strncpy(local->page_name, filename, sizeof(local->page_name));
    local->page_index = index;


    char msg[64];
    snprintf(msg, sizeof(msg), "Switch to page\n'%s' when...", local->page_name);
    gui_list_note_set_text(local->note, msg);

	gui_list_switch_set_value(local->power_on, page_is_set(&profile.ui.autoset.power_on));
	gui_list_switch_set_value(local->take_off, page_is_set(&profile.ui.autoset.take_off));
	gui_list_switch_set_value(local->circle, page_is_set(&profile.ui.autoset.circle));
	gui_list_switch_set_value(local->glide, page_is_set(&profile.ui.autoset.glide));
	gui_list_switch_set_value(local->land, page_is_set(&profile.ui.autoset.land));

}


void page_austoset_dialog_cb(uint8_t res, void * data)
{
	if (res == dialog_res_yes)
	{
		config_set_text(local->entry, local->page_name);
	}
	else
	{
		gui_list_switch_set_value(dialog_get_opt_data(), false);
	}
}

bool page_autoset_event_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
	switch(event)
	{
		case (LV_EVENT_CANCEL):
			gui_switch_task(&gui_page_settings, LV_SCR_LOAD_ANIM_MOVE_LEFT);
			page_settings_set_page_name(local->page_name, local->page_index);
		break;

		case (LV_EVENT_VALUE_CHANGED):
		{
			cfg_entry_t * e = NULL;

			if (obj == local->power_on)
				e = &profile.ui.autoset.power_on;
			else if (obj == local->take_off)
				e = &profile.ui.autoset.take_off;
			else if (obj == local->circle)
				e = &profile.ui.autoset.circle;
			else if (obj == local->glide)
				e = &profile.ui.autoset.glide;
			else if(obj == local->land)
				e = &profile.ui.autoset.land;
			else
				break;

			bool cfg_on = page_is_set(e);
			bool tgl_on = gui_list_switch_get_value(obj);

			if (cfg_on == tgl_on)
				break;

			if (cfg_on)
			{
				config_set_text(e, "");
				gui_list_switch_set_value(obj, false);
			}
			else
			{
				if (strlen(config_get_text(e)) != 0)
				{
					//dialog
					char msg[128];
					char * label = gui_list_switch_get_title(obj);

					local->entry = e;

					snprintf(msg, sizeof(msg), "Event '%s' is currently set to page\n'%s'.\n\nDo you want to change it to\n'%s'?", label, config_get_text(e), local->page_name);
					dialog_show("Warning", msg, dialog_yes_no, page_austoset_dialog_cb);
					dialog_add_opt_data(obj);
				}
				else
				{
					//set
					config_set_text(e, local->page_name);
					gui_list_switch_set_value(obj, true);
				}
			}
		}
		break;
	}
	return true;
}

lv_obj_t * page_autoset_init(lv_obj_t * par)
{
    help_set_base("Page/Autoset");

	lv_obj_t * list = gui_list_create(par, "Page autoset", NULL, page_autoset_event_cb);

	local->note = gui_list_note_add_entry(list, "", LV_COLOR_BLACK);

	local->power_on = gui_list_switch_add_entry(list, "Powered on", false);
	local->take_off = gui_list_switch_add_entry(list, "Take-off", false);
	local->circle = gui_list_switch_add_entry(list, "Circling", false);
	local->glide = gui_list_switch_add_entry(list, "Gliding", false);
	local->land = gui_list_switch_add_entry(list, "Landed", false);

	return list;
}

