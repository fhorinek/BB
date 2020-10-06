#include "sensors.h"

#include "settings.h"

#include "../gui_list.h"

#include "../../config/config.h"
#include "../../fc/fc.h"
#include "../../etc/format.h"

REGISTER_TASK_IL(sensors,
	lv_obj_t * gnss_status;
	lv_obj_t * ms5611_status;
);

void sensors_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_settings, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
}

lv_obj_t * sensors_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Sensors", sensors_cb);

	local->gnss_status = gui_list_info_add_entry(list, "GNSS", "");
	local->ms5611_status = gui_list_info_add_entry(list, "MS5611", "");

	return list;
}

void sensors_loop()
{
	char value[32];

	snprintf(value, sizeof(value), "%0.1f Pa", fc.vario.pressure);
	gui_list_info_set_value(local->ms5611_status, value);
}
