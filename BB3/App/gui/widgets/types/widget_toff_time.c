/*
 * widget_toff_time.c
 *
 *  Created on: 02. 02. 2022
 *      Author: tilmann@bubecks.de
 */

#include "gui/widgets/widget.h"
#include "fc/agl.h"
#include "gui/statusbar.h"

REGISTER_WIDGET_IU
(
    TimeToTakeoff,
    "Takeoff - time to arrive",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_units_hide),

    lv_obj_t * value;
	lv_obj_t * units;
);


static void TimeToTakeoff_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "TO time");

    if (widget_flag_is_set(slot, wf_units_hide))
    {
    	local->value = widget_add_value(base, slot, NULL, NULL);
    	local->units = NULL;
    }
    else
    	local->value = widget_add_value(base, slot, "", &local->units);
}

static void TimeToTakeoff_update(widget_slot_t * slot)
{
    char value[16];
    char *unit = "";

    if (fc.flight.takeoff_distance != INVALID_UINT32)
    {
    	//this must be GPS heading not compass, since we have to move towards the target, not just pointing to it!
    	int16_t relative_direction = fc.flight.takeoff_bearing - fc.gnss.heading;
    	if (abs(relative_direction) < 45)
    	{
    		// Pilot is heading towards home.
    		// distance is in m, ground_speed in m/s. This gives seconds.
    		uint32_t sec = fc.flight.takeoff_distance / fc.gnss.ground_speed;
    		sprintf(value, "%.1f", sec / 60.0);
    		unit = "min";
    	}
    	else
    	{
    		strcpy(value, "Turn to\ntakeoff");
    	}
    }
    else
    {
    	strcpy(value, "No\nstart\npos");
    }

    lv_label_set_text(local->value, value);
    if ( local->units != NULL )
    	lv_label_set_text(local->units, unit);

    widget_update_font_size(local->value);
}
