#include "empty.h"

#include <gui/tasks/menu/settings.h>

#include "gui/keyboard.h"
#include "gui/gui_list.h"

REGISTER_TASK_I(empty,
	lv_obj_t * item;
);

//this is menu template

static void empty_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	//back
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

	//textbox
	if (event == LV_EVENT_LEAVE || event == LV_EVENT_APPLY || event == LV_EVENT_FOCUSED)
	{
		if (obj == local->item)
		{
			keyboard_hide();
		}
	}

	//click
	if (event == LV_EVENT_CLICKED)
	{
		if (obj == local->item)
		{
		}

		if (index == 0)
		{

		}

	}

	//switch / slider
	if (event == LV_EVENT_VALUE_CHANGED)
	{

	}


}

static lv_obj_t * empty_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Empty", empty_cb);

//    local->item = gui_list_checkbox_add_entry(list, "Checkbox");
//    local->item = gui_list_text_add_entry(list, "Text");
//    local->item = gui_list_info_add_entry(list, "Info", "Value");
//    local->item = gui_list_switch_add_entry(list, "Switch", true);
//    local->item = gui_list_slider_add_entry(list, "Slider", 0, 100, 50);
//    local->item = gui_list_dropdown_add_entry(list, "Dropdown", "1/n2/n3", 0);

	return list;
}



