/*
 * widget_alt_graph.c
 *
 *  Created on: 31. 1. 2022
 *      Author: tilmann@bubecks.de
 */

#define DEBUG_LEVEL DBG_DEBUG

#include <limits.h>
#include "gui/widgets/widget.h"

REGISTER_WIDGET_ISU
(
    AltitudeGraph,
    "Altitude - Graph",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide) | _b(wf_hide_icon),

	graph_t graph;
	uint16_t last_index;      // stores the last displayed fc.history.index to update only on change.
);

static void AltitudeGraph_init(lv_obj_t * base, widget_slot_t * slot)
{
	int t_h;

	widget_create_base(base, slot);

	if (!widget_flag_is_set(slot, wf_label_hide))
	{
		widget_add_title(base, slot, slot->widget->name);
		t_h = lv_obj_get_height(slot->title);
	} else
		t_h = 0;

	local->graph.w = slot->w;
	local->graph.h = slot->h - t_h;

	widget_add_graph(base, slot, &local->graph);
	lv_obj_set_pos(local->graph.canvas, 0, t_h);

	uint8_t units = config_get_select(&config.units.altitude);
	strcpy(local->graph.line_label_format, "%d ");
	format_altitude_units_2(local->graph.line_label_format + 3, units);

	switch (units)
	{
		case ALTITUDE_M:
			local->graph.line_alt3_step = 100;
			local->graph.line_alt2_step = 50;
			local->graph.line_alt1_step = 10;
			break;
		case ALTITUDE_FT:
			local->graph.line_alt3_step = 500;
			local->graph.line_alt2_step = 100;
			local->graph.line_alt1_step = 25;
			break;
	}
	DBG("AltitudeGraph: canvas_h=%d line_alt1/2/3_step=%d/%d/%d", local->graph.h, local->graph.line_alt1_step, local->graph.line_alt2_step, local->graph.line_alt3_step);

	local->last_index = UINT16_MAX;
}

static void AltitudeGraph_stop(widget_slot_t * slot)
{
	widget_remove_graph(&local->graph);
}

static void AltitudeGraph_update(widget_slot_t * slot)
{
	int values_num = 0;
	float values_min = INT_MAX;
	float values_max = INT_MIN;
	float values[GRAPH_NUM_PLOTS];

	if ( fc.history.index == local->last_index) return; // Nothing has changed.
	local->last_index = fc.history.index;

	/*
	 * [0] Collect values.
	 */
	int16_t i = GRAPH_HISTORY_STEP * 2; // skip newest 2 history positions
	uint8_t units = config_get_select(&config.units.altitude);

	while (i < fc.history.size)
	{
		uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i) % FC_HISTORY_SIZE;
		fc_pos_history_t * pos = &fc.history.positions[index];

		bool valid = pos->flags & FC_POS_GNSS_3D;
		if ( valid )
		{
			values[values_num] = pos->gnss_alt;
			if (units == ALTITUDE_FT)
				values[values_num] *= FC_METER_TO_FEET;

			values_max = max(values[values_num], values_max);
			values_min = min(values[values_num], values_min);

			// DBG("index=%d, values_num=%d, values[values_num]=%d, values_min=%d, values_max=%d", index, values_num, values[values_num], values_min, values_max);

			values_num++;

			if ( values_num >= GRAPH_NUM_PLOTS ) break;
		}
		i += GRAPH_HISTORY_STEP;
	}

	widget_update_graph(slot, &local->graph, values, values_num, values_min, values_max);
}


