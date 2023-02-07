/*
 * map_obj.c
 *
 *  Created on: 20.09.2022
 *      Author: bubeck
 *
 * This is used to display the map in a widget. In order to use it, call map_obj_init once at the beginning
 * and use map_obj_loop inside the GUI loop to show the map.
 */

#include "map_obj.h"

#include "gui/gui_list.h"
#include "gui/map/tile.h"
#include "gui/map/map_obj.h"
#include "gui/images/images.h"
#include "map_types.h"

#include "fc/fc.h"
#include "etc/format.h"
#include "etc/geo_calc.h"

static bool static_init = false;
static lv_style_t static_label = { 0 };
static lv_style_t fanet_label = { 0 };

#define DOT_RADIUS 10

/**
 * Initialize the MAP object inside the given parent. All necessary data is stored
 * in "local" which has to be allocated by the caller. Typically this will be done
 * by putting map_obj_data_t inside the widgets local memory.
 *
 * @param parent the parent lv_objtto put the map into
 * @param local a pointer to a memory allocated by caller to store local data.
 *
 * @return the map object just created.
 */
lv_obj_t* map_obj_init(lv_obj_t *par, map_obj_data_t *local)
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


    local->magic = 0xFF;
    local->master_tile = 0xFF;

    local->map = lv_obj_create(par, NULL);
    lv_obj_set_size(local->map, lv_obj_get_width(par), lv_obj_get_height(par));
    lv_obj_set_pos(local->map, 0, 0);

    for (uint8_t i = 0; i < MAP_CHUNKS; i++)
    {
        local->image[i] = lv_canvas_create(local->map, NULL);
        lv_obj_set_size(local->image[i], MAP_W, MAP_H);

        while (gui.map.chunks[i].buffer == NULL)
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

    for (uint8_t i = 0; i < NB_NUMBER_IN_MEMORY; i++)
    {
        local->fanet_icons[i] = NULL;
        local->fanet_labels[i] = NULL;
    }

    local->dot = NULL;
    local->arrow = NULL;

    local->spinner = lv_spinner_create(par, NULL);
    lv_obj_set_size(local->spinner, 50, 50);
    lv_obj_set_pos(local->spinner, 0, 0);
    lv_spinner_set_type(local->spinner, LV_SPINNER_TYPE_CONSTANT_ARC);
    lv_obj_set_style_local_bg_opa(local->spinner, LV_SPINNER_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_line_width(local->spinner, LV_SPINNER_PART_BG, LV_STATE_DEFAULT, 0);
//    lv_obj_set_style_local_line_width(local->spinner, LV_SPINNER_PART_INDIC, LV_STATE_DEFAULT, 5);

    return local->map;
}

/**
 * Create an image to display a POI.
 *
 * @param map the parent object for the new image
 * @param src_img the source of the image
 *
 * @return the image object of the POI
 */
static lv_obj_t *create_img_pin(lv_obj_t *map, const void *src_img)
{
    lv_obj_t *l;

    l = lv_img_create(map, NULL);
    lv_img_set_src(l, src_img);
    lv_obj_align(l, map, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_move_foreground(l);
    lv_img_set_antialias(l, true);

    return l;
}

/**
 * Decide which image to use for the given POI type.
 *
 * @param type the POI type
 *
 * @return the image or NULL if type is not using image.
 */
static lv_img_dsc_t *get_poi_img(int type)
{
  lv_img_dsc_t *l;

  switch (type)
    {
    case MAP_TYPE_POI_LANDING:
      l = &img_pin_landing_blue_black_border;
      break;
    case MAP_TYPE_POI_TAKEOFF:
      l = &img_pin_takeoff_orange_black_border;
      break;
    case MAP_TYPE_POI_AEROWAY:
      l = &img_pin_aerodrome_blue_black_border;
      break;
    default:
      l = NULL;
      break;
    }

  return l;
}

/**
 * Display the map at the given position given as the center.
 *
 * @param local a pointer to a memory allocated by caller to store local data.
 * @param disp_lat latitude of the center of the map
 * @param disp_lon longitude of the center of the map
 */
void map_obj_loop(map_obj_data_t *local, int32_t disp_lat, int32_t disp_lon)
{
    if (local->magic != gui.map.magic)
    {
//    	DBG("Widget set to index %u", gui.map.disp_buffer);
        bool idle = true;
        for (uint8_t i = 0; i < MAP_CHUNKS; i++)
        {
            if (gui.map.chunks[i].ready)
            {
                lv_obj_set_hidden(local->image[i], false);
            }
            else
            {
                idle = false;
                lv_obj_set_hidden(local->image[i], true);
                if (i == local->master_tile)
                    local->master_tile = 0xFF;
            }
        }

        lv_obj_set_hidden(local->spinner, idle);

        for (uint8_t i = 0; i < MAP_CHUNKS; i++)
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

            x = -(x - lv_obj_get_width(local->map) / 2);
            y = -(y - lv_obj_get_height(local->map) / 2);

            lv_obj_set_pos(local->image[i], x, y);

            if (local->master_tile == i)
            {
                local->offsets[i].x = x;
                local->offsets[i].y = y;
            }
            else
            {
                local->offsets[i].x = x - local->offsets[local->master_tile].x;
                local->offsets[i].y = y - local->offsets[local->master_tile].y;
           }
        }

        for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
        {
            if (gui.map.poi[i].chunk != 0xFF)
            {
                //create
                if (local->poi[i] != NULL) lv_obj_del(local->poi[i]);

				lv_obj_t *l;
				lv_img_dsc_t *img_dsc;

				img_dsc = get_poi_img(gui.map.poi[i].type);
				if (img_dsc != NULL)
				{
					l = create_img_pin(local->map, img_dsc);
				}
				else
				{
					l = lv_label_create(local->map, NULL);
					lv_obj_add_style(l, LV_LABEL_PART_MAIN, &static_label);
					lv_label_set_text(l, gui.map.poi[i].name);
				}

				local->poi[i] = l;
				local->poi_magic[i] = gui.map.poi[i].magic;
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

        x = -(x - lv_obj_get_width(local->map) / 2);
        y = -(y - lv_obj_get_height(local->map) / 2);

        local->offsets[local->master_tile].x = x;
        local->offsets[local->master_tile].y = y;

        lv_obj_set_pos(local->image[local->master_tile], x, y);

        for (uint8_t i = 0; i < MAP_CHUNKS; i++)
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

    // POI draw #1: Labels. Draw them first, so that icons are in front.
    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        if (gui.map.poi[i].chunk != 0xFF && local->poi[i] != NULL && MAP_TYPE_IS_POI_LABEL(gui.map.poi[i].type))
        {
            uint16_t x = gui.map.poi[i].x;
            uint16_t y = gui.map.poi[i].y;
            lv_obj_align_mid(local->poi[i], local->image[gui.map.poi[i].chunk], LV_ALIGN_IN_TOP_LEFT, x, y);
        }
    }
    // POI draw #2: Icons
    for (uint8_t i = 0; i < NUMBER_OF_POI; i++)
    {
        if (gui.map.poi[i].chunk != 0xFF && local->poi[i] != NULL && MAP_TYPE_IS_POI_ICON(gui.map.poi[i].type))
        {
            uint16_t x = gui.map.poi[i].x;
            uint16_t y = gui.map.poi[i].y;
            lv_img_dsc_t *img_dsc;

            img_dsc = get_poi_img(gui.map.poi[i].type);
            if ( img_dsc != NULL)
            {
            	// POI image should be aligned at the bottom
            	y -= img_dsc->header.h / 2;
                lv_obj_move_foreground(local->poi[i]);
                lv_obj_align_mid(local->poi[i], local->image[gui.map.poi[i].chunk], LV_ALIGN_IN_TOP_LEFT, x, y);
            }
        }
    }
}

/**
 * Return the screen position of the master tile. This can be used to 
 * find out, whether the map has been moved by comparing this position
 * with a previous position.
 *
 * @param local the pointer to the map_obj_data_t
 * @param p a pointer to a lv_point_t where x/y is stored. If there is
 *          no master_tile, then position INT16_MAX/INT16_MAX is returned.
 */
void map_get_master_tile_xy(map_obj_data_t * map, lv_point_t *p)
{
    if (map->master_tile != 0xFF)
    {
    	*p = map->offsets[map->master_tile];
    }
    else
    {
    	p->x = INT16_MAX;
    	p->y = INT16_MAX;
    }
}

/**
 * Show the glider in the middle of the map.
 *
 * @param local the map data
 */
void map_obj_glider_loop(map_obj_data_t *local, lv_point_t glider_pos)
{
    if (local->arrow == NULL)
    {
        local->arrow = lv_img_create(local->map, NULL);
        lv_img_set_src(local->arrow, &img_map_arrow);
        lv_obj_align(local->arrow, local->map, LV_ALIGN_CENTER, 0, 0);
	lv_obj_move_foreground(local->arrow);
        lv_img_set_antialias(local->arrow, true);

	    local->dot = lv_obj_create(local->map, NULL);
	    lv_obj_set_size(local->dot, DOT_RADIUS, DOT_RADIUS);
	    lv_obj_align(local->dot, local->map, LV_ALIGN_CENTER, 0, 0);
	    lv_obj_move_foreground(local->dot);
	    lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
	    lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, DOT_RADIUS / 2);
	}

    if (fc.gnss.fix == 0)
    {
        lv_obj_set_hidden(local->arrow, true);
        lv_obj_set_hidden(local->dot, true);
    }
    else
    {
        lv_obj_set_pos(local->arrow, glider_pos.x - img_map_arrow.header.w/2, glider_pos.y- img_map_arrow.header.h/2);
        lv_obj_set_pos(local->dot, glider_pos.x - DOT_RADIUS/2, glider_pos.y - DOT_RADIUS/2);
        if (fc.gnss.ground_speed > FC_SPEED_MOVING)
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
    }
}

void map_obj_fanet_loop(map_obj_data_t *local, int32_t disp_lat, int32_t disp_lon, uint16_t zoom)
{
    if (config_get_bool(&profile.map.show_fanet))
    {
        if (fc.fanet.neighbors_magic != local->fanet_magic && fc.gnss.fix > 0)
        {
            char label_value[50];
            char buffer[32];

            int8_t t = 0;

            for (uint8_t i = 0; i < fc.fanet.neighbors_size; i++)
            {
                if (fc.fanet.neighbor[i].flags & NB_HAVE_POS)
                {
                    int16_t x, y;

                    geo_to_pix_w_h(disp_lon, disp_lat, zoom, fc.fanet.neighbor[i].longitude, fc.fanet.neighbor[i].latitude, &x, &y, lv_obj_get_width(local->map), lv_obj_get_height(local->map));

                    if (local->fanet_icons[t] == NULL && local->fanet_labels[t] == NULL)
                    {
                        local->fanet_icons[t] = lv_img_create(local->map, NULL);
                        lv_img_set_antialias(local->fanet_icons[t], true);

                        if (config_get_bool(&profile.fanet.show_labels))
                        {
                            local->fanet_labels[t] = lv_label_create(local->map, NULL);
                            lv_obj_add_style(local->fanet_labels[t], LV_LABEL_PART_MAIN, &fanet_label);
                            lv_label_set_align(local->fanet_labels[t], LV_LABEL_ALIGN_LEFT);
                        }
                        else
                        {
                            local->fanet_labels[t] = NULL;
                        }
                    }

                    if (fc.fanet.neighbor[i].flags & NB_IS_FLYING)
                    {
                        lv_img_set_src(local->fanet_icons[t], &img_fanet_glider);
                        lv_img_set_angle(local->fanet_icons[t], fc.fanet.neighbor[i].heading * 14); // ~ 360/255 * 10
                    }
                    else
                    {
                        lv_img_set_src(local->fanet_icons[t], &img_fanet_hike);
                        lv_img_set_angle(local->fanet_icons[t], 0); // ~ 360/255 * 10
                    }

                    if (local->fanet_labels[t] != NULL)
                    {
                        format_altitude_with_units(buffer, fc.fanet.neighbor[i].alititude);
                        if (strlen(fc.fanet.neighbor[i].name) > 0)
                        {
                            snprintf(label_value, sizeof(label_value), "%s\n%s", fc.fanet.neighbor[i].name, buffer);
                        }
                        else
                        {
                            strncpy(label_value, buffer, sizeof(label_value));
                        }

                        lv_label_set_text(local->fanet_labels[t], label_value);
                        lv_obj_align(local->fanet_labels[t], local->arrow, LV_ALIGN_CENTER, x - lv_obj_get_width(local->map) / 2 + 35, y - lv_obj_get_height(local->map) / 2);
                        lv_obj_set_hidden(local->fanet_labels[t], false);
                    }

                    lv_obj_align(local->fanet_icons[t], local->arrow, LV_ALIGN_CENTER, x - lv_obj_get_width(local->map) / 2, y - lv_obj_get_height(local->map) / 2);

                    lv_obj_set_hidden(local->fanet_icons[t], false);
                    t++;
                }
            }

            for (uint8_t i = t; i < NB_NUMBER_IN_MEMORY; i++)
            {
                if (local->fanet_icons[i] != NULL && local->fanet_labels[i] != NULL)
                {
                    lv_obj_set_hidden(local->fanet_icons[i], true);
                    if (local->fanet_labels[t] != NULL)
                    {
                        lv_obj_set_hidden(local->fanet_labels[i], true);
                    }
                }
            }
        }
        local->fanet_magic = fc.fanet.neighbors_magic;
    }
}
