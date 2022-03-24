/*
 * widget_fanet_radar.c
 *
 *  Created on: 26. 2. 2022
 *      Author: thrull
 */
#include <limits.h>
#include <common.h>

#include "gui/widgets/widget.h"
#include "gui/images/fanet/pg_icon.h"
#include "etc/geo_calc.h"

LV_IMG_DECLARE(pg_icon);

REGISTER_WIDGET_ISU
(
    FanetRadar,
    "FANET - radar",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
        _b(wf_label_hide) | _b(wf_hide_icon),

	lv_obj_t * canvas;
	lv_color_t *cbuf;
	uint32_t base_radius;

	lv_obj_t * arrow;
	lv_obj_t * dot;
	uint8_t fanet_magic;

    lv_point_t points[WIDGET_ARROW_POINTS];
);

static bool static_init = false;
static lv_draw_line_dsc_t draw_dsc;
static lv_draw_label_dsc_t label_dsc;
static lv_draw_img_dsc_t pg_icon_dsc;

static void FanetRadar_init(lv_obj_t * base, widget_slot_t * slot)
{
	int t_h;

    if (!static_init)
    {
    	lv_draw_label_dsc_init(&label_dsc);
    	label_dsc.color = LV_COLOR_WHITE;
    	label_dsc.font = &lv_font_montserrat_12;

    	lv_draw_line_dsc_init(&draw_dsc);
    	draw_dsc.color = LV_COLOR_GRAY;
    	draw_dsc.width = 2;

    	lv_draw_img_dsc_init(&pg_icon_dsc);
    	pg_icon_dsc.antialias = true;
    	pg_icon_dsc.pivot.x = pg_icon.header.w / 2;
    	pg_icon_dsc.pivot.y = pg_icon.header.h / 2;

        static_init = true;
    }

	local->fanet_magic = 0xFF;

	widget_create_base(base, slot);

	if (!widget_flag_is_set(slot, wf_label_hide))
	{
		widget_add_title(base, slot, slot->widget->name);
		t_h = lv_obj_get_height(slot->title);
	} else
		t_h = 0;

	local->base_radius = min(slot->h, slot->w) / 7;

	local->canvas = lv_canvas_create(slot->obj, NULL);
	local->cbuf = malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(slot->w, slot->h - t_h));

	lv_canvas_set_buffer(local->canvas, local->cbuf, slot->w, slot->h - t_h, LV_IMG_CF_TRUE_COLOR);

	lv_obj_set_size(local->canvas, slot->w, slot->h - t_h);
	lv_obj_set_pos(local->canvas, 0, t_h);

    local->dot = lv_obj_create(slot->obj, NULL);
    lv_obj_set_size(local->dot, 10, 10);
    lv_obj_align(local->dot, slot->obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_color(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_radius(local->dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 5);

    local->arrow = widget_add_arrow(base, slot, local->points, NULL, NULL);
    lv_obj_set_style_local_line_color(local->arrow, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_line_width(local->arrow, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 4);

}

static void FanetRadar_stop(widget_slot_t * slot)
{
	free(local->cbuf);
	local->cbuf = NULL;
}

static void FanetRadar_update(widget_slot_t * slot)
{
	char label_value[50];

	if (fc.fanet.neighbors_magic == local->fanet_magic) {
		// nothing to do
		return;
	}
	lv_canvas_fill_bg(local->canvas, LV_COLOR_BLACK, LV_OPA_COVER);

	lv_canvas_draw_arc(local->canvas, slot->w / 2, slot->h / 2, local->base_radius, 300, 240, &draw_dsc);
	lv_canvas_draw_arc(local->canvas, slot->w / 2, slot->h / 2, local->base_radius * 2, 290, 250, &draw_dsc);
	lv_canvas_draw_arc(local->canvas, slot->w / 2, slot->h / 2, local->base_radius * 3, 290, 250, &draw_dsc);

	uint8_t distance_units = config_get_select(&config.units.distance);

	sprintf(label_value, "1 %s", (distance_units == DISTANCE_METERS) ? "km" : "mi");
	lv_canvas_draw_text(local->canvas, slot->w / 2 - 12, slot->h / 2 - local->base_radius, 50, &label_dsc, label_value, LV_LABEL_ALIGN_LEFT);

	sprintf(label_value, "3 %s", (distance_units == DISTANCE_METERS) ? "km" : "mi");
	lv_canvas_draw_text(local->canvas, slot->w / 2 - 12, slot->h / 2 - local->base_radius * 2, 50, &label_dsc, label_value, LV_LABEL_ALIGN_LEFT);

	sprintf(label_value, "5 %s", (distance_units == DISTANCE_METERS) ? "km" : "mi");
	lv_canvas_draw_text(local->canvas, slot->w / 2 - 12, slot->h / 2 - local->base_radius * 3, 50, &label_dsc, label_value, LV_LABEL_ALIGN_LEFT);

	if (fc.gnss.fix > 0) {
		char buffer[32];
		bool use_fai = config_get_select(&config.units.earth_model) == EARTH_FAI;
		for (uint8_t i = 0; i < fc.fanet.neighbors_size; i++) {
			if (fc.fanet.neighbor[i].flags & NB_HAVE_POS) {

				int16_t bearing = 0;
				int32_t radius;
				uint32_t dist;
				float step = 1000.0;

				dist = geo_distance(fc.gnss.latitude, fc.gnss.longtitude, fc.fanet.neighbor[i].latitude, fc.fanet.neighbor[i].longitude, use_fai, &bearing) / 100;

				if (distance_units == DISTANCE_MILES) {
					dist = dist * FC_METER_TO_FEET;
					step = FC_FEET_IN_MILE;
				}

				format_altitude_with_units(buffer, fc.fanet.neighbor[i].alititude);

				snprintf(label_value, sizeof(label_value), "%s\n%s", fc.fanet.neighbor[i].name, buffer);

				if (dist < (step)){
					radius = local->base_radius * 0.5 + (local->base_radius * (dist / step * 0.5));
				} else if (dist < (step * 3)) {
					radius = local->base_radius + (local->base_radius * ((dist - step) / (step * 2.0)));
				} else if (dist < (step * 5))
					radius = local->base_radius * 2 + (local->base_radius * ((dist - (step * 3)) / (step * 2.0)));
				else
					radius = local->base_radius * 3 + pg_icon.header.h;

				lv_coord_t x = radius * cos(to_radians(bearing - 90)) + slot->w / 2 - pg_icon.header.w / 2;
				lv_coord_t y = radius * sin(to_radians(bearing - 90)) + slot->h / 2 - pg_icon.header.h / 2;

				lv_canvas_draw_text(local->canvas, x + pg_icon.header.w, y - 5, 50, &label_dsc, label_value, LV_LABEL_ALIGN_LEFT);

				pg_icon_dsc.angle = fc.fanet.neighbor[i].heading * 14; // ~ 360/255 * 10

				lv_canvas_draw_img(local->canvas, x, y, &pg_icon, &pg_icon_dsc);
			}
		}
	}

	local->fanet_magic = fc.fanet.neighbors_magic;

	if (fc.gnss.fix == 0) {
		lv_obj_set_hidden(local->arrow, true);
		lv_obj_set_hidden(local->dot, false);
	} else {
		if (fc.gnss.ground_speed > 2) {
			widget_arrow_rotate_size(local->arrow, local->points, fc.gnss.heading, 40);
			lv_obj_set_hidden(local->arrow, false);
			lv_obj_set_hidden(local->dot, true);
		} else {
			lv_obj_set_hidden(local->arrow, true);
			lv_obj_set_hidden(local->dot, false);
		}
	}
}
