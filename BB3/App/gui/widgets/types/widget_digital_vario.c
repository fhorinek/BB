/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    Vario,
    "Digital vario",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
);

static void Vario_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, NULL);

    char units[8];
    format_vario_units(units);
    local->value = widget_add_value(base, slot, units, NULL);

}

static void Vario_update(widget_slot_t * slot)
{
    char value[8];
    if (fc.fused.status != fc_dev_ready)
        strcpy(value, "---");
    else
        format_vario(value, fc.fused.vario);

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value, slot->obj);
}


