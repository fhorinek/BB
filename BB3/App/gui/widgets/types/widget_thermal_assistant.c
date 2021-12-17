/*
 * widget_flight_time.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */


#include "gui/widgets/widget.h"
#include "fc/agl.h"

REGISTER_WIDGET_IU
(
    TAss,
    "Thermal assistant",
    120,
    120,
	_b(wf_label_hide),

    lv_obj_t * text;
    lv_obj_t * circle[8];

    lv_obj_t * arrow;
    lv_point_t arrow_points[WIDGET_ARROW_POINTS];
);

static void TAss_init(lv_obj_t * base, widget_slot_t * slot)
{
    widget_create_base(base, slot);
    if (!widget_flag_is_set(slot, wf_label_hide))
    	widget_add_title(base, slot, "Thermal assistant");

    local->text = widget_add_value(base, slot, NULL, NULL);

    int16_t s = min(slot->w, slot->h);

    for (uint8_t i = 0; i < 8; i++)
    {
        float angle = to_radians(90 + i * 45);

        int16_t x = -cos(angle) * s / 3;
        int16_t y = -sin(angle) * s / 3;

        local->circle[i] = lv_obj_create(slot->obj, NULL);
        lv_obj_set_style_local_radius(local->circle[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
        lv_obj_align(local->circle[i], local->text, LV_ALIGN_CENTER, x, y);
        lv_obj_set_auto_realign(local->circle[i], true);
    }

    local->arrow = widget_add_arrow(base, slot, local->arrow_points, NULL, NULL);
}

#define MIN_DOT_SIZE    5

static void TAss_update(widget_slot_t * slot)
{
    char value[16];

    if (fc.flight.circling)
    {
        lv_obj_set_hidden(local->arrow, false);

        format_vario(value, fc.fused.avg_vario);
        lv_label_set_text(local->text, value);
        widget_update_font_size_box(local->text, slot->w / 2, slot->h / 2);

        uint8_t max_val = VARIO_CIRCLING_HISTORY_SCALE; // +/- 1m
        int16_t s = min(slot->w, slot->h);

        for(uint8_t i = 0; i < 8; i++)
        {
            max_val = max(max_val, abs(fc.flight.circling_history[i]));
        }

        for (uint8_t i = 0; i < 8; i++)
        {
            uint8_t j;
            if (fc.flight.avg_heading_change >= 0)
            {
                //circling clockwise
                j = (i + 2) % 8;
            }
            else
            {
                //circling counter clockwise
                j = (i + 6) % 8;
            }

            if (j == (((uint16_t)fc.gnss.heading + 22) % 360) / 45)
            {
                lv_obj_set_hidden(local->circle[i], true);

                float angle;
                if (fc.flight.avg_heading_change >= 0)
                {
                    //cw
                    angle = to_radians(fc.gnss.heading);
                }
                else
                {
                    //ccw
                    angle = to_radians(fc.gnss.heading + 180);
                }

                int16_t x = -cos(angle) * s / 3;
                int16_t y = -sin(angle) * s / 3;

                widget_arrow_rotate_size(local->arrow, local->arrow_points, fc.gnss.heading, s / 3);
                lv_obj_align(local->arrow, local->text, LV_ALIGN_CENTER, x, y);
            }
            else
            {
                lv_obj_set_hidden(local->circle[i], false);
                int16_t rad = (fc.flight.circling_history[j] * (s / 5)) / max_val;
                if (rad == 0)
                    lv_obj_set_style_local_bg_color(local->circle[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
                else if (rad > 0)
                    lv_obj_set_style_local_bg_color(local->circle[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
                else
                    lv_obj_set_style_local_bg_color(local->circle[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

                rad = abs(rad) + MIN_DOT_SIZE;
                lv_obj_set_size(local->circle[i], rad, rad);
            }
        }
    }
    else
    {
        lv_obj_set_hidden(local->arrow, true);
        lv_label_set_text(local->text, "Not\ncircling");
        widget_update_font_size_box(local->text, slot->w / 2, slot->h / 2);

        for (uint8_t i = 0; i < 8; i++)
        {

            lv_obj_set_hidden(local->circle[i], false);
            lv_obj_set_size(local->circle[i], MIN_DOT_SIZE, MIN_DOT_SIZE);
            lv_obj_set_style_local_bg_color(local->circle[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        }
    }

}


