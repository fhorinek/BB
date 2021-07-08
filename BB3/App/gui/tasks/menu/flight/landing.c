#include "landing.h"

#include "gui/tasks/menu/flight/flight.h"

#include "gui/gui_list.h"

#include "etc/format.h"

REGISTER_TASK_I(landing);

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

static lv_obj_t * landing_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Automatic Landing", &gui_flight, NULL);

	gui_config_entry_create(list, &profile.flight.auto_landing.alt_change_enabled, "Use altitude", NULL);
	gui_config_entry_create(list, &profile.flight.auto_landing.alt_change_value, "Stay within", &alt_param);
	gui_config_entry_create(list, &profile.flight.auto_landing.speed_enabled, "Use ground speed", NULL);
	gui_config_entry_create(list, &profile.flight.auto_landing.speed_value, "Be slower than", &spd_param);
	gui_config_entry_create(list, &profile.flight.auto_landing.timeout, "Within time", &time_param);

	return list;
}



