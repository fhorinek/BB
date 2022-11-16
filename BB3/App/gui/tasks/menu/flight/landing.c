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
    help_set_base("Flight/Landing");

	lv_obj_t * list = gui_list_create(par, _("Automatic Landing"), &gui_flight, NULL);

	gui_list_auto_entry(list, _("Use altitude"), &profile.flight.auto_landing.alt_change_enabled, NULL);
	gui_list_auto_entry(list, _("Stay within"), &profile.flight.auto_landing.alt_change_value, &alt_param);
	gui_list_auto_entry(list, _("Use ground speed"), &profile.flight.auto_landing.speed_enabled, NULL);
	gui_list_auto_entry(list, _("Be slower than"), &profile.flight.auto_landing.speed_value, &spd_param);
	gui_list_auto_entry(list, _("Within time"), &profile.flight.auto_landing.timeout, &time_param);

	return list;
}



