/*
 * widget_fanet_radar.c
 *
 *  Created on: 26. 2. 2022
 *      Author: thrull
 */
#include <limits.h>
#include <common.h>

#include "gui/widgets/widget.h"
#include "etc/geo_calc.h"




REGISTER_WIDGET_ISUE
(
    FanetRadar,
    "FANET - radar",
    120,
    120,
    _b(wf_label_hide),

	lv_obj_t * canvas;
	lv_color_t *cbuf;
	uint32_t base_radius;

	lv_obj_t * arrow;
	lv_obj_t * dot;
	uint8_t fanet_magic;
);

static bool static_init = false;
static lv_draw_line_dsc_t draw_dsc;
static lv_draw_label_dsc_t label_dsc;
static lv_draw_img_dsc_t icon_dsc;

static void FanetRadar_init(lv_obj_t * base, widget_slot_t * slot)
{
    if (!static_init)
    {
    	lv_draw_label_dsc_init(&label_dsc);
    	label_dsc.color = LV_COLOR_WHITE;
    	label_dsc.font = &lv_font_montserrat_12;

    	lv_draw_line_dsc_init(&draw_dsc);
    	draw_dsc.color = LV_COLOR_GRAY;
    	draw_dsc.width = 2;

    	lv_draw_img_dsc_init(&icon_dsc);
    	icon_dsc.antialias = true;


        static_init = true;
    }

	local->fanet_magic = 0xFF;

	widget_create_base(base, slot);

	if (!widget_flag_is_set(slot, wf_label_hide))
	{
		widget_add_title(base, slot, slot->widget->name);
	}

	local->base_radius = min(slot->h, slot->w) / 7;

	local->cbuf = ps_malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(slot->w, slot->h));

	if (local->cbuf != NULL)
	{
	    local->canvas = lv_canvas_create(slot->obj, NULL);
	    lv_obj_move_background(local->canvas);
        lv_canvas_set_buffer(local->canvas, local->cbuf, slot->w, slot->h, LV_IMG_CF_TRUE_COLOR);

        lv_obj_set_size(local->canvas, slot->w, slot->h);
        lv_obj_set_pos(local->canvas, 0, 0);

        local->dot = lv_obj_create(slot->obj, NULL);
        lv_obj_set_size(local->dot, 10, 10);
        lv_obj_align(local->dot, slot->obj, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);

        local->arrow = lv_img_create(slot->obj, NULL);
        lv_img_set_src(local->arrow, &img_map_arrow);
        lv_obj_align(local->arrow, slot->obj, LV_ALIGN_CENTER, 0, 0);
        lv_img_set_antialias(local->arrow, true);
	}
	else
	{
	    lv_obj_t * l = lv_label_create(slot->obj, NULL);
	    lv_label_set_text(l, "Unable to allocate\nthe memory :-(");
	    lv_obj_align(l, slot->obj, LV_ALIGN_CENTER, 0, 0);
	}
}

static void FanetRadar_stop(widget_slot_t * slot)
{
    if (local->cbuf != NULL)
        ps_free(local->cbuf);
}

static uint16_t zoom_levels[] = {100, 200, 500, 1000, 1500, 2500, 3000, 5000};

