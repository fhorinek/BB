/*
 * widget.h
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#ifndef GUI_WIDGETS_WIDGET_H_
#define GUI_WIDGETS_WIDGET_H_

#include "widgets.h"
#include "fc/fc.h"
#include "etc/format.h"

void widget_create_base(lv_obj_t * base, widget_slot_t * slot);
void widget_add_title(lv_obj_t * base, widget_slot_t * slot, char * title);
lv_obj_t * widget_add_value(lv_obj_t * base, widget_slot_t * slot, char * unit, lv_obj_t ** unit_obj);

void widget_update_font_size_box(lv_obj_t * label, lv_coord_t w, lv_coord_t h);
void widget_update_font_size(lv_obj_t * label);

void widget_arrow_rotate(lv_obj_t * arrow, lv_point_t * points, int16_t angle);
void widget_arrow_rotate_size(lv_obj_t * arrow, lv_point_t * points, int16_t angle, uint8_t s);
lv_obj_t * widget_add_arrow(lv_obj_t * base, widget_slot_t * slot, lv_point_t * points, char * unit, lv_obj_t ** unit_obj);

lv_obj_t * widget_create_edit_overlay(char * title, char * message);
void widget_destroy_edit_overlay(lv_obj_t * base);
lv_obj_t * widget_edit_overlay_get_base(lv_obj_t * edit);
void widget_reset_edit_overlay_timer();

// Number of seconds, that we plot in this graph
#define GRAPH_NUM_PLOTS 30
#define GRAPH_HISTORY_STEP (1000 / FC_HISTORY_PERIOD)

typedef struct _graph_t
{
	uint16_t w, h;

	lv_obj_t * canvas;        // Canvas to paint to
    lv_color_t *cbuf;         // associated buffer of the canvas

    uint16_t line_alt1_step;     // step for line_alt1, typically  10m depending on canvas_h
    uint16_t line_alt2_step;     // step for line_alt2, typically  50m depending on canvas_h
    uint16_t line_alt3_step;     // step for line_alt3, typically 100m depending on canvas_h

    char line_label_format[15];     // e.g. "%+d m/s"
} graph_t;

extern const lv_img_dsc_t glider;

void widget_add_graph(lv_obj_t * base, widget_slot_t * slot, graph_t *graph);
void widget_update_graph(graph_t * graph, float values[], int values_num, float values_min, float values_max);
void widget_remove_graph(graph_t * graph);

lv_color_t get_vario_color(int gain);
lv_color_t get_vario_color2(int gain);

#define local	((widget_local_vars_t *)slot->vars)

#endif /* GUI_WIDGETS_WIDGET_H_ */
