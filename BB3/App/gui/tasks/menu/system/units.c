#include "gui/gui_list.h"

#include "system.h"

REGISTER_TASK_I(units);

lv_obj_t * units_init(lv_obj_t * par)
{
    help_set_base("System/Units");

    lv_obj_t * list = gui_list_create(par, _("Units"), &gui_system, NULL);

    gui_list_auto_entry(list, _("Altitude"), &config.units.altitude, NULL);
    gui_list_auto_entry(list, _("Speed"), &config.units.speed, NULL);
    gui_list_auto_entry(list, _("Distance"), &config.units.distance, NULL);
    gui_list_auto_entry(list, _("Variometer"), &config.units.vario, NULL);
    gui_list_auto_entry(list, _("Date format"), &config.units.date, NULL);
    gui_list_auto_entry(list, _("Use 24h time"), &config.units.time24, NULL);
    gui_list_auto_entry(list, _("Position"), &config.units.geo_datum, NULL);
    gui_list_auto_entry(list, _("Earth model"), &config.units.earth_model, NULL);
    gui_list_auto_entry(list, _("GNSS Altitude"), &config.units.galt, NULL);

    return list;
}
