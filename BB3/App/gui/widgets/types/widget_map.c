/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"
#include "gui/map/map_thread.h"

REGISTER_WIDGET_IU(Map,
    "Map",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * image;
    uint8_t magic;
);


static void Map_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);

    local->image = lv_canvas_create(slot->obj, NULL);
    local->magic = gui.map.magic - 1;

    lv_obj_set_size(local->image, slot->w, slot->h);
}

static void Map_update(widget_slot_t * slot)
{
    if (local->magic != gui.map.magic)
    {
        lv_canvas_set_buffer(local->image, gui.map.buffer[gui.map.active_buffer], MAP_W, MAP_H, LV_IMG_CF_TRUE_COLOR);
        local->magic = gui.map.magic;
    }
}
