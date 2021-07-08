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
	.disp_multi = 1,
	.step = 0.1,
	.format = format_percent,
};

static lv_obj_t * vario_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Vario settings", &gui_settings, NULL);

	gui_config_entry_create(list, &profile.vario.in_flight, "Audio only in flight", NULL);
	gui_config_entry_create(list, &profile.vario.acc_gain, "Accelerometer gain", &acc_opt);
	gui_config_entry_create(list, &profile.vario.lift, "Lift threshold", &sink_lift_opt);
	gui_config_entry_create(list, &profile.vario.sink, "Sink threshold", &sink_lift_opt);

	return list;
}



