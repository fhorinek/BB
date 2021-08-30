/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"
#include "fc/agl.h"

REGISTER_WIDGET_IU
(
    Agl,
    "Height above ground",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
);


static void Agl_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, NULL);

    char units[8];
    format_altitude_units(units);
    local->value = widget_add_value(base, slot, units, NULL);

}

static void Agl_update(widget_slot_t * slot)
{
    char value[16];

    if (fc.agl.agl == AGL_INVALID)
        strcpy(value, "No TOPO\ndata");
    else
        format_altitude(value, fc.agl.agl);

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value, slot->obj);
}


