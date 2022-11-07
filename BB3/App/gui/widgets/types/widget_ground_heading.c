/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_IU
(
    GHdg,
    "Heading - ground",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide),

    lv_obj_t * value;
);

static void GHdg_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, _("GHdg"));

    local->value = widget_add_value(base, slot, NULL, NULL);

}

static void GHdg_update(widget_slot_t * slot)
{
    char value[8];
    if (fc.gnss.fix == 0)
    {
    	strcpy(value, _("No\nGNSS"));
    	lv_label_set_text(local->value, value);
    }
    else
    {
		snprintf(value, sizeof(value), "%u", (uint16_t)fc.gnss.heading);
		lv_label_set_text(local->value, value);
    }
    widget_update_font_size(local->value);
}


