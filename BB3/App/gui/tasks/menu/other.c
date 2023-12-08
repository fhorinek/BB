#include "gui/gui_list.h"

#include "settings.h"
#include "other.h"
#include "drivers/power/pwr_mng.h"

REGISTER_TASK_IL(other,
    lv_obj_t * ext_charger;
    lv_obj_t * charger_debug;
    lv_obj_t * bat_volt;
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

    char * mode;
    if (pwr.data_usb_mode == dm_client)
        mode = "Client";
    else if (pwr.data_usb_mode == dm_host_boost)
        mode = "Boost";
    else
        mode = "Pass";

    char value[64];
    snprintf(value, sizeof(value), "%s %0.2fV", mode, 4.55 + pwr.boost_volt * 0.064);
    gui_list_info_set_value(local->charger_debug, value);

    snprintf(value, sizeof(value), "%u%% %0.2fV %dmA",
            pwr.fuel_gauge.battery_percentage,
            pwr.fuel_gauge.bat_voltage / 100.0, pwr.fuel_gauge.bat_current);
    gui_list_info_set_value(local->bat_volt, value);
}

lv_obj_t * other_init(lv_obj_t * par)
{
    help_set_base("Other");

    lv_obj_t * list = gui_list_create(par, _("Other"), &gui_settings, other_cb);

    local->ext_charger = gui_list_switch_add_entry(list, _h("Enable charging"), pwr.charge_from_strato);
    local->charger_debug = gui_list_info_add_entry(list, _h("Charger info"), "");
    local->bat_volt = gui_list_info_add_entry(list, "Battery gauge", 0, "");


    return list;
}
