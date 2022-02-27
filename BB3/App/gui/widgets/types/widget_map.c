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

typedef struct {
    lv_obj_t * aircraft;
    lv_obj_t * label;
    lv_obj_t * label_shadow;
    lv_point_t direction[2];

    neighbor_t *nb;
} map_fanet_data_t;

REGISTER_WIDGET_ISUE(Map,
    "Map",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	0,

    lv_obj_t * image[9];
	lv_obj_t * arrow;
	lv_obj_t * dot;

    lv_point_t points[WIDGET_ARROW_POINTS];
    lv_obj_t * poi[NUMBER_OF_POI];
    uint8_t poi_magic[NUMBER_OF_POI];

    lv_obj_t * edit;
    uint32_t last_action;
    lv_point_t offsets[9];
    uint8_t master_tile;
    uint8_t action_cnt;
    uint8_t fanet_magic;
    lv_obj_t * arrow_image;
    map_fanet_data_t *fanet_data;

    uint8_t magic;
);

static bool static_init = false;
static lv_style_t static_label = {0};

#define FA_SIZE 1


static void Map_init(lv_obj_t * base, widget_slot_t * slot)
{
    if (!static_init)
    {
        lv_style_init(&static_label);
        lv_style_set_text_color(&static_label, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        static_init = true;
    }

    widget_create_base(base, slot);

    local->magic = 0xFF;
    local->master_tile = 0xFF;

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

    local->dot = lv_obj_create(slot->obj, NULL);
    lv_obj_set_size(local->dot, 10, 10);
    lv_obj_align(local->dot, slot->obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);

    local->arrow = widget_add_arrow(base, slot, local->points, NULL, NULL);
    lv_obj_set_style_local_line_color(local->arrow, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_obj_set_style_local_line_width(local->arrow, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 4);

	local->fanet_data = malloc(sizeof(map_fanet_data_t) * FA_SIZE);
	memset(local->fanet_data, 0, sizeof(map_fanet_data_t) * FA_SIZE);
	local->fanet_magic = 0;
}

static lv_obj_t * create_fanet_obj(widget_slot_t * slot, map_fanet_data_t *data);
static void update_fanet_obj(map_fanet_data_t *fanet, widget_slot_t * slot, char *label, int16_t x, int16_t y);

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

			widget_arrow_rotate_size(local->arrow, local->points, fc.gnss.heading, 40);

			lv_obj_align(local->arrow_image, slot->obj, LV_ALIGN_CENTER, 0, 0);
			lv_img_set_angle(local->arrow_image, fc.gnss.heading * 10);

			lv_obj_set_hidden(local->arrow, true);
			lv_obj_set_hidden(local->dot, true);
    	}
    	else
    	{
			lv_obj_set_hidden(local->arrow, true);
			lv_obj_set_hidden(local->dot, false);
    	}

    	// move fanet here
    }




//    // MOCK ITEM
//    map_fanet_data_t * fanet =  local->fanet_data;
//    neighbor_t * _nb = &fanet->nb;
//
//    _nb->alititude = 500;
//    _nb->heading = 180;
//    _nb->flags |= NB_HAVE_POS;
//    _nb->latitude = 522213000;
//    		        //529777833;
//    _nb->longitude = 209785166;//222111500;
//
//    _nb->dist = geo_distance(_nb->latitude, _nb->longitude, fc.gnss.latitude, fc.gnss.longtitude, true, NULL) / 100;
//    snprintf(_nb->name, sizeof(_nb->name), "fero");

