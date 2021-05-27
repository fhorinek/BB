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
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * arrow;
    lv_point_t points[WIDGET_ARROW_POINTS];
);


static void CompArrow_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    widget_add_title(base, slot, "Compass");

    local->arrow = widget_add_arrow(base, slot, local->points, NULL, NULL);
    widget_arrow_rotate(local->arrow, local->points, 0);
}

static void CompArrow_update(widget_slot_t * slot)
{
    widget_arrow_rotate(local->arrow, local->points, -fc.fused.azimuth);
}


