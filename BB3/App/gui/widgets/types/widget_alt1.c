/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

REGISTER_WIDGET_ISUE
(
    Alt1,
    _i("Altitude - QNH1"),
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_units_hide) | _b(wf_alt_unit),

    lv_obj_t * value;
	lv_obj_t * edit;
	uint32_t last_action;
	uint8_t action_cnt;
);

#include "widget_alt.h"

static void Alt1_init(lv_obj_t * base, widget_slot_t * slot)
{
	Alt_init(local, base, slot, "Altitude 1");
}

static void Alt1_edit(widget_slot_t * slot, uint8_t action)
{
	Alt_edit(local, slot, action, "Altitude 1", &fc.fused.altitude1, &config.vario.qnh1);
}

static void Alt1_update(widget_slot_t * slot)
{
	Alt_update(local, slot, fc.fused.altitude1, config_get_big_int(&config.vario.qnh1));
}

static void Alt1_stop(widget_slot_t * slot)
{
	Alt_stop(local, slot);
}
