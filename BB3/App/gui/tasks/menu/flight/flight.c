#include "flight.h"

#include "gui/tasks/menu/settings.h"

#include "take-off.h"
#include "landing.h"

#include "gui/gui_list.h"

REGISTER_TASK_I(flight);

static lv_obj_t * flight_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Flight settings", &gui_settings, NULL);

	gui_config_entry_create(list, NULL, "Automatic Take-off", &gui_take_off);
	gui_config_entry_create(list, NULL, "Automatic Landing", &gui_landing);

	return list;
}



