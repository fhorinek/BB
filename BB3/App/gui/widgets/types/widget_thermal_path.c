/*
 * widget_thermal_path.c
 *
 *  Created on: 12. 12. 2021
 *      Author: thrull
 */

#include "gui/widgets/widget.h"
#include "fc/fc.h"
#include "etc/geo_calc.h"

#define THERMAL_DOTS_POSITIONS 60
#define THERMAL_DOT_SIZE 10
#define THERMAL_HISTORY_STEP (1000 / FC_HISTORY_PERIOD)
#define THERMAL_ZOOM_THRESHOLD_GS 18.0 // GS above zoom

REGISTER_WIDGET_IU
(
    TTrace,
    _i("Thermal trace"),
    120,
    120,
	_b(wf_label_hide),

    lv_obj_t * thermals[THERMAL_DOTS_POSITIONS];

    lv_obj_t * arrow;
);

static bool static_init = false;
static lv_style_t static_dot = {0};

static void TTrace_init(lv_obj_t * base, widget_slot_t * slot)
{
    if (!static_init)
    {
        lv_style_init(&static_dot);
        lv_style_set_radius(&static_dot, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
        static_init = true;
    }

    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, _("Thermal trace"));

	 for (uint8_t i = 0; i < THERMAL_DOTS_POSITIONS; i++)
	 {
		 local->thermals[i] = lv_obj_create(slot->obj, NULL);
		 lv_obj_add_style(local->thermals[i], LV_OBJ_PART_MAIN, &static_dot);
		 lv_obj_set_hidden(local->thermals[i], true);
	 }

    local->arrow = lv_img_create(slot->obj, NULL);
    lv_img_set_src(local->arrow, &img_map_arrow);
    lv_obj_align(local->arrow, slot->obj, LV_ALIGN_CENTER, 0, 0);
    lv_img_set_antialias(local->arrow, true);
}

static void TTrace_update(widget_slot_t * slot)
{
	if (fc.gnss.fix > 0)
	{
    	int32_t curr_lat = fc.gnss.latitude;
    	int32_t curr_lon = fc.gnss.longitude;

    	int16_t x, y;

    	int16_t i = THERMAL_HISTORY_STEP * 2; // skip last 2 history positions
		int16_t t = THERMAL_DOTS_POSITIONS;

		while (i < fc.history.size)
		{
			uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i) % FC_HISTORY_SIZE;
			fc_pos_history_t * h_pos = &fc.history.positions[index];

			i += THERMAL_HISTORY_STEP;

			int32_t point_lat = h_pos->lat;
			int32_t point_lon = h_pos->lon;
			if (fc.wind.valid && config_get_bool(&profile.flight.compensate_wind))
			{
			    //time in s
			    //wind speed is in m/s
			    //dist is in km
			    float time = (fc.gnss.altitude_above_ellipsiod - h_pos->gnss_alt) / (h_pos->vario / 100.0);
			    float dist = (fc.wind.speed / 1000.0) * time;
			    geo_destination(point_lat, point_lon, fc.wind.direction + 180, dist, &point_lat, &point_lon);
			}
			geo_to_pix_w_h(curr_lon, curr_lat, 1, point_lon, point_lat, &x, &y, slot->w, slot->h);

			lv_color_t c = LV_COLOR_SILVER;

            if (h_pos->vario < -60)
            {
                c = LV_COLOR_GRAY;
            }
            else if (h_pos->vario < 0)
            {
                c = LV_COLOR_SILVER;
            }
            else if (h_pos->vario < 50)
            {
                c = LV_COLOR_MAKE(0x11, 0xcc, 0x11);
            }
            else if (h_pos->vario < 100)
            {
                c = LV_COLOR_MAKE(0x00, 0xAF, 0x00);
            }
            else if (h_pos->vario < 150)
            {
                c = LV_COLOR_GREEN;
            }
            else if (h_pos->vario < 220)
            {
                c = LV_COLOR_MAKE(0xFF, 0x6F, 0x00);
            }
            else if (h_pos->vario >= 250)
            {
                c = LV_COLOR_MAKE(0xFF, 0x1A, 0x00);
            }

            t--;

            c = lv_color_darken(c, ((THERMAL_DOTS_POSITIONS - t) * 255) / THERMAL_DOTS_POSITIONS);

			if (lv_obj_get_style_bg_color(local->thermals[t], LV_OBJ_PART_MAIN).full != c.full)
			    lv_obj_set_style_local_bg_color(local->thermals[t], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, c);

			uint16_t size = THERMAL_DOT_SIZE + h_pos->vario / 75;
            size = max(size, THERMAL_DOT_SIZE / 2);
            size = min(size, THERMAL_DOT_SIZE * 3);

            lv_obj_set_size(local->thermals[t], size, size);

            uint16_t m = 2; //TODO: adaptive zoom

			lv_obj_set_hidden(local->thermals[t], false);
			lv_obj_align(local->thermals[t], local->arrow, LV_ALIGN_CENTER,
			        (x - slot->w / 2) * m,
			        (y - slot->h / 2) * m);


			if (t == 0)
                break;
		}
	}

	if (fc.gnss.fix == 0)
	{
		lv_obj_set_hidden(local->arrow, true);
	}
	else
	{
	    lv_img_set_angle(local->arrow, fc.gnss.heading * 10);
        lv_obj_set_hidden(local->arrow, false);
	}
}
