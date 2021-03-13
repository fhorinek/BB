/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"

#define SEGMENTS_CNT  6


REGISTER_WIDGET_ISU(Bar,
    "Vario bar",
    24,
    128,
    lv_obj_t * bar_vario;
    lv_obj_t * bar_avg;
    lv_obj_t * lines[SEGMENTS_CNT - 1];
    lv_obj_t * labels[SEGMENTS_CNT - 1];

    lv_style_t line;
    lv_style_t label;

    int8_t offset;
);

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
            lv_label_set_text_fmt(local->labels[i], "%+d", val);
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
    local->offset = 0;

    widget_create_base(base, slot);

    lv_style_init(&local->line);
    lv_style_set_bg_color(&local->line, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_border_color(&local->line, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_border_width(&local->line, LV_STATE_DEFAULT, 1);

    lv_style_init(&local->label);
    lv_style_set_text_font(&local->label, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_style_set_text_color(&local->label, LV_STATE_DEFAULT, LV_COLOR_WHITE);

    local->bar_vario = lv_obj_create(slot->obj, NULL);
    lv_obj_set_x(local->bar_vario, 1);
    lv_obj_set_width(local->bar_vario, slot->w - 2);

    local->bar_avg = lv_obj_create(slot->obj, NULL);
    lv_obj_set_x(local->bar_avg, 1);
    lv_obj_set_width(local->bar_avg, slot->w / 3 - 1);

    for (uint8_t i = 0; i < SEGMENTS_CNT - 1; i++)
    {
        int16_t y = (slot->h / (SEGMENTS_CNT)) * (i + 1);

        local->lines[i] = lv_obj_create(slot->obj, NULL);
        lv_obj_add_style(local->lines[i], LV_OBJ_PART_MAIN, &local->line);

        local->labels[i] = lv_label_create(slot->obj, NULL);
        lv_obj_add_style(local->labels[i], LV_LABEL_PART_MAIN, &local->label);
        lv_obj_align(local->labels[i], local->lines[i], LV_ALIGN_IN_BOTTOM_RIGHT, -3, -3);
    }

    Bar_update_range(slot);
}

static void Bar_update(widget_slot_t * slot)
{
    //TODO: change offests
    int16_t y1 = slot->h / 2 - (slot->h / SEGMENTS_CNT) * local->offset;
    int16_t y2 = (slot->h / SEGMENTS_CNT) * -fc.fused.avg_vario;

    if (y2 < 0)
    {
        lv_obj_set_style_local_bg_color(local->bar_avg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_darken(LV_COLOR_GREEN, LV_OPA_60));
        lv_obj_set_y(local->bar_avg, y1 + y2);
        lv_obj_set_height(local->bar_avg, -y2);
    }
    else
    {
        lv_obj_set_style_local_bg_color(local->bar_avg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_darken(LV_COLOR_RED, LV_OPA_60));
        lv_obj_set_y(local->bar_avg, y1);
        lv_obj_set_height(local->bar_avg, y2);
    }

    y2 = (slot->h / SEGMENTS_CNT) * -fc.fused.vario;

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

static void Bar_stop(widget_slot_t * slot)
{
    for (uint8_t i = 0; i < SEGMENTS_CNT - 1; i++)
    {
        lv_obj_remove_style(local->lines[i], LV_OBJ_PART_MAIN, &local->line);
        lv_obj_remove_style(local->labels[i], LV_LABEL_PART_MAIN, &local->label);
    }

    lv_style_reset(&local->line);
    lv_style_reset(&local->label);
}
