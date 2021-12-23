/*
 * widget_toff_dist.c
 *
 *  Created on: 7. 12. 2021
 *      Author: thrull
 */

#include "gui/widgets/widget.h"
#include "fc/agl.h"
#include "gui/statusbar.h"

REGISTER_WIDGET_IU
(
    DToff,
    "Distance from take off",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_units_hide),

    lv_obj_t * value;
	lv_obj_t * units;
);


static void DToff_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "TO distance");

    if (widget_flag_is_set(slot, wf_units_hide))
    {
    	local->value = widget_add_value(base, slot, NULL, NULL);
    	local->units = NULL;
    }
    else
    	local->value = widget_add_value(base, slot, "", &local->units);

}

static void DToff_update(widget_slot_t * slot)
{
    char value[16];

    if (fc.flight.takeoff_distance != INVALID_UINT32)
    {
		format_distance(value, fc.flight.takeoff_distance);

		if (local->units != NULL)
		{
			format_distance_units(value, fc.flight.takeoff_bearing);
			lv_label_set_text(local->units, value);
		}
    }
    else
    {
    	strcpy(value, "No\nstart");

    	if (local->units != NULL)
    		lv_label_set_text(local->units, "");
    }

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value);
}
