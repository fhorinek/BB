/*
 * polygon.c
 *
 *  Created on: Feb 3, 2022
 *      Author: horinek
 */

#include "common.h"
//#include "gui/gui.h"

typedef struct
{
    int16_t y_min;
    int16_t y_max;
    float x_val;
    float slope;
} polygon_edge_t;

void draw_polygon(lv_obj_t * canvas, lv_point_t * points, uint16_t number_of_points, lv_draw_line_dsc_t * draw_desc)
{
    polygon_edge_t * edges = (polygon_edge_t * ) malloc(sizeof(polygon_edge_t) * number_of_points);

    uint16_t edge_cnt = 0;

    //get valid edges
    for (uint16_t i = 0; i < number_of_points - 1; i++)
    {
        polygon_edge_t tmp;

        //multipolygon separator
        if (points[i + 1].x == 0x7FFF && points[i + 1].y == 0x7FFF)
        {
            i += 2;
        }

        if (points[i].y != points[i + 1].y)
        {
            if (points[i].y < points[i + 1].y)
            {
                tmp.x_val = points[i].x;
                tmp.y_min = points[i].y;
                tmp.y_max = points[i + 1].y;
            }
            else
            {
                tmp.x_val = points[i + 1].x;
                tmp.y_min = points[i + 1].y;
                tmp.y_max = points[i].y;
            }
            tmp.slope = (points[i + 1].x - points[i].x) / (float)(points[i + 1].y - points[i].y);
            int64_t tmp_value = (int64_t)tmp.y_min << 32;
            tmp_value += (int64_t)tmp.y_max << 16;
            tmp_value += tmp.x_val;

            for (uint16_t j = 0; j < edge_cnt + 1; j++)
            {
                int64_t edge_value = (int64_t)edges[j].y_min << 32;
                edge_value += (int64_t)edges[j].y_max << 16;
                edge_value += edges[j].x_val;

                if  (edge_cnt == j)
                {
                    memcpy((void *)&edges[j], (void *)&tmp, sizeof(polygon_edge_t));
                }
                else if (edge_value > tmp_value)
                {
                    __align polygon_edge_t move;
                    memcpy((void *)&move, (void *)&edges[j], sizeof(polygon_edge_t));
                    memcpy((void *)&edges[j], (void *)&tmp, sizeof(polygon_edge_t));

                    for (uint16_t k = j + 1; k < edge_cnt + 1; k++)
                    {
                        memcpy((void *)&tmp, (void *)&edges[k], sizeof(polygon_edge_t));
                        memcpy((void *)&edges[k], (void *)&move, sizeof(polygon_edge_t));
                        memcpy((void *)&move,  (void *)&tmp, sizeof(polygon_edge_t));
                    }
                    break;
                }
            }

            edge_cnt++;
        }
    }

//  for (uint16_t i = 0; i < edge_cnt; i++)
//  {
//      INFO("%u: %d %d %0.1f %0.3f", i, edges[i].y_min, edges[i].y_max, edges[i].x_val, edges[i].slope);
//  }

    if (edge_cnt > 0)
    {
        int16_t scan_start = edges[0].y_min;
        int16_t scan_end = edges[edge_cnt - 1].y_max;

        uint16_t * active = (uint16_t *) malloc(sizeof(uint16_t) * edge_cnt);

        for (int16_t scan_line = scan_start; scan_line < scan_end + 1; scan_line++)
        {
            //get active
            uint16_t active_cnt = 0;
            for (uint16_t i = 0; i < edge_cnt; i++)
            {
                if (edges[i].y_min <= scan_line && edges[i].y_max > scan_line)
                {
                    for (uint16_t j = 0; j < active_cnt + 1; j++)
                    {
                        bool pass = false;

                        if (j < active_cnt)
                        {
                            if (edges[active[j]].x_val > edges[i].x_val)
                                pass = true;
                        }
                        else if (active_cnt == j)
                        {
                            pass = true;
                        }

                        if (pass)
                        {
                            uint16_t move;
                            move = active[j];
                            active[j] = i;

                            for (uint16_t k = j + 1; k < active_cnt + 1; k++)
                            {
                                uint16_t tmp = active[k];
                                active[k] = move;
                                move = tmp;
                            }
                            break;
                        }
                    }

                    active_cnt++;
                }
            }

            //draw active
            lv_point_t line_points[2];
            for (uint16_t i = 0; i < active_cnt; i++)
            {
                line_points[i % 2].x = edges[active[i]].x_val;
                line_points[i % 2].y = scan_line;

                if (i % 2 == 1)
                {
                    //TODO: draw directly to memory without lvgl
                    gui_lock_acquire();
                    lv_canvas_draw_line(canvas, line_points, 2, draw_desc);
                    gui_lock_release();
                }

                edges[active[i]].x_val += edges[active[i]].slope;
            }
        }

        free(active);
    }
    free(edges);
}
