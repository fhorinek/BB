/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IUE
(
    Alt1,
    "Altitude QNH1",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
    lv_obj_t * edit;
);


static void Alt1_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, "Altitude 1");

    char units[8];
    format_altitude_units(units);
    local->value = widget_add_value(base, slot, units, NULL);

    local->edit = NULL;
}

static void Alt1_edit(widget_slot_t * slot, uint8_t action)
{
    if (action == WIDGET_ACTION_DEFOCUS)
    {
        if (local->edit != NULL)
        {
            widget_destroy_edit_overlay(local->edit);
            local->edit = NULL;
        }

        return;
    }


    if (local->edit == NULL)
    {
        local->edit = widget_create_edit_overlay("Altitude 1", "Set QNH1");

    }
}

static void Alt1_update(widget_slot_t * slot)
{
    if (fc.fused.status == fc_dev_ready)
    {
        char value[10];

        format_altitude(value, fc.fused.altitude1);
        lv_label_set_text(local->value, value);
    }
    else
    {
        lv_label_set_text(local->value, "---");
    }
    widget_update_font_size(local->value, slot->obj);
}


