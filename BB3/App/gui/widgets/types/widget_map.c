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
	lv_obj_t * arrow;
	lv_obj_t * dot;

    lv_point_t points[WIDGET_ARROW_POINTS];

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

    local->dot = lv_obj_create(slot->obj, NULL);
    lv_obj_set_size(local->dot, 10, 10);
    lv_obj_align(local->dot, slot->obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_PURPLE);
    lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);

    local->arrow = widget_add_arrow(base, slot, local->points, NULL, NULL);
    lv_obj_set_style_local_line_color(local->arrow, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_PURPLE);
    lv_obj_set_style_local_line_width(local->arrow, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 4);
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
    	{
    		continue;
    	}

    	int32_t disp_lat;
    	int32_t disp_lon;

        if (fc.gnss.fix == 0)
    	{
        	disp_lat = config_get_big_int(&profile.ui.last_lat);
        	disp_lon = config_get_big_int(&profile.ui.last_lon);
    	}
        else
        {
        	disp_lat = fc.gnss.latitude;
        	disp_lon = fc.gnss.longtitude;
        }

		int16_t x, y;
		tile_geo_to_pix(i, disp_lon, disp_lat, &x, &y);

		x -= slot->w / 2;
		y -= slot->h / 2;

		lv_obj_set_pos(local->image[i], -x, -y);
    }

    if (fc.gnss.fix == 0)
    {
    	lv_obj_set_hidden(local->arrow, true);
    	lv_obj_set_hidden(local->dot, true);
    }
    else
    {
    	if (fc.gnss.ground_speed > 2)
		{
			widget_arrow_rotate_size(local->arrow, local->points, -fc.gnss.heading, 40);
			lv_obj_set_hidden(local->arrow, false);
			lv_obj_set_hidden(local->dot, true);
    	}
    	else
    	{
			lv_obj_set_hidden(local->arrow, true);
			lv_obj_set_hidden(local->dot, false);
    	}
    }

}
