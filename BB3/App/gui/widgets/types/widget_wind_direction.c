/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    WindDir,
    "Wind - direction",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_no_rotate),

    lv_obj_t * arrow;
	lv_obj_t * text;

    lv_point_t points[WIDGET_ARROW_POINTS];
);


static void WindDir_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, _("Wind dir"));

    local->text = widget_add_value(base, slot, NULL, NULL);
    local->arrow = widget_add_arrow(base, slot, local->points, NULL, NULL);
}

static void WindDir_update(widget_slot_t * slot)
{
	if (fc.gnss.fix == 0)
	{
		lv_label_set_text(local->text, _("No\nGNSS"));
		widget_update_font_size(local->text);
		lv_obj_set_hidden(local->arrow, true);
		lv_obj_set_hidden(local->text, false);
	}
	else if (!fc.wind.valid)
	{
        lv_label_set_text(local->text, "---");
        widget_update_font_size(local->text);
        lv_obj_set_hidden(local->arrow, true);
        lv_obj_set_hidden(local->text, false);
	}
	else
	{
	    int16_t angle = widget_flag_is_set(slot, wf_no_rotate) ? fc.wind.direction + 180 : fc.wind.direction + 180 - fc.gnss.heading;
		widget_arrow_rotate(local->arrow, local->points, angle);
		lv_obj_set_hidden(local->arrow, false);
		lv_obj_set_hidden(local->text, true);
	}
}


