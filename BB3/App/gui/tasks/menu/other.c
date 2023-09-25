#include "gui/gui_list.h"

#include "settings.h"
#include "other.h"
#include "drivers/power/pwr_mng.h"

REGISTER_TASK_IL(other,
    lv_obj_t * ext_charger;
);

static bool other_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
    UNUSED(index);

    if (event == LV_EVENT_VALUE_CHANGED)
    {
        if (obj == local->ext_charger)
        {
            bool val = gui_list_switch_get_value(local->ext_charger);
            pwr.charge_from_strato = val;
        }
    }
    return true;
}


void other_loop()
{
    bool val = gui_list_switch_get_value(local->ext_charger);

    if (val != pwr.charge_from_strato)
        gui_list_switch_set_value(local->ext_charger, pwr.charge_from_strato);
}

lv_obj_t * other_init(lv_obj_t * par)
{
    help_set_base("Other");

    lv_obj_t * list = gui_list_create(par, _("Other"), &gui_settings, other_cb);

    local->ext_charger = gui_list_switch_add_entry(list, _h("Enable charging"), pwr.charge_from_strato);


    return list;
}
