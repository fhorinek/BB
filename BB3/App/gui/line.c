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


static inline float capsuleSDF(lv_coord_t px, lv_coord_t py, lv_coord_t ax, lv_coord_t ay, lv_coord_t bx, lv_coord_t by, float r) {
    int32_t pax = px - ax, pay = py - ay, bax = bx - ax, bay = by - ay;
    int32_t top = pax * bax + pay * bay;
    float bot = bax * bax + bay * bay;
    float h = fmaxf(fminf(top / (bot), 1.0f), 0.0f);
    int32_t dx = pax - bax * h, dy = pay - bay * h;
    return sqrtf(dx * dx + dy * dy) - r;
}


static inline void lineSDFAABB(lv_color_t * buf, lv_coord_t w, lv_coord_t ax, lv_coord_t ay, lv_coord_t bx, lv_coord_t by, float r, lv_color_t color)
{
    lv_coord_t x0 = max(0, min(w - 1, (min(ax, bx) - r)));
    lv_coord_t x1 = max(0, min(w - 1, (max(ax, bx) + r)));
    lv_coord_t y0 = max(0, min(w - 1, (min(ay, by) - r)));
    lv_coord_t y1 = max(0, min(w - 1, (max(ay, by) + r)));

    for (lv_coord_t y = y0; y <= y1; y++)
    {
        for (lv_coord_t x = x0; x <= x1; x++)
        {
            uint32_t index = w * y + x;
            lv_color_t * pixel = &buf[index];

            uint8_t opa = 255 * fmaxf(fminf(0.5f - capsuleSDF(x, y, ax, ay, bx, by, r/2.0), 1.0f), 0.0f);
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
                p1.x, p1.y, p2.x, p2.y, line_draw_dsc->width, line_draw_dsc->color);
        p1 = p2;
    }

}

void compute_line_arrow_points(lv_point_t p1, lv_point_t p2, lv_point_t *points)
{
	points[0] = p1;
	points[1] = p2;

	int dx = p2.x - p1.x;
	int dy = p2.y - p1.y;
	double angle = atan2(dy, dx);
	int arrow_size = 10;

	points[2].x = p2.x - arrow_size * cos(angle - 0.3);
	points[2].y = p2.y - arrow_size * sin(angle - 0.3);
	points[3] = p2;;
	points[4].x = p2.x - arrow_size * cos(angle + 0.3);
	points[4].y = p2.y - arrow_size * sin(angle + 0.3);
}
