#include "flight.h"

#include "gui/tasks/menu/settings.h"

#include "take-off.h"
#include "landing.h"

#include "gui/gui_list.h"

REGISTER_TASK_I(flight);

static lv_obj_t * flight_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Flight settings", &gui_settings, NULL);

	gui_list_auto_entry(list, "Automatic Take-off", NEXT_TASK, &gui_take_off);
	gui_list_auto_entry(list, "Automatic Landing", NEXT_TASK, &gui_landing);

	return list;
}



