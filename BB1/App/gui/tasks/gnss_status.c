#include "gnss_status.h"
#include "gnss.h"

#include "../gui_list.h"

#include "../../config/config.h"
#include "../../fc/fc.h"
#include "../../etc/format.h"

#define GNSS_STATUS_MAP	150
#define GNSS_STATUS_SAT	65
#define GNSS_DOT_SIZE	16

typedef struct
{
	lv_obj_t * map;
	lv_obj_t * sat[GNSS_NUMBER_OF_SYSTEMS];
	lv_style_t style_sats;
	lv_style_t style_bars;
	lv_style_t style_dots;
	lv_style_t style_color[GNSS_NUMBER_OF_SYSTEMS];
} local_vars_t;

void gnss_status_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_gnss, GUI_SW_LEFT_RIGHT);

}

void gui_list_event_cb(lv_obj_t * obj, lv_event_t event);

lv_obj_t * gnss_status_init(lv_obj_t * par)
{
	lv_style_init(&local.style_sats);
	lv_style_set_text_font(&local.style_sats, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_SMALL);
	lv_style_set_pad_inner(&local.style_sats, LV_STATE_DEFAULT, 0);

	lv_style_init(&local.style_bars);
	lv_style_set_margin_left(&local.style_bars, LV_STATE_DEFAULT, 3);
	lv_style_set_margin_right(&local.style_bars, LV_STATE_DEFAULT, 4);

	lv_style_init(&local.style_dots);
	lv_style_set_radius(&local.style_dots, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
	lv_style_set_text_font(&local.style_dots, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_SMALL);
	lv_style_set_bg_blend_mode(&local.style_dots, LV_STATE_DEFAULT, LV_BLEND_MODE_NORMAL);
	lv_style_set_bg_opa(&local.style_dots, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_style_set_text_color(&local.style_dots, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	lv_style_init(&local.style_color[0]);
	lv_style_set_bg_color(&local.style_color[0], LV_STATE_DEFAULT, lv_color_hex(0x0000FF));

	lv_style_init(&local.style_color[1]);
	lv_style_set_bg_color(&local.style_color[1], LV_STATE_DEFAULT, lv_color_hex(0xFF0000));

	lv_style_init(&local.style_color[2]);
	lv_style_set_bg_color(&local.style_color[2], LV_STATE_DEFAULT, lv_color_hex(0x00FF00));


	lv_obj_t * list = gui_list_create(par, "GNSS Status", gnss_status_cb);

	local.map = gui_list_cont_add(list, GNSS_STATUS_MAP);
	lv_obj_set_style_local_bg_color(local.map, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x7e8aea));
	lv_obj_set_style_local_bg_color(local.map, LV_CONT_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x7e8aea));
	lv_obj_set_style_local_radius(local.map, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
	lv_cont_set_layout(local.map, LV_LAYOUT_OFF);

	for (uint8_t i = 0; i < GNSS_NUMBER_OF_SYSTEMS; i++)
	{
		local.sat[i] = gui_list_cont_add(list, GNSS_STATUS_SAT);
		lv_obj_add_style(local.sat[i], LV_CONT_PART_MAIN, &local.style_sats);
		lv_cont_set_layout(local.sat[i], LV_LAYOUT_PRETTY_MID);

		for (uint8_t j = 0; j < GNSS_NUMBER_OF_SATS; j++)
		{
			lv_obj_t * bar = lv_bar_create(local.sat[i], NULL);
			lv_obj_add_style(bar, LV_BAR_PART_BG, &local.style_bars);
			lv_obj_add_style(bar, LV_BAR_PART_INDIC, &local.style_color[i]);
			lv_obj_set_size(bar, 10, 40);
		}

		for (uint8_t j = 0; j < GNSS_NUMBER_OF_SATS; j++)
		{
			lv_obj_t * label = lv_label_create(local.sat[i], NULL);
			lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
			lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
			lv_obj_add_style(label, LV_LABEL_PART_MAIN, &local.style_sats);
			lv_obj_set_size(label, 16, 20);

			lv_obj_t * dot = lv_label_create(local.map, NULL);
			lv_obj_add_style(dot, LV_LABEL_PART_MAIN, &local.style_dots);
			lv_obj_add_style(dot, LV_LABEL_PART_MAIN, &local.style_color[i]);
			lv_label_set_long_mode(dot, LV_LABEL_LONG_CROP);
			lv_label_set_align(dot, LV_LABEL_ALIGN_CENTER);
			lv_label_set_text_fmt(dot, "%02u", j);
			lv_obj_set_size(dot, GNSS_DOT_SIZE, GNSS_DOT_SIZE);
			lv_obj_set_pos(dot, j* 20, i * 20);
			lv_obj_set_hidden(dot, true);
		}
	}


	return list;
}

void gnss_status_loop()
{
	uint16_t map_w = lv_obj_get_width(local.map);
	uint16_t map_h= lv_obj_get_height(local.map);

	lv_obj_t * dot = lv_obj_get_child_back(local.map, NULL);

	for (uint8_t i = 0; i < GNSS_NUMBER_OF_SYSTEMS; i++)
	{
		lv_obj_t * obj = lv_obj_get_child_back(local.sat[i], NULL);

		for (uint8_t j = 0; j < GNSS_NUMBER_OF_SATS; j++)
		{
			uint8_t snr = fc.gnss.sat_info[i].sats[j].sat_id != 0 ? fc.gnss.sat_info[i].sats[j].snr : 0;

			lv_bar_set_value(obj, snr, LV_ANIM_ON);
			obj = lv_obj_get_child_back(local.sat[i], obj);
		}

		for (uint8_t j = 0; j < GNSS_NUMBER_OF_SATS; j++)
		{
			if (fc.gnss.sat_info[i].sats[j].sat_id)
			{
				lv_label_set_text_fmt(obj, "%02u", fc.gnss.sat_info[i].sats[j].sat_id);

				float rad = to_radians(fc.gnss.sat_info[i].sats[j].azimuth * 2);

				//move dot
				uint16_t x = map_w / 2 + sin(rad) * (fc.gnss.sat_info[i].sats[j].elevation * map_w / 180) - GNSS_DOT_SIZE / 2;
				uint16_t y = map_h / 2 + cos(rad) * (fc.gnss.sat_info[i].sats[j].elevation * map_h / 180) - GNSS_DOT_SIZE / 2;
				lv_obj_set_pos(dot, x, y);

				lv_obj_set_hidden(dot, false);
			}
			else
			{
				lv_obj_set_hidden(dot, true);
				lv_label_set_text(obj, "");
			}

			obj = lv_obj_get_child_back(local.sat[i], obj);
			dot = lv_obj_get_child_back(local.map, dot);
		}
	}
}

bool gnss_status_stop()
{
	return true;
}

gui_task_t gui_gnss_status =
{
	gnss_status_init,
	gnss_status_loop,
	gnss_status_stop
};
