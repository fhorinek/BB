/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"
#include "fc/agl.h"

REGISTER_WIDGET_IU
(
    TTime,
    "Thermal time",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide),

    lv_obj_t * value;
);


static void TTime_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "Thermal time");

    local->value = widget_add_value(base, slot, NULL, NULL);
}

static void TTime_update(widget_slot_t * slot)
{
    char value[16];

    uint8_t min = fc.flight.circling_time / 60;
    uint8_t sec = fc.flight.circling_time % 60;

    if (min == 0)
        snprintf(value, sizeof(value), "%u", sec);
    else
        snprintf(value, sizeof(value), "%u:%02u", min, sec);

    lv_label_set_text(local->value, value);
    widget_update_font_size(local->value);
}


