#include "vario.h"

#include "gui/tasks/menu/settings.h"

#include "gui/gui_list.h"
#include "gui/keyboard.h"

#include "etc/format.h"

REGISTER_TASK_I(vario,
	lv_obj_t * in_flight;
	lv_obj_t * acc_gain;
	lv_obj_t * lift;
	lv_obj_t * sink;
);

gui_list_slider_options_t sink_lift_opt = {
	.disp_multi = 0.1,
	.step = 1,
	.format = format_vario_with_units,
};

gui_list_slider_options_t acc_opt = {
	.disp_multi = 1,
	.step = 0.1,
	.format = format_percent,
};

static void vario_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	//back
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

	//switch / slider
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		if (obj == local->acc_gain)
		{
			gui_config_entry_update(obj, &profile.vario.acc_gain, &acc_opt);
		}

		if (obj == local->in_flight)
		{
			gui_config_entry_update(obj, &profile.vario.in_flight, NULL);
		}

		if (obj == local->lift)
		{
			gui_config_entry_update(obj, &profile.vario.lift, &sink_lift_opt);
		}

		if (obj == local->sink)
		{
			gui_config_entry_update(obj, &profile.vario.sink, &sink_lift_opt);
		}
	}
}



static lv_obj_t * vario_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Vario settings", vario_cb);

	local->in_flight = gui_config_entry_create(list, &profile.vario.in_flight, "Audio only in flight", NULL);
	local->acc_gain = gui_config_entry_create(list, &profile.vario.acc_gain, "Accelerometer gain", &acc_opt);
	local->lift = gui_config_entry_create(list, &profile.vario.lift, "Lift threshold", &sink_lift_opt);
	local->sink = gui_config_entry_create(list, &profile.vario.sink, "Sink threshold", &sink_lift_opt);

	return list;
}



