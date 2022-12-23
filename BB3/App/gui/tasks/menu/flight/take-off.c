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
    help_set_base("Flight/Takeoff");

	lv_obj_t * list = gui_list_create(par, _("Automatic Take-off"), &gui_flight, NULL);

	gui_list_auto_entry(list, _h("Use altitude"), &profile.flight.auto_take_off.alt_change_enabled, NULL);
	gui_list_auto_entry(list, _h("Change greater than"), &profile.flight.auto_take_off.alt_change_value, &alt_param);
	gui_list_auto_entry(list, _h("Use ground speed"), &profile.flight.auto_take_off.speed_enabled, NULL);
	gui_list_auto_entry(list, _h("Exceed speed"), &profile.flight.auto_take_off.speed_value, &spd_param);
	gui_list_auto_entry(list, _h("Within time"), &profile.flight.auto_take_off.timeout, &time_param);

	return list;
}



