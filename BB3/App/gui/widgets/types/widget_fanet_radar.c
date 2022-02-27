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

REGISTER_WIDGET_ISU
(
    FanetRadar,
    "FANET - radar",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
        _b(wf_label_hide) | _b(wf_hide_icon),

	lv_obj_t * canvas;        // Canvas to paint to
	lv_color_t *cbuf;         // associated buffer of the canvas
	uint32_t base_radius;

	lv_obj_t * arrow;
	lv_obj_t * dot;
	uint8_t fanet_magic;

    lv_point_t points[WIDGET_ARROW_POINTS];
);

static bool static_init = false;
static lv_draw_line_dsc_t draw_dsc;
static lv_draw_line_dsc_t nb_dsc;
static lv_draw_line_dsc_t nb_dsc_proximity;
static lv_draw_label_dsc_t label_dsc;

static void FanetRadar_init(lv_obj_t * base, widget_slot_t * slot)
{
	int t_h;

    if (!static_init)
    {
    	lv_draw_label_dsc_init(&label_dsc);
    	label_dsc.color = LV_COLOR_WHITE;
    	label_dsc.font = &lv_font_montserrat_12;

    	lv_draw_line_dsc_init(&nb_dsc);
    	nb_dsc.color = LV_COLOR_GREEN;
    	nb_dsc.width = 2;

    	lv_draw_line_dsc_init(&nb_dsc_proximity);
    	nb_dsc_proximity.color = LV_COLOR_RED;
    	nb_dsc_proximity.width = 2;

    	lv_draw_line_dsc_init(&draw_dsc);
    	draw_dsc.color = LV_COLOR_GRAY;
    	draw_dsc.width = 2;

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

	local->base_radius = slot->h / 7;

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
	char label_buffer[20];

	if (fc.fanet.neighbors_magic == local->fanet_magic) {
		// nothing to do
		return;
	}

	lv_canvas_fill_bg(local->canvas, LV_COLOR_BLACK, LV_OPA_COVER);

	lv_canvas_draw_arc(local->canvas, slot->w / 2, slot->h / 2, local->base_radius, 300, 240, &draw_dsc);
	lv_canvas_draw_arc(local->canvas, slot->w / 2, slot->h / 2, local->base_radius * 2, 290, 250, &draw_dsc);
	lv_canvas_draw_arc(local->canvas, slot->w / 2, slot->h / 2, local->base_radius * 3, 290, 250, &draw_dsc);

	sprintf(label_buffer, "1 km");
	lv_canvas_draw_text(local->canvas, slot->w / 2 - 12, slot->h / 2 - local->base_radius, 50, &label_dsc, label_buffer, LV_LABEL_ALIGN_LEFT);

	sprintf(label_buffer, "3 km");
	lv_canvas_draw_text(local->canvas, slot->w / 2 - 12, slot->h / 2 - local->base_radius * 2, 50, &label_dsc, label_buffer, LV_LABEL_ALIGN_LEFT);

	sprintf(label_buffer, "5 km");
	lv_canvas_draw_text(local->canvas, slot->w / 2 - 12, slot->h / 2 - local->base_radius * 3, 50, &label_dsc, label_buffer, LV_LABEL_ALIGN_LEFT);


	if (fc.gnss.fix > 0) {
		for (uint8_t i = 0; i < fc.fanet.neighbors_size; i++) {
			fc.fanet.neighbor[i].latitude = 529777033;
			fc.fanet.neighbor[i].longitude = 222111000;
			fc.fanet.neighbor[i].flags |= NB_HAVE_POS;
			fc.fanet.neighbor[i].dist = 1000;
			fc.fanet.neighbor[i].climb = 12;

			if (fc.fanet.neighbor[i].flags & NB_HAVE_POS) {
				char value[32];

				int16_t relalt = fc.fanet.neighbor[i].alititude - fc.gnss.altitude_above_ellipsiod;

				snprintf(value, sizeof(value), "%s\n%s%i m\n%s%.1f m/s", fc.fanet.neighbor[i].name, (relalt > 0) ? "+" : "", relalt, (fc.fanet.neighbor[i].climb > 0) ? "+" : "", (double)(fc.fanet.neighbor[i].climb / 10.0) );

				bool use_fai = config_get_select(&config.units.earth_model) == EARTH_FAI;
				int16_t bearing = 0;
				int32_t radius;

				uint32_t dist = geo_distance(fc.gnss.latitude, fc.gnss.longtitude, fc.fanet.neighbor[i].latitude, fc.fanet.neighbor[i].longitude, use_fai, &bearing) / 100;

				radius = (dist < 1000 ) ? local->base_radius - 5 : ((dist < 3000) ? (local->base_radius * 2 - 5) : (local->base_radius * 3 + 5));

				lv_coord_t x = radius * cos(to_radians(bearing + 90)) + slot->w / 2;
				lv_coord_t y = radius * sin(to_radians(bearing + 90)) + slot->h / 2;

				lv_canvas_draw_text(local->canvas, x + 10, y - 10, 50, &label_dsc, value, LV_LABEL_ALIGN_LEFT);

				if (dist > 100)
					lv_canvas_draw_arc(local->canvas, x, y, 5, 1, 360, &nb_dsc);
				else
					lv_canvas_draw_arc(local->canvas, x, y, 5, 1, 360, &nb_dsc_proximity);
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
