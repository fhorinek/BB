#include "display.h"
#include "system.h"

#include "gui/gui_list.h"

#include "drivers/power/led.h"

#include "fc/fc.h"
#include "etc/format.h"

REGISTER_TASK_I(display,
	lv_obj_t * brightness_slider;
);

static bool display_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		switch(index)
		{
			case 0:
			{
				int16_t val = gui_list_slider_get_value(local->brightness_slider) * 10;
				led_set_backlight(val);
				config_set_int(&config.display.backlight, val);
			}
			break;
		}

	}

	return true;
}

lv_obj_t * display_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Display Settings", &gui_system, display_cb);

	local->brightness_slider = gui_list_slider_add_entry(list, "Brightness", 0, 10, config_get_int(&config.display.backlight) / 10);

	return list;
}

