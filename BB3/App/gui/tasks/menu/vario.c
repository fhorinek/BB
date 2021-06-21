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

static void update_sliders()
{
	char val[32];
	char units[16];

	snprintf(val, sizeof(val), "%u%%", gui_list_slider_get_value(local->acc_gain) * 10);
	gui_list_slider_set_label(local->acc_gain, val);

	format_vario_units(units);

	format_vario(val, (float)config_get_int(&profile.vario.lift) / 10.0);
	sprintf(val + strlen(val), " %s", units);

	gui_list_slider_set_label(local->lift, val);

	format_vario(val, (float)config_get_int(&profile.vario.sink) / 10.0);
	sprintf(val + strlen(val), " %s", units);
	gui_list_slider_set_label(local->sink, val);

}


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
			config_set_float(&profile.vario.acc_gain, gui_list_slider_get_value(local->acc_gain) / 10.0);
			update_sliders();
		}

		if (obj == local->in_flight)
			config_set_bool(&profile.vario.in_flight, gui_list_switch_get_value(local->in_flight));

		if (obj == local->lift)
		{
			config_set_int(&profile.vario.lift, gui_list_slider_get_value(local->lift));
			update_sliders();
		}

		if (obj == local->sink)
		{
			config_set_int(&profile.vario.sink, gui_list_slider_get_value(local->sink));
			update_sliders();
		}
	}


}


static lv_obj_t * vario_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Vario settings", vario_cb);

	local->in_flight = gui_list_switch_add_entry(list, "Audio only in flight", config_get_bool(&profile.vario.in_flight));
	local->acc_gain = gui_list_slider_add_entry(list, "Accelerometer gain", 0, 10, config_get_float(&profile.vario.acc_gain) * 10);

	local->lift = gui_list_slider_add_entry(list, "Lift threshold", -100, 100, config_get_int(&profile.vario.lift));
	local->sink = gui_list_slider_add_entry(list, "Sink threshold", -100, 100, config_get_int(&profile.vario.sink));
	update_sliders();

	return list;
}



