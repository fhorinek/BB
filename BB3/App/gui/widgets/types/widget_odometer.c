/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"
#include "fc/agl.h"
#include "gui/statusbar.h"

REGISTER_WIDGET_IUE
(
    Odo,
    "Odometer",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
);


static void Odo_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, NULL);

    local->value = widget_add_value(base, slot, NULL, NULL);

}

static void Odo_edit(widget_slot_t * slot, uint8_t action)
{
    if (action == WIDGET_ACTION_HOLD)
    {
    	if (fc.flight.odometer != 0)
    	{
			fc.flight.odometer = 0;
			statusbar_add_msg(STATUSBAR_MSG_INFO, "Odometer reset");
    	}
    }
}

static void Odo_update(widget_slot_t * slot)
{
    char value[16];

    format_distance_with_units(value, fc.flight.odometer);
    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value, slot->obj);
}


