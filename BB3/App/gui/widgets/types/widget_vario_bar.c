/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

#define SEGMENTS_CNT  6


REGISTER_WIDGET_IU(
    Bar,
    _i("Vario - bar"),
    16,
    80,
	0,

    lv_obj_t * bar_vario;
    lv_obj_t * bar_avg;
    lv_obj_t * lines[SEGMENTS_CNT - 1];
    lv_obj_t * labels[SEGMENTS_CNT - 1];

    int8_t offset;
);

static bool static_init = false;
static lv_style_t static_line = {0};
static lv_style_t static_label = {0};

static void Bar_update_range(widget_slot_t * slot)
{
    for (uint8_t i = 0; i < SEGMENTS_CNT - 1; i++)
    {
        int8_t val = -(i -(SEGMENTS_CNT / 2) + 1) + local->offset;
        int16_t y = (slot->h / (SEGMENTS_CNT)) * (i + 1);

        if (val != 0)
        {
            lv_obj_set_pos(local->lines[i], 0, y);
            lv_obj_set_size(local->lines[i], slot->w, 3);
            switch (config_get_select(&config.units.vario))
            {
				case(VARIO_MPS):
					lv_label_set_text_fmt(local->labels[i], "%+d", val);
				break;
				case(VARIO_KN):
					lv_label_set_text_fmt(local->labels[i], "%+d", val * 2);
				break;
				case(VARIO_FPM):
					lv_label_set_text_fmt(local->labels[i], "%+d", val * 200);
				break;
            }
        }
        else
        {
            lv_obj_set_pos(local->lines[i], 0, y - 1);
            lv_obj_set_size(local->lines[i], slot->w, 5);
            lv_label_set_text(local->labels[i], "0");
        }

        lv_obj_realign(local->labels[i]);
    }
}

static void Bar_init(lv_obj_t * base, widget_slot_t * slot)
{
    if (!static_init)
    {
        lv_style_init(&static_line);
        lv_style_set_bg_color(&static_line, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_style_set_border_color(&static_line, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_style_set_border_width(&static_line, LV_STATE_DEFAULT, 1);

        lv_style_init(&static_label);
        lv_style_set_text_font(&static_label, LV_STATE_DEFAULT, &lv_font_montserrat_12);
        lv_style_set_text_color(&static_label, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        static_init = true;
    }

    local->offset = 0;

    widget_create_base(base, slot);

    local->bar_vario = lv_obj_create(slot->obj, NULL);
    lv_obj_set_x(local->bar_vario, 1);
    lv_obj_set_width(local->bar_vario, slot->w - 2);

    local->bar_avg = lv_obj_create(slot->obj, NULL);
    lv_obj_set_x(local->bar_avg, 1);
    lv_obj_set_width(local->bar_avg, slot->w / 2 - 1);

    for (uint8_t i = 0; i < SEGMENTS_CNT - 1; i++)
    {
        local->lines[i] = lv_obj_create(slot->obj, NULL);
        lv_obj_add_style(local->lines[i], LV_OBJ_PART_MAIN, &static_line);

        local->labels[i] = lv_label_create(slot->obj, NULL);
        lv_obj_add_style(local->labels[i], LV_LABEL_PART_MAIN, &static_label);
        lv_obj_align(local->labels[i], local->lines[i], LV_ALIGN_IN_BOTTOM_RIGHT, -3, -3);
    }

    Bar_update_range(slot);
}

static void Bar_update(widget_slot_t * slot)
{
    int8_t old_offset = local->offset;

    float vario = fc.fused.vario;
    float avg_vario = fc.fused.avg_vario;

    switch (config_get_select(&config.units.vario))
    {
		case(VARIO_KN): //one slot == 2 kn
			vario = vario * 0.97192;
			avg_vario = avg_vario * 0.97192;
		break;
		case(VARIO_FPM): //one slot == 200fpm
			vario = vario * 0.98425;
			avg_vario = avg_vario * 0.98425;
		break;
    }

    if (abs(vario) >= 9.0)
    	local->offset = 9 * (vario / abs(vario));
    else if (abs(vario) >= 6.0)
    	local->offset = 6 * (vario / abs(vario));
    else if (abs(vario) >= 3.0)
    	local->offset = 3 * (vario / abs(vario));
    else if (abs(vario) <= 1.0)
    	local->offset = 0;

    if (old_offset != local->offset)
    {
    	Bar_update_range(slot);
    }

	int16_t y1 = slot->h / 2 - (slot->h / SEGMENTS_CNT) * -local->offset;
    int16_t y2 = (slot->h / SEGMENTS_CNT) * -avg_vario;

    if (y2 < 0)
    {
        lv_obj_set_style_local_bg_color(local->bar_avg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_darken(LV_COLOR_GREEN, LV_OPA_70));
        lv_obj_set_y(local->bar_avg, y1 + y2);
        lv_obj_set_height(local->bar_avg, -y2);
    }
    else
    {
        lv_obj_set_style_local_bg_color(local->bar_avg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_darken(LV_COLOR_RED, LV_OPA_70));
        lv_obj_set_y(local->bar_avg, y1);
        lv_obj_set_height(local->bar_avg, y2);
    }

    y2 = (slot->h / SEGMENTS_CNT) * -vario;

    if (y2 < 0)
    {
        lv_obj_set_style_local_bg_color(local->bar_vario, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
        lv_obj_set_y(local->bar_vario, y1 + y2);
        lv_obj_set_height(local->bar_vario, -y2);
    }
    else
    {
        lv_obj_set_style_local_bg_color(local->bar_vario, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
        lv_obj_set_y(local->bar_vario, y1);
        lv_obj_set_height(local->bar_vario, y2);
    }
}

