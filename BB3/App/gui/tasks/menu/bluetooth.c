
#include "settings.h"
#include "gui/gui_list.h"

#include "drivers/esp/protocol.h"
#include "etc/format.h"

REGISTER_TASK_I(bluetooth,
	lv_obj_t * volume;
);

gui_list_slider_options_t vol_opt = {
	.disp_multi = 1,
	.step = 1,
	.format = format_percent,
};

static lv_obj_t * bluetooth_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Audio", &gui_settings, NULL);

	local->volume = gui_list_auto_entry(list, "Volume", &config.bluetooth.volume, &vol_opt);

	return list;
}



