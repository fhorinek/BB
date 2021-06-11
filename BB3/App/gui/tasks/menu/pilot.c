#include "pilot.h"

#include "gui/tasks/menu/settings.h"

#include "gui/keyboard.h"
#include "gui/gui_list.h"



REGISTER_TASK_I(pilot,
	lv_obj_t * name;
);

static void pilot_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

	if (event == LV_EVENT_LEAVE || event == LV_EVENT_APPLY || event == LV_EVENT_FOCUSED)
	{
		if (obj == local->name)
		{
			keyboard_hide();

			char * text = (char *)gui_list_textbox_get_value(local->name);

			config_set_text(&pilot.name, text);
		}
	}

}

static lv_obj_t * pilot_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Pilot", pilot_cb);

    local->name = gui_list_textbox_add_entry(list, "Pilot name", config_get_text(&pilot.name), PILOT_NAME_LEN);

	return list;
}



