#include <gui/tasks/menu/gnss/gnss.h>
#include <gui/tasks/menu/gnss/gnss_status.h>
#include "gui/gui_list.h"

#include "fc/fc.h"
#include "etc/format.h"

#define GNSS_STATUS_MAP		150
#define GNSS_STATUS_SAT		150
#define GNSS_STATUS_INFO_W	70
#define GNSS_STATUS_INFO_H	20
#define GNSS_DOT_SIZE		16

#define GNSS_INFO_CNT		6

REGISTER_TASK_IL(gnss_status,
	lv_obj_t * map;
	lv_obj_t * sats;
	lv_obj_t * info[GNSS_INFO_CNT];

	lv_style_t style_sats;
	lv_style_t style_dots;
	lv_style_t style_color[4];
	lv_style_t style_unused;
);

#define STYLE_GPS		0
#define STYLE_GLONASS	1
#define STYLE_GALILEO	2
#define STYLE_BEIDOU	3

void gnss_status_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_gnss, LV_SCR_LOAD_ANIM_MOVE_RIGHT);

}

lv_obj_t * gnss_status_init(lv_obj_t * par)
{
	lv_style_init(&local->style_sats);
	lv_style_set_text_font(&local->style_sats, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_SMALL);
	lv_style_set_pad_inner(&local->style_sats, LV_STATE_DEFAULT, 0);

	lv_style_init(&local->style_dots);
	lv_style_set_radius(&local->style_dots, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
	lv_style_set_text_font(&local->style_dots, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_SMALL);
	lv_style_set_bg_blend_mode(&local->style_dots, LV_STATE_DEFAULT, LV_BLEND_MODE_NORMAL);
	lv_style_set_bg_opa(&local->style_dots, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_style_set_text_color(&local->style_dots, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	lv_style_init(&local->style_color[STYLE_GPS]);
	lv_style_set_bg_color(&local->style_color[0], LV_STATE_DEFAULT, lv_color_hex(0x0000FF));

	lv_style_init(&local->style_color[STYLE_GLONASS]);
	lv_style_set_bg_color(&local->style_color[1], LV_STATE_DEFAULT, lv_color_hex(0xFF0000));

	lv_style_init(&local->style_color[STYLE_GALILEO]);
	lv_style_set_bg_color(&local->style_color[2], LV_STATE_DEFAULT, lv_color_hex(0x00FF00));

	lv_style_init(&local->style_color[STYLE_BEIDOU]);
	lv_style_set_bg_color(&local->style_color[2], LV_STATE_DEFAULT, lv_color_hex(0xFFFF00));

	lv_style_init(&local->style_unused);
	lv_style_set_bg_opa(&local->style_unused, LV_STATE_DEFAULT, LV_OPA_50);

	lv_obj_t * list = gui_list_create(par, "GNSS Status", NULL, NULL);

    gui_set_dummy_event_cb(par, gnss_status_cb);

	local->map = gui_list_cont_add(list, GNSS_STATUS_MAP);
	lv_obj_set_style_local_bg_color(local->map, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x7e8aea));
    lv_obj_set_style_local_radius(local->map, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
	lv_cont_set_layout(local->map, LV_LAYOUT_OFF);

	local->sats = gui_list_cont_add(list, GNSS_STATUS_SAT);
	lv_obj_add_style(local->sats, LV_CONT_PART_MAIN, &local->style_sats);
	lv_cont_set_layout(local->sats, LV_LAYOUT_GRID);

	//info labels
	for (uint8_t i = 0; i < GNSS_INFO_CNT; i++)
	{
		local->info[i] = lv_label_create(local->sats, NULL);
		lv_label_set_long_mode(local->info[i], LV_LABEL_LONG_CROP);
		lv_label_set_align(local->info[i], i % 3);
		lv_label_set_text(local->info[i], "");
		lv_obj_set_size(local->info[i], GNSS_STATUS_INFO_W, GNSS_STATUS_INFO_H);
	}

	return list;
}

void gnss_status_loop()
{
	uint16_t map_w = lv_obj_get_width(local->map);
	uint16_t map_h = lv_obj_get_height(local->map);

	lv_label_set_text_fmt(local->info[2], "%u/%u", fc.gnss.sat_info.sat_used, fc.gnss.sat_info.sat_total);

	if (fc.gnss.fix > 1)
	{
		lv_label_set_text_fmt(local->info[0], "Fix %uD", fc.gnss.fix);
		lv_label_set_text_fmt(local->info[1], "TTF: %0.1fs", fc.gnss.ttf / 1000.0);
		lv_label_set_text_fmt(local->info[3], "Hor: %0.1fm", fc.gnss.horizontal_accuracy / 1000.0);
		lv_label_set_text_fmt(local->info[4], "Ver: %0.1fm", fc.gnss.vertical_accuracy / 1000.0);
		lv_label_set_text_fmt(local->info[5], "Alt: %0.1fm", fc.gnss.altitude_above_ellipsiod);
	}
	else
	{
		lv_label_set_text(local->info[0], "No Fix");
		lv_label_set_text(local->info[1], "TTF: ---");
		lv_label_set_text(local->info[3], "Hor: ---");
		lv_label_set_text(local->info[4], "Ver: ---");
		lv_label_set_text(local->info[5], "Alt: ---");
	}

	lv_obj_t * dot = lv_obj_get_child_back(local->map, NULL);
	lv_obj_t * sat = lv_obj_get_child_back(local->sats, local->info[GNSS_INFO_CNT - 1]);

	for (uint8_t i = 0; i < GNSS_NUMBER_OF_SATS; i++)
	{
		//data are avalible
		if (i < fc.gnss.sat_info.sat_total)
		{
			lv_obj_t * next_dot;
			lv_obj_t * next_sat;
			lv_obj_t * lab;
			lv_obj_t * bar;

			//dot does not exists
			if (dot == NULL)
			{
				//create new
				dot = lv_label_create(local->map, NULL);
				lv_obj_set_size(dot, GNSS_DOT_SIZE, GNSS_DOT_SIZE);
				lv_label_set_align(dot, LV_LABEL_ALIGN_CENTER);

				//bar indicator
				sat = lv_obj_create(local->sats, NULL);
				lv_obj_set_size(sat, GNSS_STATUS_INFO_W, GNSS_STATUS_INFO_H);

				lab = lv_label_create(sat, NULL);
				lv_label_set_align(lab, LV_LABEL_ALIGN_CENTER);
				lv_label_set_long_mode(lab, LV_LABEL_LONG_CROP);
				lv_obj_set_size(lab, 30, 16);
				lv_obj_align(lab, sat, LV_ALIGN_IN_LEFT_MID, 0, 0);

				bar = lv_bar_create(sat, NULL);
				lv_obj_set_size(bar, 35, 12);
				lv_obj_align(bar, sat, LV_ALIGN_IN_RIGHT_MID, 0, 0);

				//next dot will not exist
				next_dot = NULL;
			}
			else
			{
				next_dot = lv_obj_get_child_back(local->map, dot);

				bar = lv_obj_get_child(sat, NULL);
				lab = lv_obj_get_child(sat, bar);

				next_sat = lv_obj_get_child_back(local->sats, sat);
			}

			uint8_t c = 0;
			uint8_t s = STYLE_GPS;
			switch (fc.gnss.sat_info.sats[i].flags & GNSS_SAT_SYSTEM_MASK)
			{
				//GPS is default SBAS, IMES, QZSS are augmentation for GPS
				case(GNSS_SAT_SBAS):
					c = 'S';
				break;
				case(GNSS_SAT_IMES):
					c = 'I';
				break;
				case(GNSS_SAT_QZSS):
					c = 'Q';
				break;
				case(GNSS_SAT_GALILEO):
					s = STYLE_GALILEO;
				break;
				case(GNSS_SAT_BEIDOU):
					s = STYLE_BEIDOU;
				break;
				case(GNSS_SAT_GLONASS):
					s = STYLE_GLONASS;
				break;

			}

			//set style
			lv_obj_clean_style_list(dot, LV_LABEL_PART_MAIN);
			lv_obj_add_style(dot, LV_LABEL_PART_MAIN, &local->style_dots);
			lv_obj_add_style(dot, LV_LABEL_PART_MAIN, &local->style_color[s]);
			if (!(fc.gnss.sat_info.sats[i].flags & GNSS_SAT_USED))
				lv_obj_add_style(dot, LV_LABEL_PART_MAIN, &local->style_unused);

			lv_obj_clean_style_list(bar, LV_BAR_PART_INDIC);
			lv_obj_add_style(bar, LV_BAR_PART_INDIC, &local->style_color[s]);
			if (!(fc.gnss.sat_info.sats[i].flags & GNSS_SAT_USED))
				lv_obj_add_style(bar, LV_BAR_PART_INDIC, &local->style_unused);

			char text[8];
			if (c == 0)
				snprintf(text, sizeof(text), " %u ", fc.gnss.sat_info.sats[i].sat_id);
			else
				snprintf(text, sizeof(text), " %c%u ", c, fc.gnss.sat_info.sats[i].sat_id);

			lv_label_set_text(dot, text);
			lv_label_set_text(lab, text);
//			lv_bar_set_value(bar, 100, LV_ANIM_OFF);
			lv_bar_set_value(bar, fc.gnss.sat_info.sats[i].snr, LV_ANIM_OFF);

			float rad = to_radians(fc.gnss.sat_info.sats[i].azimuth * 2);

			//move dot
			uint16_t dot_w = lv_obj_get_width(dot);
			uint16_t dot_h = lv_obj_get_height(dot);
			uint16_t x = map_w / 2 + sin(rad) * (fc.gnss.sat_info.sats[i].elevation * map_w / 180) - dot_w / 2;
			uint16_t y = map_h / 2 + cos(rad) * (fc.gnss.sat_info.sats[i].elevation * map_h / 180) - dot_h / 2;
			lv_obj_set_pos(dot, x, y);

			dot = next_dot;
			sat = next_sat;
		}
		else
		{
			//dot exist
			while (dot != NULL)
			{
				lv_obj_t * old_dot = dot;
				dot = lv_obj_get_child_back(local->map, dot);
				lv_obj_del(old_dot);

				lv_obj_t * old_sat = sat;
				sat = lv_obj_get_child_back(local->sats, sat);
				lv_obj_del(old_sat);
			}
			break;
		}



	}

//	for (uint8_t i = 0; i < GNSS_NUMBER_OF_SYSTEMS; i++)
//	{
//		lv_obj_t * obj = lv_obj_get_child_back(local->sat[i], NULL);
//
//		for (uint8_t j = 0; j < GNSS_NUMBER_OF_SATS; j++)
//		{
//			uint8_t snr = fc.gnss.sat_info[i].sats[j].sat_id != 0 ? fc.gnss.sat_info[i].sats[j].snr : 0;
//
//			lv_bar_set_value(obj, snr, LV_ANIM_ON);
//			obj = lv_obj_get_child_back(local->sat[i], obj);
//		}
//
//		for (uint8_t j = 0; j < GNSS_NUMBER_OF_SATS; j++)
//		{
//			if (fc.gnss.sat_info[i].sats[j].sat_id)
//			{
//				lv_label_set_text_fmt(obj, "%02u", fc.gnss.sat_info[i].sats[j].sat_id);
//
//				float rad = to_radians(fc.gnss.sat_info[i].sats[j].azimuth * 2);
//
//				//move dot
//				uint16_t x = map_w / 2 + sin(rad) * (fc.gnss.sat_info[i].sats[j].elevation * map_w / 180) - GNSS_DOT_SIZE / 2;
//				uint16_t y = map_h / 2 + cos(rad) * (fc.gnss.sat_info[i].sats[j].elevation * map_h / 180) - GNSS_DOT_SIZE / 2;
//				lv_obj_set_pos(dot, x, y);
//
//				lv_obj_set_hidden(dot, false);
//			}
//			else
//			{
//				lv_obj_set_hidden(dot, true);
//				lv_label_set_text(obj, "");
//			}
//
//			obj = lv_obj_get_child_back(local->sat[i], obj);
//			dot = lv_obj_get_child_back(local->map, dot);
//		}
//	}
}
