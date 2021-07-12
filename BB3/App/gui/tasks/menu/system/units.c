#include "gui/gui_list.h"

#include "system.h"

REGISTER_TASK_I(units);

lv_obj_t * units_init(lv_obj_t * par)
{
    lv_obj_t * list = gui_list_create(par, "Units", &gui_system, NULL);

    gui_list_auto_entry(list, "Altitude", &config.units.altitude, NULL);
    gui_list_auto_entry(list, "Speed", &config.units.speed, NULL);
    gui_list_auto_entry(list, "Distance", &config.units.distance, NULL);
    gui_list_auto_entry(list, "Variometer", &config.units.vario, NULL);
    gui_list_auto_entry(list, "Datum format", &config.units.date, NULL);
    gui_list_auto_entry(list, "Use 24h time", &config.units.time24, NULL);
    gui_list_auto_entry(list, "Position", &config.units.geo_datum, NULL);
    gui_list_auto_entry(list, "Earth model", &config.units.earth_model, NULL);

    return list;
}
