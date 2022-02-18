/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG


#include "etc/geo_calc.h"

typedef struct
{
    lv_obj_t * image[9];
    lv_point_t offset[9];

	lv_obj_t * dot;

    lv_obj_t * poi[NUMBER_OF_POI];
    uint8_t poi_magic[NUMBER_OF_POI];

    uint8_t master;
    uint8_t magic;
} local_vars_t;

local_vars_t slocal;

local_vars_t * local = &slocal;

static bool static_init = false;
static lv_style_t static_label = {0};

void widget_map_init(lv_obj_t * base)
{
    if (!static_init)
    {
        lv_style_init(&static_label);
        lv_style_set_text_color(&static_label, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        static_init = true;
    }

    local->magic = 0xFF;
    local->master = 0xFF;

    for (uint8_t i = 0; i < 9; i++)
    {
		local->image[i] = lv_canvas_create(base, NULL);
		lv_obj_set_size(local->image[i], MAP_W, MAP_H);


		lv_canvas_set_buffer(local->image[i], gui.map.chunks[i].buffer, MAP_W, MAP_H, LV_IMG_CF_TRUE_COLOR);
		lv_obj_set_hidden(local->image[i], true);
	}

    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        local->poi[i] = NULL;
        local->poi_magic[i] = 0xFF;
    }

    local->dot = lv_obj_create(base, NULL);
    lv_obj_set_size(local->dot, 10, 10);
    lv_obj_align(local->dot, base, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_PURPLE);
    lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);


}

extern int16_t map_zoom;

void widget_map_update(lv_obj_t * base, int32_t disp_lat, int32_t disp_lon)
{
    if (local->magic != gui.map.magic)
    {
//    	DBG("Widget set to index %u", gui.map.disp_buffer);

        for (uint8_t i = 0; i < 9; i++)
        {
        	if (!gui.map.chunks[i].ready)
        	{
        		lv_obj_set_hidden(local->image[i], true);
        		if (local->master == i)
        			local->master = 0xFF;
        	}
        	else
        	{
        		lv_obj_set_hidden(local->image[i], false);
        	}
        }

        for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
        {
            FC_ATOMIC_ACCESS
            {
                if (gui.map.poi[i].chunk != 0xFF)
                {
                    //create
                    if (local->poi[i] == NULL)
                    {
                        lv_obj_t * l = lv_label_create(base, NULL);
                        lv_obj_add_style(l, LV_LABEL_PART_MAIN, &static_label);
                        lv_label_set_text(l, gui.map.poi[i].name);

                        local->poi[i] = l;
                        local->poi_magic[i] = gui.map.poi[i].magic;
                    }
                    else
                    {
                        //update
                        if (local->poi_magic[i] != gui.map.poi[i].magic)
                        {
                            lv_label_set_text(local->poi[i], gui.map.poi[i].name);
                            local->poi_magic[i] = gui.map.poi[i].magic;
                        }
                    }
                }
                else
                {
                    //remove
                    if (local->poi[i] != NULL)
                    {
                        lv_obj_del(local->poi[i]);
                        local->poi[i] = NULL;
                        local->poi_magic[i] = 0xFF;
                    }
                }
            }
        }
        local->magic = gui.map.magic;
    }

    int16_t zoom = map_zoom;
    int16_t slot_w =  lv_obj_get_width(base);
    int16_t slot_h =  lv_obj_get_height(base);
    for (uint8_t i = 0; i < 9; i++)
    {
    	if (!gui.map.chunks[i].ready)
    	{
    		continue;
    	}

		int16_t x, y;
		tile_geo_to_pix(i, disp_lon, disp_lat, &x, &y);

		x -= slot_w / 2;
		y -= slot_h / 2;

		lv_obj_set_pos(local->image[i], -x, -y);
    }

    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        FC_ATOMIC_ACCESS
        {
            if (gui.map.poi[i].chunk != 0xFF && local->poi[i] != NULL)
            {
                uint16_t x = gui.map.poi[i].x;
                uint16_t y = gui.map.poi[i].y;

                lv_obj_align_mid(local->poi[i], local->image[gui.map.poi[i].chunk], LV_ALIGN_IN_TOP_LEFT, x, y);
            }
        }
    }


}
