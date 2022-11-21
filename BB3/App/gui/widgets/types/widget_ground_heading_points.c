/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    GHdgPoints,
    "Heading - ground points",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide),

    lv_obj_t * value;
);

static void GHdgPoints_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, _("GHdg"));

    local->value = widget_add_value(base, slot, NULL, NULL);

}

static void GHdgPoints_update(widget_slot_t * slot)
{
    char value[32];
    if (fc.gnss.fix == 0)
    {
    	strcpy(value, _("No\nGNSS"));
    	lv_label_set_text(local->value, value);
    }
    else
    {
		format_hdg_to_points(value, fc.gnss.heading);
		lv_label_set_text(local->value, value);
    }
    widget_update_font_size(local->value);
}


