/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "widget.h"

typedef struct
{
	lv_obj_t * value;
}
widget_local_vars_t;

void widget_flight_time_init(lv_obj_t * base, widget_slot_t * slot, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	widget_title(base, slot, x, y, w, h);

	local->value = lv_label_create(slot->obj, NULL);
	lv_label_set_text(local->value, "00:27");
}

widget_t widget_flight_time =
{
		"Flight Time",
		"FTime",
		widget_flight_time_init,
		NULL,
		NULL,
		NULL,
		sizeof(widget_local_vars_t)
};
