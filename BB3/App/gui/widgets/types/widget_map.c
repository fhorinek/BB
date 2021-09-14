/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG
#include "gui/widgets/widget.h"
#include "gui/map/map_thread.h"
#include "gui/map/tile.h"

REGISTER_WIDGET_IU(Map,
    "Map",
    WIDGET_VAL_MIN_W,
    WIDGET_VAL_MIN_H,

    lv_obj_t * image[9];
    uint8_t magic;
);


static void Map_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);

    local->magic = gui.map.magic - 1;

    for (uint8_t i = 0; i < 9; i++)
    {
		local->image[i] = lv_canvas_create(slot->obj, NULL);
		lv_obj_set_size(local->image[i], MAP_W, MAP_H);

		while(gui.map.chunks[i].buffer == NULL);
		lv_canvas_set_buffer(local->image[i], gui.map.chunks[i].buffer, MAP_W, MAP_H, LV_IMG_CF_TRUE_COLOR);
		lv_obj_set_hidden(local->image[i], true);
	}

    lv_obj_t * pos = lv_obj_create(slot->obj, NULL);
    lv_obj_set_size(pos, 10, 10);
    lv_obj_align(pos, slot->obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(pos, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_PURPLE);
}

static void Map_update(widget_slot_t * slot)
{
    if (local->magic != gui.map.magic)
    {
//    	DBG("Widget set to index %u", gui.map.disp_buffer);
        for (uint8_t i = 0; i < 9; i++)
        {
        	lv_obj_set_hidden(local->image[i], !gui.map.chunks[i].ready);
        }
        local->magic = gui.map.magic;
    }

    for (uint8_t i = 0; i < 9; i++)
    {
    	if (!gui.map.chunks[i].ready)
    		continue;

		int16_t x, y;
		tile_geo_to_pix(i, fc.gnss.longtitude, fc.gnss.latitude, &x, &y);

		x -= slot->w / 2;
		y -= slot->h / 2;

		lv_obj_set_pos(local->image[i], -x, -y);
    }

}
