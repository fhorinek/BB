/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    Avg,
    "Average vario",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
);


static void Avg_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    char title[16];
    sprintf(title, "Avg (%us)", config_get_int(&profile.vario.avg_duration));
    widget_add_title(base, slot, title);

    char units[8];
    format_vario_units(units);
    local->value = widget_add_value(base, slot, units, NULL);

}

static void Avg_update(widget_slot_t * slot)
{
    char value[8];

    if (fc.fused.status != fc_dev_ready)
        strcpy(value, "---");
    else
        format_vario(value, fc.fused.avg_vario);

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value, slot->obj);
}


