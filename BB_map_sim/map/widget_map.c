/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "tile.h"

#include "etc/geo_calc.h"

typedef struct
{
    lv_obj_t * image[9];
	lv_obj_t * dot;

    uint8_t magic;
} local_t;

local_t data;
local_t * local = &data;

void widget_map_init(lv_obj_t * base)
{

    local->magic = gui.map.magic - 1;

    for (uint8_t i = 0; i < 9; i++)
    {
		local->image[i] = lv_canvas_create(base, NULL);
		lv_obj_set_size(local->image[i], MAP_W, MAP_H);

		lv_canvas_set_buffer(local->image[i], gui.map.chunks[i].buffer, MAP_W, MAP_H, LV_IMG_CF_TRUE_COLOR);
		lv_obj_set_hidden(local->image[i], true);
	}

    local->dot = lv_obj_create(base, NULL);
    lv_obj_set_size(local->dot, 10, 10);
    lv_obj_align(local->dot, base, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_PURPLE);
    lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);

}

void widget_map_update(lv_obj_t * base, int32_t disp_lat, int32_t disp_lon)
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



		int16_t x, y;
		tile_geo_to_pix(i, disp_lon, disp_lat, &x, &y);

		int16_t w = lv_obj_get_width(base);
		int16_t h = lv_obj_get_height(base);

		x -= w / 2;
		y -= h / 2;

		lv_obj_set_pos(local->image[i], -x, -y);
    }


}
