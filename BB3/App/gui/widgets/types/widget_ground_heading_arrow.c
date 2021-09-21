/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    GHdgArrow,
    "Ground heading arrow",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * arrow;
	lv_obj_t * text;

    lv_point_t points[WIDGET_ARROW_POINTS];
);


static void GHdgArrow_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, "GHdg");

    local->text = widget_add_value(base, slot, NULL, NULL);
    local->arrow = widget_add_arrow(base, slot, local->points, NULL, NULL);
}

static void GHdgArrow_update(widget_slot_t * slot)
{
	if (fc.gnss.fix != 3)
	{
		lv_label_set_text(local->text, "No\nGNSS");
		widget_update_font_size(local->text, slot->obj);
		lv_obj_set_hidden(local->arrow, true);
		lv_obj_set_hidden(local->text, false);
	}
	else
	{
		widget_arrow_rotate(local->arrow, local->points, -fc.gnss.heading);
		lv_obj_set_hidden(local->arrow, false);
		lv_obj_set_hidden(local->text, true);
	}
}


