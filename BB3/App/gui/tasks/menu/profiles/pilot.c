#include "pilot.h"

#include "gui/tasks/menu/profiles/profiles.h"
#include "gui/gui_list.h"

REGISTER_TASK_I(pilot);

static lv_obj_t * pilot_init(lv_obj_t * par)
{
    help_set_base("Pilot");

	lv_obj_t * list = gui_list_create(par, _("Pilot"), &gui_profiles, NULL);

    gui_list_auto_entry(list, _h("Pilot name"), &pilot.name, NULL);
    gui_list_auto_entry(list, _h("Glider type"), &pilot.glider_type, NULL);
    gui_list_auto_entry(list, _h("Glider ID"), &pilot.glider_id, NULL);
    gui_list_auto_entry(list, _h("Broadcast name"), &pilot.broadcast_name, NULL);
    gui_list_auto_entry(list, _h("Track online"), &pilot.online_track, NULL);

    return list;
}



