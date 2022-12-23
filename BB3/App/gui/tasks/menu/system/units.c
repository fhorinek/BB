#include "gui/gui_list.h"

#include "system.h"

REGISTER_TASK_I(units);

lv_obj_t * units_init(lv_obj_t * par)
{
    help_set_base("System/Units");

    lv_obj_t * list = gui_list_create(par, _("Units"), &gui_system, NULL);

    gui_list_auto_entry(list, _h("Altitude"), &config.units.altitude, NULL);
    gui_list_auto_entry(list, _h("Speed"), &config.units.speed, NULL);
    gui_list_auto_entry(list, _h("Distance"), &config.units.distance, NULL);
    gui_list_auto_entry(list, _h("Variometer"), &config.units.vario, NULL);
    gui_list_auto_entry(list, _h("Date format"), &config.units.date, NULL);
    gui_list_auto_entry(list, _h("Use 24h time"), &config.units.time24, NULL);
    gui_list_auto_entry(list, _h("Position"), &config.units.geo_datum, NULL);
    gui_list_auto_entry(list, _h("Earth model"), &config.units.earth_model, NULL);
    gui_list_auto_entry(list, _h("GNSS Altitude"), &config.units.galt, NULL);

    return list;
}