//    if (size > 1) {
//		fanet = (map_fanet_data_t * ) local->fanet_data + sizeof(map_fanet_data_t);
//		_nb = &fanet->nb;
//		_nb->alititude = 101;
//		_nb->heading = 90;
//		_nb->flags |= NB_HAVE_POS;
//		_nb->latitude = 529777033;
//		_nb->longitude = 222111000;
//		_nb->dist = geo_distance(_nb->latitude, _nb->longitude, fc.gnss.latitude, fc.gnss.longtitude, true, NULL) / 100;
//
//		snprintf(_nb->name, sizeof(_nb->name), "tillman");
//    }

    // FANET
    // if cfg display fanet data on map

    int size = fc.fanet.neighbors_size;//FA_SIZE

    if (config_get_bool(&profile.map.show_fanet) && size > 0 && fc.gnss.fix > 0) {
		for (uint8_t i = 0; i < size; i++) {
			INFO("evaluatin nb: %d -> dist: %d", fc.fanet.neighbor[i].addr.user_id, fc.fanet.neighbor[i].dist);
//			fc.fanet.neighbor[i].latitude = 529777033;
//			fc.fanet.neighbor[i].longitude = 222111000;
//			fc.fanet.neighbor[i].flags |= NB_HAVE_POS;
//			fc.fanet.neighbor[i].dist = 1000;
			if (fc.fanet.neighbor[i].flags & NB_HAVE_POS && fc.fanet.neighbor[i].dist < 2000) {

				map_fanet_data_t *fanet = sizeof(map_fanet_data_t) * i + local->fanet_data;
				fanet->nb = &fc.fanet.neighbor[i];

				if (fanet->aircraft == NULL) {
					create_fanet_obj(slot, fanet);
				}

				char value[32];
				int16_t relalt;

				relalt = fanet->nb->alititude - fc.gnss.altitude_above_ellipsiod;

				if (relalt > 0)
					snprintf(value, sizeof(value), "%s\n+%i m", fanet->nb->name, relalt);
				else
					snprintf(value, sizeof(value), "%s\n%i m", fanet->nb->name, relalt);

				int32_t curr_lat = fc.gnss.latitude;
				int32_t curr_lon = fc.gnss.longtitude;
				int16_t zoom = config_get_int(&profile.map.zoom_flight);
				int16_t x, y;

				geo_to_pix_w_h(curr_lon, curr_lat, zoom, fanet->nb->longitude, fanet->nb->latitude, &x, &y, slot->w, slot->h);
		    	update_fanet_obj(fanet, slot, value, x, y);
				break; //FIXME: make it work for X
			} else {
				INFO("FANET nb: %d -> dist: %d is too far or no dist", fc.fanet.neighbor[i].addr.user_id, fc.fanet.neighbor[i].dist);
			}
		}
    }

    local->fanet_magic = fc.fanet.neighbors_magic;

    if (local->edit != NULL)
    {
    	lv_obj_t * base = widget_edit_overlay_get_base(local->edit);
    	lv_obj_t * zoom = lv_obj_get_child(base, NULL);

    	char buff[32];

    	snprintf(buff, sizeof(buff), "%d", config_get_int(&profile.map.zoom_flight));
    	lv_label_set_text(zoom, buff);
    }
}

static void hide_fanet_obj(map_fanet_data_t *fanet)
{
	lv_obj_set_hidden(fanet->aircraft, true);
}

static void show_fanet_obj(map_fanet_data_t *fanet)
{
	lv_obj_set_hidden(fanet->aircraft, false);
}

static void update_fanet_obj(map_fanet_data_t *fanet, widget_slot_t * slot, char *text, int16_t x, int16_t y)
{
	lv_obj_align(fanet->aircraft, local->arrow, LV_ALIGN_CENTER, x - slot->w / 2, y - slot->h / 2);
	lv_obj_align(fanet->label, fanet->aircraft, LV_ALIGN_OUT_BOTTOM_RIGHT, 10, 0);
	lv_obj_align(fanet->label_shadow, fanet->label, LV_ALIGN_CENTER, 1, 1);

	lv_label_set_text(fanet->label, text);
	lv_label_set_text(fanet->label_shadow, text);

	if (fanet->nb->dist < 200) {
		lv_obj_set_style_local_bg_color(fanet->aircraft, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
	} else {
		lv_obj_set_style_local_bg_color(fanet->aircraft, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	}
}

static lv_obj_t * create_fanet_obj(widget_slot_t * slot, map_fanet_data_t *fanet)
{
	fanet->aircraft = lv_obj_create(slot->obj, NULL);
	lv_obj_set_size(fanet->aircraft, 10, 10);
	lv_obj_align(fanet->aircraft, slot->obj, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_color(fanet->aircraft, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_obj_set_style_local_radius(fanet->aircraft, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 6);
	lv_obj_set_style_local_border_color(fanet->aircraft, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_obj_set_style_local_border_width(fanet->aircraft, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);

	fanet->label = lv_label_create(slot->obj, NULL);
	lv_label_set_long_mode(fanet->label, LV_LABEL_LONG_EXPAND);
	lv_label_set_align(fanet->label, LV_LABEL_ALIGN_CENTER);
	lv_obj_add_style(fanet->label, LV_LABEL_PART_MAIN, &gui.styles.widget_unit);
	lv_obj_set_width(fanet->label, 50);
	lv_obj_set_auto_realign(fanet->label, true);
	lv_obj_set_style_local_text_color(fanet->label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	fanet->label_shadow = lv_label_create(slot->obj, NULL);
	lv_label_set_long_mode(fanet->label_shadow, LV_LABEL_LONG_EXPAND);
	lv_label_set_align(fanet->label_shadow, LV_LABEL_ALIGN_CENTER);
	lv_obj_add_style(fanet->label_shadow, LV_LABEL_PART_MAIN, &gui.styles.widget_unit);
	lv_obj_set_width(fanet->label_shadow, 50);
	lv_obj_set_auto_realign(fanet->label_shadow, true);
	lv_obj_set_style_local_text_color(fanet->label_shadow, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

	return fanet->aircraft;
}

static void Map_stop(widget_slot_t * slot)
{
	free(local->fanet_data);
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
			local->edit = widget_create_edit_overlay("Map", "Set Zoom Level");
			lv_obj_t * base = widget_edit_overlay_get_base(local->edit);
			lv_obj_t * zoom = lv_label_create(base, NULL);
			lv_obj_set_style_local_text_font(zoom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, gui.styles.widget_fonts[FONT_XL]);

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
