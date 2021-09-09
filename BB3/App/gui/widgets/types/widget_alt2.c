/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_ISUE
(
    Alt2,
    "Altitude QNH2",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * value;
	lv_obj_t * edit;
	uint32_t last_action;
	uint8_t action_cnt;
);

#include "widget_alt.h"

static void Alt2_init(lv_obj_t * base, widget_slot_t * slot)
{
	Alt_init(local, base, slot, "Altitude 2");
}

static void Alt2_edit(widget_slot_t * slot, uint8_t action)
{
	Alt_edit(local, slot, action, "Altitude 2", &fc.fused.altitude2, &config.vario.qnh2);
}

static void Alt2_update(widget_slot_t * slot)
{
	Alt_update(local, slot, fc.fused.altitude2, config_get_big_int(&config.vario.qnh2));
}

static void Alt2_stop(widget_slot_t * slot)
{
	Alt_stop(local, slot);
}
