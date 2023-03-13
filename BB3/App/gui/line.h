/*
 * line.h
 *
 *  Created on: 9. 11. 2022
 *      Author: horinek
 */

#ifndef GUI_LINE_H_
#define GUI_LINE_H_

#include "common.h"
#include "gui.h"

// How many points do we need to store arrow points with compute_line_arrow_points
#define LINE_ARROW_NUM_POINTS 5

void draw_line(lv_obj_t * canvas, const lv_point_t points[], uint32_t point_cnt,
        const lv_draw_line_dsc_t * line_draw_dsc);

void compute_line_arrow_points(lv_point_t p1, lv_point_t p2, lv_point_t *points);

#endif /* GUI_LINE_H_ */
