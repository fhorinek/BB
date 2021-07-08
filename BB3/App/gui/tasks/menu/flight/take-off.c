#include "take-off.h"

#include "gui/tasks/menu/settings.h"

#include "gui/gui_list.h"
#include "gui/tasks/menu/flight/flight.h"

#include "etc/format.h"

REGISTER_TASK_I(take_off);

static gui_list_slider_options_t alt_param = {
	.disp_multi = 1,
	.step = 1,
	.format = format_altitude_with_units,
};

static gui_list_slider_options_t spd_param = {
	.disp_multi = 1,
	.step = 1,
	.format = format_speed_with_units,
};

static gui_list_slider_options_t time_param = {
	.disp_multi = 1,
	.step = 1,
	.format = format_duration,
};

static lv_obj_t * take_off_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Automatic Take-off", &gui_flight, NULL);

	gui_config_entry_create(list, &profile.flight.auto_take_off.alt_change_enabled, "Use altitude", NULL);
	gui_config_entry_create(list, &profile.flight.auto_take_off.alt_change_value, "Change greater then", &alt_param);
	gui_config_entry_create(list, &profile.flight.auto_take_off.speed_enabled, "Use ground speed", NULL);
	gui_config_entry_create(list, &profile.flight.auto_take_off.speed_value, "Exceed speed", &spd_param);
	gui_config_entry_create(list, &profile.flight.auto_take_off.timeout, "Within time", &time_param);

	return list;
}



