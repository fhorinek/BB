/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
   WindDirPoints,
    "Wind - direction points",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide),

    lv_obj_t * value;
);

static void WindDirPoints_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "Wind dir");

    local->value = widget_add_value(base, slot, NULL, NULL);

}

static void WindDirPoints_update(widget_slot_t * slot)
{
    char value[8];
    if (fc.gnss.fix == 0)
    {
    	strcpy(value, "No\nGNSS");
    }
    else if (!fc.wind.valid)
    {
        strcpy(value, "---");
    }
    else
    {
		format_hdg_to_points(value, fc.wind.direction);
    }
    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value);
}