static void FanetRadar_update(widget_slot_t * slot)
{
	char label_value[50];

	if (local->cbuf == NULL)
	    return;

	if (fc.fanet.neighbors_magic == local->fanet_magic)
	{
		// nothing to do
		return;
	}

	lv_canvas_fill_bg(local->canvas, LV_COLOR_BLACK, LV_OPA_COVER);

	lv_canvas_draw_arc(local->canvas, slot->w / 2, slot->h / 2, local->base_radius, 300, 240, &draw_dsc);
	lv_canvas_draw_arc(local->canvas, slot->w / 2, slot->h / 2, local->base_radius * 2, 290, 250, &draw_dsc);
	lv_canvas_draw_arc(local->canvas, slot->w / 2, slot->h / 2, local->base_radius * 3, 290, 250, &draw_dsc);

	uint8_t distance_units = config_get_select(&config.units.distance);

	float zoom = zoom_levels[config_get_int(&profile.fanet.radar_zoom)] / 1000.0;

	sprintf(label_value, "%0.1f", zoom * 1);
	lv_canvas_draw_text(local->canvas, slot->w / 2 - 25, slot->h / 2 - local->base_radius, 50, &label_dsc, label_value, LV_LABEL_ALIGN_CENTER);

	sprintf(label_value, "%0.1f", zoom * 3);
	lv_canvas_draw_text(local->canvas, slot->w / 2 - 25, slot->h / 2 - local->base_radius * 2, 50, &label_dsc, label_value, LV_LABEL_ALIGN_CENTER);

	sprintf(label_value, "%0.1f %s", zoom * 5, (distance_units == DISTANCE_METERS) ? "km" : "mi");
	lv_canvas_draw_text(local->canvas, slot->w / 2 - 25, slot->h / 2 - local->base_radius * 3, 50, &label_dsc, label_value, LV_LABEL_ALIGN_CENTER);

	if (fc.gnss.fix > 0)
	{
		char buffer[32];
		bool use_fai = config_get_select(&config.units.earth_model) == EARTH_FAI;

		for (uint8_t i = 0; i < fc.fanet.neighbors_size; i++)
		{
			if (fc.fanet.neighbor[i].flags & NB_HAVE_POS)
			{
				int16_t bearing = 0;
				int32_t radius;
				uint32_t dist;
				float step = 1000.0 * zoom;

				dist = geo_distance(fc.gnss.latitude, fc.gnss.longtitude, fc.fanet.neighbor[i].latitude, fc.fanet.neighbor[i].longitude, use_fai, &bearing) / 100;

				if (distance_units == DISTANCE_MILES)
				{
					dist = dist * FC_METER_TO_FEET;
					step = FC_FEET_IN_MILE;
				}

				const lv_img_dsc_t * icon = NULL;
                if (fc.fanet.neighbor[i].flags & NB_IS_FLYING)
                {
                    icon = &img_fanet_glider;
                    icon_dsc.angle = fc.fanet.neighbor[i].heading * 14; // ~ 360/255 * 10
                }
                else
                {
                    icon = &img_fanet_hike;
                    icon_dsc.angle = 0;
                }
                icon_dsc.pivot.x = icon->header.w / 2;
                icon_dsc.pivot.y = icon->header.h / 2;



				if (dist < (step))
				{
					radius = local->base_radius * 0.5 + (local->base_radius * (dist / step * 0.5));
				}
				else if (dist < (step * 3))
				{
					radius = local->base_radius + (local->base_radius * ((dist - step) / (step * 2.0)));
				}
				else if (dist < (step * 5))
				{
					radius = local->base_radius * 2 + (local->base_radius * ((dist - (step * 3)) / (step * 2.0)));
				}
				else
				{
					radius = local->base_radius * 3 + icon->header.h;
				}

				lv_coord_t x = radius * cos(to_radians(bearing - 90)) + slot->w / 2 - icon->header.w / 2;
				lv_coord_t y = radius * sin(to_radians(bearing - 90)) + slot->h / 2 - icon->header.h / 2;

                if (config_get_bool(&profile.fanet.show_labels))
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

                    lv_canvas_draw_text(local->canvas, x + icon->header.w, y - 5, 50, &label_dsc, label_value, LV_LABEL_ALIGN_LEFT);
                }

				lv_canvas_draw_img(local->canvas, x, y, icon, &icon_dsc);
			}
		}
	}

	local->fanet_magic = fc.fanet.neighbors_magic;

	if (fc.gnss.fix == 0)
	{
		lv_obj_set_hidden(local->arrow, true);
		lv_obj_set_hidden(local->dot, false);
	}
	else
	{
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

static void FanetRadar_edit(widget_slot_t * slot, uint8_t action)
{

    if (action == WIDGET_ACTION_LEFT || action == WIDGET_ACTION_RIGHT)
    {
        int8_t diff = 0;
        if (action == WIDGET_ACTION_LEFT)
            diff = +1;

        if (action == WIDGET_ACTION_RIGHT)
            diff = -1;

        if (diff != 0)
        {
            int16_t zoom = config_get_int(&profile.fanet.radar_zoom);
            int16_t new = zoom + diff;
            config_set_int(&profile.fanet.radar_zoom, new);

            widget_reset_edit_overlay_timer();
            local->fanet_magic = 0;
        }

    }

}

