#include "vario.h"

#include "gui/tasks/menu/settings.h"

#include "gui/gui_list.h"
#include "gui/keyboard.h"

REGISTER_TASK_I(vario,
	lv_obj_t * in_flight;
	lv_obj_t * acc_gain;
	lv_obj_t * lift;
	lv_obj_t * sink;
);

//this is menu template

static void vario_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	//back
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

	//switch / slider
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		if (obj == local->acc_gain)
			config_set_float(&profile.vario.acc_gain, gui_list_slider_get_value(local->acc_gain) / 10.0);

		if (obj == local->in_flight)
			config_set_bool(&profile.vario.in_flight, gui_list_switch_get_value(local->in_flight));
	}


}

static lv_obj_t * vario_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Vario", vario_cb);

	local->in_flight = gui_list_switch_add_entry(list, "Only in flight", config_get_bool(&profile.vario.in_flight));
	local->acc_gain = gui_list_slider_add_entry(list, "Accelerometer gain", 0, 10, config_get_float(&profile.vario.acc_gain) * 10);
	local->lift = gui_list_info_add_entry(list, "Lift threshold", "0.0m/s");
	local->sink = gui_list_info_add_entry(list, "Sink threshold", "0.0m/s");

	return list;
}



