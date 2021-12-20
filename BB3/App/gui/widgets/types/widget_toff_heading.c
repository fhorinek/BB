/*
 * widget_toff_heading.c
 *
 *  Created on: 10. 12. 2021
 *      Author: thrull
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
	HdgToff,
    "Direction to take off",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide),

    lv_obj_t * arrow;
	lv_obj_t * text;

    lv_point_t points[WIDGET_ARROW_POINTS];
);

static void HdgToff_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "Dir TO");

    local->text = widget_add_value(base, slot, NULL, NULL);
    local->arrow = widget_add_arrow(base, slot, local->points, NULL, NULL);
}

static void HdgToff_update(widget_slot_t * slot)
{
	if (fc.gnss.fix == 0)
	{
		lv_label_set_text(local->text, "No\nGNSS");
		widget_update_font_size(local->text);
		lv_obj_set_hidden(local->arrow, true);
		lv_obj_set_hidden(local->text, false);
	}
	else
	{
		widget_arrow_rotate(local->arrow, local->points, (fc.flight.toff_bearing - fc.gnss.heading));
		lv_obj_set_hidden(local->arrow, false);
		lv_obj_set_hidden(local->text, true);
	}
}
