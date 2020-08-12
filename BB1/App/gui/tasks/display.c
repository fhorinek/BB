#include "display.h"

#include "settings.h"

#include "../gui_list.h"

#include "../../config/config.h"
#include "../../fc/fc.h"
#include "../../etc/format.h"

typedef struct
{
	lv_obj_t * brightness_slider;
} local_vars_t;

void display_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, GUI_SW_LEFT_RIGHT);

	if (event == LV_EVENT_VALUE_CHANGED)
	{
		switch(index)
		{
			case 0:
			{
				int16_t val = gui_list_slider_get_value(local.brightness_slider) * 10;
				gui_set_backlight(val);
				config_set_int(&config.settings.display.backlight, val);
			}
			break;
		}

	}
}

lv_obj_t * display_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Display Settings", display_cb);

	local.brightness_slider = gui_list_slider_add_entry(list, "Brightness", 0, 10, config_get_int(&config.settings.display.backlight) / 10);

	return list;
}

void display_loop()
{

}

bool display_stop()
{
	return true;
}

gui_task_t gui_display =
{
	display_init,
	display_loop,
	display_stop
};
