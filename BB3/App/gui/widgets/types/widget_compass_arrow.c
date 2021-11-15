/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    CompArrow,
    "Compass arrow",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide),

    lv_obj_t * arrow;
	lv_obj_t * text;
    lv_point_t points[WIDGET_ARROW_POINTS];
);


static void CompArrow_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "Compass");

    local->text = widget_add_value(base, slot, NULL, NULL);
    local->arrow = widget_add_arrow(base, slot, local->points, NULL, NULL);
    widget_arrow_rotate(local->arrow, local->points, 0);
}

static void CompArrow_update(widget_slot_t * slot)
{
	if (fc.imu.status == fc_device_not_calibrated)
	{
		lv_label_set_text(local->text, "Need\nCalib.");
		widget_update_font_size(local->text);
		lv_obj_set_hidden(local->arrow, true);
		lv_obj_set_hidden(local->text, false);
	}
	else
	{
		widget_arrow_rotate(local->arrow, local->points, fc.fused.azimuth);
		lv_obj_set_hidden(local->arrow, false);
		lv_obj_set_hidden(local->text, true);
	}
}


