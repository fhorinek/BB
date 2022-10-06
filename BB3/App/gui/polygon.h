/*
 * polygon.h
 *
 *  Created on: 6. 10. 2022
 *      Author: horinek
 */

#ifndef GUI_POLYGON_H_
#define GUI_POLYGON_H_

#include "common.h"
#include "gui/gui.h"

void draw_polygon(lv_obj_t * canvas, lv_point_t * points, uint16_t number_of_points, lv_draw_line_dsc_t * draw_desc, lv_coord_t max_height);

#endif /* GUI_POLYGON_H_ */
