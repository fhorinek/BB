#include "vario.h"

#include "gui/tasks/menu/settings.h"

#include "gui/gui_list.h"
#include "gui/keyboard.h"

#include "etc/format.h"

REGISTER_TASK_I(vario);

static gui_list_slider_options_t sink_lift_opt = {
	.disp_multi = 0.1,
	.step = 1,
	.format = format_vario_with_units,
};

static gui_list_slider_options_t acc_opt = {
	.disp_multi = 100,
	.step = 0.1,
	.format = format_percent,
};

static gui_list_slider_options_t avg_opt = {
	.disp_multi = 1,
	.step = 1,
	.format = format_duration,
};

static lv_obj_t * vario_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Vario settings", &gui_settings, NULL);

	gui_list_auto_entry(list, "Audio only in flight", &profile.vario.in_flight, NULL);
	gui_list_auto_entry(list, "Accelerometer gain", &profile.vario.acc_gain, &acc_opt);
	gui_list_auto_entry(list, "Lift threshold", &profile.vario.lift, &sink_lift_opt);
	gui_list_auto_entry(list, "Sink threshold", &profile.vario.sink, &sink_lift_opt);
	gui_list_auto_entry(list, "Average time", &profile.vario.avg_duration, &avg_opt);

	return list;
}



