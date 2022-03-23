/*
 * widget_map.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG
#include "gui/widgets/widget.h"
#include "gui/map/map_thread.h"
#include "gui/map/tile.h"

#include "etc/geo_calc.h"

#include "gui/images/arrow/arrow.h"

REGISTER_WIDGET_ISUE(Map,
    "Map",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	0,

    lv_obj_t * image[9];
	lv_obj_t * dot;
    lv_obj_t * arrow;

    lv_obj_t * poi[NUMBER_OF_POI];
    uint8_t poi_magic[NUMBER_OF_POI];

    lv_obj_t * edit;
    uint32_t last_action;
    lv_point_t offsets[9];
    uint8_t master_tile;
    uint8_t action_cnt;

    uint8_t magic;

    lv_obj_t * fanet_icons[NB_NUMBER_IN_MEMORY];
    lv_obj_t * fanet_labels[NB_NUMBER_IN_MEMORY];
    uint8_t fanet_obj_count;
    uint8_t fanet_magic;
);

static bool static_init = false;
static lv_style_t static_label = {0};
static lv_style_t fanet_label = {0};

LV_IMG_DECLARE(arrow_new);
LV_IMG_DECLARE(pg_icon);

static void Map_init(lv_obj_t * base, widget_slot_t * slot)
{
    if (!static_init)
    {
        lv_style_init(&static_label);
        lv_style_set_text_color(&static_label, LV_STATE_DEFAULT, LV_COLOR_BLACK);

        lv_style_init(&fanet_label);
        lv_style_set_text_color(&fanet_label, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_style_set_bg_opa(&fanet_label, LV_STATE_DEFAULT, LV_OPA_50);
    	lv_style_set_bg_blend_mode(&fanet_label, LV_STATE_DEFAULT, LV_BLEND_MODE_NORMAL);
    	lv_style_set_bg_color(&fanet_label, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    	lv_style_set_radius(&fanet_label, LV_STATE_DEFAULT, 4);
    	lv_style_set_text_font(&fanet_label, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_SMALL);
    	lv_style_set_pad_left(&fanet_label, LV_STATE_DEFAULT, 2);

        static_init = true;
    }

    widget_create_base(base, slot);

    local->magic = 0xFF;
    local->master_tile = 0xFF;
    local->fanet_magic = 0xFF;

    local->edit = NULL;
    local->last_action = 0;
    local->action_cnt = 0;

    for (uint8_t i = 0; i < 9; i++)
    {
		local->image[i] = lv_canvas_create(slot->obj, NULL);
		lv_obj_set_size(local->image[i], MAP_W, MAP_H);

		while(gui.map.chunks[i].buffer == NULL)
		{
			osDelay(1);
		}
		lv_canvas_set_buffer(local->image[i], gui.map.chunks[i].buffer, MAP_W, MAP_H, LV_IMG_CF_TRUE_COLOR);
		lv_obj_set_hidden(local->image[i], true);
	}

    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        local->poi[i] = NULL;
        local->poi_magic[i] = 0xFF;
    }

    for (uint8_t i = 0; i < NB_NUMBER_IN_MEMORY; i++) {
    	local->fanet_icons[i] = NULL;
    	local->fanet_labels[i] = NULL;
    }

    local->dot = lv_obj_create(slot->obj, NULL);
    lv_obj_set_size(local->dot, 10, 10);
    lv_obj_align(local->dot, slot->obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);

    local->arrow = lv_img_create(slot->obj, NULL);
    lv_img_set_src(local->arrow, &arrow_new);
    lv_obj_align(local->arrow, slot->obj, LV_ALIGN_CENTER, 0, 0);
    lv_img_set_antialias(local->arrow, true);
}

static void Map_update(widget_slot_t * slot)
{
    int32_t disp_lat;
    int32_t disp_lon;
//    int16_t zoom = config_get_int(&profile.map.zoom_flight);

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

    if (local->magic != gui.map.magic)
    {
//    	DBG("Widget set to index %u", gui.map.disp_buffer);
        for (uint8_t i = 0; i < 9; i++)
        {
            if (gui.map.chunks[i].ready)
            {
                lv_obj_set_hidden(local->image[i], false);
            }
            else
            {
                lv_obj_set_hidden(local->image[i], true);
                if (i == local->master_tile)
                    local->master_tile = 0xFF;
            }
        }

        for (uint8_t i = 0; i < 9; i++)
        {
            if (!gui.map.chunks[i].ready)
            {
                continue;
            }

            if (local->master_tile == 0xFF)
            {
                local->master_tile = i;
            }

            int16_t x, y;
            tile_geo_to_pix(i, disp_lon, disp_lat, &x, &y);

            x = -(x - slot->w / 2);
            y = -(y - slot->h / 2);

            lv_obj_set_pos(local->image[i], x, y);

            if (local->master_tile == i)
            {
                local->offsets[i].x = x;
                local->offsets[i].y = y;
            }
            else
            {
                local->offsets[i].x = x - local->offsets[local->master_tile].x;
                local->offsets[i].y = y - local->offsets[local->master_tile].y;;
            }
        }

        for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
        {
            if (gui.map.poi[i].chunk != 0xFF)
            {
                //create
                if (local->poi[i] == NULL)
                {
                    lv_obj_t * l = lv_label_create(slot->obj, NULL);
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
        local->magic = gui.map.magic;
    }


    if (local->master_tile != 0xFF)
    {
        int16_t x, y;
        tile_geo_to_pix(local->master_tile, disp_lon, disp_lat, &x, &y);

        x = -(x - slot->w / 2);
        y = -(y - slot->h / 2);

        local->offsets[local->master_tile].x = x;
        local->offsets[local->master_tile].y = y;

        lv_obj_set_pos(local->image[local->master_tile], x, y);

        for (uint8_t i = 0; i < 9; i++)
        {
            if (!gui.map.chunks[i].ready || i == local->master_tile)
            {
                continue;
            }

            x = local->offsets[local->master_tile].x + local->offsets[i].x;
            y = local->offsets[local->master_tile].y + local->offsets[i].y;
            lv_obj_set_pos(local->image[i], x, y);
        }
    }

    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        if (gui.map.poi[i].chunk != 0xFF && local->poi[i] != NULL)
        {
            uint16_t x = gui.map.poi[i].x;
            uint16_t y = gui.map.poi[i].y;

            lv_obj_align_mid(local->poi[i], local->image[gui.map.poi[i].chunk], LV_ALIGN_IN_TOP_LEFT, x, y);
        }
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
    		lv_img_set_angle(local->arrow, fc.gnss.heading * 10);
			lv_obj_set_hidden(local->arrow, false);
			lv_obj_set_hidden(local->dot, true);
    	}
    	else
    	{
			lv_obj_set_hidden(local->arrow, true);
			lv_obj_set_hidden(local->dot, false);
    	}

    	if (config_get_bool(&profile.map.show_fanet))
   	    {
   			if (fc.fanet.neighbors_magic != local->fanet_magic && fc.gnss.fix > 0)
   			{
   				char label_value[50];
   				char buffer[32];

   				int16_t zoom = config_get_int(&profile.map.zoom_flight);
    			int8_t t = 0;

    			for (uint8_t i = 0; i < fc.fanet.neighbors_size; i++)
    			{
    				if (fc.fanet.neighbor[i].flags & NB_HAVE_POS)
    				{
    					int16_t x, y;

    					geo_to_pix_w_h(fc.gnss.longtitude, fc.gnss.latitude, zoom, fc.fanet.neighbor[i].longitude, fc.fanet.neighbor[i].latitude, &x, &y, slot->w, slot->h);

    					format_altitude_with_units(buffer, fc.fanet.neighbor[i].alititude);
    					snprintf(label_value, sizeof(label_value), "%s\n%s", fc.fanet.neighbor[i].name, buffer);

    					if (local->fanet_icons[t] == NULL && local->fanet_labels[t] == NULL)
    					{
    						local->fanet_icons[t] = lv_img_create(slot->obj, NULL);
    						lv_img_set_src(local->fanet_icons[t], &pg_icon);
    						lv_img_set_antialias(local->fanet_icons[t], true);

    						local->fanet_labels[t] = lv_label_create(slot->obj, NULL);
    						lv_obj_add_style(local->fanet_labels[t], LV_LABEL_PART_MAIN, &fanet_label);
    						lv_label_set_align(local->fanet_labels[t], LV_LABEL_ALIGN_LEFT);
    					}

    					lv_label_set_text(local->fanet_labels[t], label_value);

    					lv_obj_align(local->fanet_icons[t], local->arrow, LV_ALIGN_CENTER, x - slot->w / 2, y - slot->h / 2);
    					lv_img_set_angle(local->fanet_icons[t], fc.fanet.neighbor[i].heading * 14); // ~ 360/255 * 10
    					lv_obj_align(local->fanet_labels[t], local->arrow, LV_ALIGN_CENTER, x - slot->w / 2 + 35, y - slot->h / 2);
    					lv_obj_set_hidden(local->fanet_icons[t], false);
    					lv_obj_set_hidden(local->fanet_labels[t], false);
    					t++;
    				}
    			}

    			for (uint8_t i = t; i < NB_NUMBER_IN_MEMORY; i++) {
    				if (local->fanet_icons[i] != NULL && local->fanet_labels[i] != NULL) {
    				    lv_obj_set_hidden(local->fanet_icons[i], true);
    					lv_obj_set_hidden(local->fanet_labels[i], true);
    				}
    			}
    		}
    		local->fanet_magic = fc.fanet.neighbors_magic;
   	    }
    }

    if (local->edit != NULL)
    {
    	lv_obj_t * base = widget_edit_overlay_get_base(local->edit);
    	lv_obj_t * zoom = lv_obj_get_child(base, lv_obj_get_child(base, NULL));

    	char buff[32];

    	uint16_t zoom_p = pow(2, config_get_int(&profile.map.zoom_flight));

    	int32_t guide_m = (zoom_p * 111000 * 120 / MAP_DIV_CONST);

    	format_distance_with_units2(buff, guide_m);
    	lv_label_set_text(zoom, buff);
    }
}

static void Map_stop(widget_slot_t * slot)
{
    if (local->edit != NULL)
    {
        widget_destroy_edit_overlay(local->edit);
    }
}

static void Map_edit(widget_slot_t * slot, uint8_t action)
{
    if (action == WIDGET_ACTION_CLOSE)
    {
        if (local->edit != NULL)
        {
            widget_destroy_edit_overlay(local->edit);
            local->edit = NULL;
        }

        return;
    }

    if (action == WIDGET_ACTION_MIDDLE)
    {
    	widget_destroy_edit_overlay(local->edit);
    	local->edit = NULL;
    }

    if (action == WIDGET_ACTION_LEFT || action == WIDGET_ACTION_RIGHT || action == WIDGET_ACTION_HOLD)
    {
		if (local->edit == NULL)
		{
			//create menu
			local->edit = widget_create_edit_overlay("", "Set Zoom Level");

            //no anim, make fully transparent
            lv_anim_get(local->edit, NULL);
            lv_obj_set_style_local_bg_opa(local->edit, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

			lv_obj_t * base = widget_edit_overlay_get_base(local->edit);
			lv_obj_t * zoom = lv_label_create(base, NULL);
            lv_obj_set_style_local_pad_top(zoom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 15);
            lv_obj_set_style_local_text_font(zoom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[FONT_L]);

			lv_obj_t * bar = lv_obj_create(base, NULL);
			lv_obj_set_size(bar, 120, 8);
            lv_obj_set_style_local_bg_color(bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_border_color(bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_obj_set_style_local_border_width(bar, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 1);

            lv_obj_t * bar2 = lv_obj_create(bar, NULL);
            lv_obj_set_size(bar2, 40, 8);
            lv_obj_align(bar2, bar, LV_ALIGN_CENTER, 0, 0);

			//update values
			Map_update(slot);
		}

		if (HAL_GetTick() - local->last_action < 300)
		{
			if (local->action_cnt < 50)
				local->action_cnt++;
		}
		else
		{
			local->action_cnt = 0;;
		}

		//INFO("AC %u %lu", local->action_cnt, HAL_GetTick() - local->last_action);

		local->last_action = HAL_GetTick();

		int8_t diff = 0;
		if (action == WIDGET_ACTION_LEFT)
			diff = -1;

		if (action == WIDGET_ACTION_RIGHT)
			diff = 1;

		if (diff != 0)
		{
			diff *= 1 + (local->action_cnt / 5);

			int16_t zoom = config_get_int(&profile.map.zoom_flight);
                        int16_t new = zoom + diff;
			config_set_int(&profile.map.zoom_flight, new);

			widget_reset_edit_overlay_timer();
		}

    }

}
