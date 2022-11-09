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

void draw_line(lv_obj_t * canvas, const lv_point_t points[], uint32_t point_cnt,
        const lv_draw_line_dsc_t * line_draw_dsc);

#endif /* GUI_LINE_H_ */
