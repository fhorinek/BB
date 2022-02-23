/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    GSpeed,
    "Speed - ground",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_units_hide) | _b(wf_decimal_precision),

    lv_obj_t * value;
);

static void GSpeed_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "GSpeed");

    char tmp[8];
    char * units = tmp;
    if (widget_flag_is_set(slot, wf_units_hide))
    	units = NULL;
    else
    	format_speed_units(tmp);

    local->value = widget_add_value(base, slot, units, NULL);

}

static void GSpeed_update(widget_slot_t * slot)
{
    char value[8];
    if (fc.gnss.fix == 0)
    {
    	strcpy(value, "No\nGNSS");
    }
    else
    {
        char * format = widget_flag_is_set(slot, wf_decimal_precision) ? "%0.1f" : "%0.0f";
        format_speed_2(value, fc.gnss.ground_speed, format);
    }

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value);
}


