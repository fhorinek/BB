/*
 * line.c
 *
 *  Created on: 9. 11. 2022
 *      Author: horinek
 */

#include "line.h"

#include <math.h>   // ceilf(), floorf(), fminf(), fmaxf(), sinf(), cosf(), sqrtf()
#include <string.h> // memset()
#define PI 3.14159265359f


float capsuleSDF(float px, float py, float ax, float ay, float bx, float by, float r) {
    float pax = px - ax, pay = py - ay, bax = bx - ax, bay = by - ay;
    float h = fmaxf(fminf((pax * bax + pay * bay) / (bax * bax + bay * bay), 1.0f), 0.0f);
    float dx = pax - bax * h, dy = pay - bay * h;
    return sqrtf(dx * dx + dy * dy) - r;
}


void lineSDFAABB(lv_color_t * buf, lv_coord_t w, float ax, float ay, float bx, float by, float r, lv_color_t color)
{
    int x0 = (int)floorf(fminf(ax, bx) - r);
    int x1 = (int) ceilf(fmaxf(ax, bx) + r);
    int y0 = (int)floorf(fminf(ay, by) - r);
    int y1 = (int) ceilf(fmaxf(ay, by) + r);

    x0 = max(0, min(w - 1, x0));
    x1 = max(0, min(w - 1, x1));
    y0 = max(0, min(w - 1, y0));
    y1 = max(0, min(w - 1, y1));

    for (int y = y0; y <= y1; y++)
    {
        for (int x = x0; x <= x1; x++)
        {
            uint32_t index = w * y + x;
            lv_color_t * pixel = &buf[index];

            uint8_t opa = 255 * fmaxf(fminf(0.5f - capsuleSDF(x, y, ax, ay, bx, by, r), 1.0f), 0.0f);
            *pixel = lv_color_mix(color, *pixel, opa);
        }
    }
}


void draw_line(lv_obj_t * canvas, const lv_point_t points[], uint32_t point_cnt,
        const lv_draw_line_dsc_t * line_draw_dsc)
{
    lv_img_dsc_t * img_dsc = lv_canvas_get_img(canvas);

    lv_point_t p1, p2;

    p1 = points[0];

    for (uint16_t i = 1; i < point_cnt; i++)
    {
        p2 = points[i];
        lineSDFAABB((lv_color_t *)img_dsc->data, img_dsc->header.w,
                p1.x, p1.y, p2.x, p2.y, line_draw_dsc->width / 2.0, line_draw_dsc->color);
        p1 = p2;
    }

}
