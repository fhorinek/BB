
#include "settings.h"
#include "gui/gui_list.h"

#include "drivers/esp/protocol.h"
#include "etc/format.h"

REGISTER_TASK_I(bluetooth,
	lv_obj_t * volume;
);

gui_list_slider_options_t vol_opt = {
	.disp_multi = 0.01,
	.step = 1,
	.format = format_percent,
};

static void bluetooth_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		if (obj == local->volume)
		{
			uint8_t vol = gui_list_slider_get_value(local->volume);
			esp_set_volume(vol);
			gui_config_entry_update(obj, &config.bluetooth.volume, &vol_opt);
		}
	}

	return true;
}


static lv_obj_t * bluetooth_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Audio", &gui_settings, bluetooth_cb);

	local->volume = gui_config_entry_create(list, &config.bluetooth.volume, "Volume", &vol_opt);

	return list;
}



