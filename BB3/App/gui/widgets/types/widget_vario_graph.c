/*
 * widget_vario_graph.c
 *
 *  Created on: 06. 2. 2022
 *      Author: tilmann@bubecks.de
 */

#define DEBUG_LEVEL DBG_DEBUG

#include <limits.h>
#include "gui/widgets/widget.h"

REGISTER_WIDGET_ISU
(
    VarioGraph,
    "Vario Graph",
    WIDGET_MIN_W,
    WIDGET_MIN_H,
	_b(wf_label_hide),

	graph_t graph;
	uint16_t last_index;      // stores the last displayed fc.history.index to update only on change.
);

static void VarioGraph_init(lv_obj_t * base, widget_slot_t * slot)
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

	uint8_t units = config_get_select(&config.units.vario);
	switch (units)
    {
		case(VARIO_MPS):
			strcpy(local->graph.line_label_format, "%+d m/s");
			local->graph.line_alt1_step = 1;
			local->graph.line_alt2_step = 2;
			local->graph.line_alt3_step = 5;
			break;
		case(VARIO_KN):
			strcpy(local->graph.line_label_format, "%+d knots");
			local->graph.line_alt1_step = 2;
			local->graph.line_alt2_step = 6;
			local->graph.line_alt3_step = 10;
			break;
		case(VARIO_FPM):
			strcpy(local->graph.line_label_format, "%+d feet/m");
			local->graph.line_alt1_step = 200;
			local->graph.line_alt2_step = 600;
			local->graph.line_alt3_step = 1000;
			break;
    }

	DBG("VarioGraph: canvas_h=%d line_alt1/2/3_step=%d/%d/%d", local->graph.h, local->graph.line_alt1_step, local->graph.line_alt2_step, local->graph.line_alt3_step);

	local->last_index = UINT16_MAX;
}

static void VarioGraph_stop(widget_slot_t * slot)
{
	widget_remove_graph(&local->graph);
}

static void VarioGraph_update(widget_slot_t * slot)
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
	uint8_t units = config_get_select(&config.units.vario);

	while (i < fc.history.size)
	{
		uint16_t index = (fc.history.index + FC_HISTORY_SIZE - i) % FC_HISTORY_SIZE;
		fc_pos_history_t * pos = &fc.history.positions[index];

		bool valid = pos->flags & FC_POS_GNSS_3D;
		if ( valid )
		{
			// DBG("history.pos[%d]: gnss_alt=%d vario=%d", index, pos->gnss_alt, pos->vario);
			float vario = pos->vario / 100.0;   // cm/s -> m/s
			switch (units)
		    {
				case(VARIO_MPS):
					values[values_num] = vario;
					break;
				case(VARIO_KN):
					values[values_num] = vario * FC_MPS_TO_KNOTS;
					break;
				case(VARIO_FPM):
					values[values_num] = vario * FC_MPS_TO_100FPM * 100;
					break;
		    }

			values_max = max(values[values_num], values_max);
			values_min = min(values[values_num], values_min);

			// DBG("index=%d, values_num=%d, values[values_num]=%d, values_min=%d, values_max=%d", index, values_num, values[values_num], values_min, values_max);

			values_num++;

			if ( values_num >= GRAPH_NUM_PLOTS ) break;
		}
		i += GRAPH_HISTORY_STEP;
	}

#if 0
	// This is not so nice, as chart axis always jump around
	if ( values_min >= 0 )
		values_min = 0;    // raising all the time, keep 0 at bottom
	else if ( values_max <= 0 )
		values_max = 0;   // sinking all the time, keep 0 at top
	else
#endif
	{
		int abs_max = max(abs(values_min), abs(values_max));
		values_min = -abs_max;
		values_max = abs_max;
	}
	widget_update_graph(&local->graph, values, values_num, values_min, values_max);
}


