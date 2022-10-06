/*
 * flightbook_flight_map.c
 *
 *  Created on: Feb 27, 2022
 *      Author: tilmann@bubecks.de
 *
 *  This display a track of a IGC on a map and in an altitude graph.
 */

#define DEBUG_LEVEL	DEBUG_DBG

#include <gui/tasks/menu/flightbook/flightbook_flight_map.h>
#include <gui/tasks/menu/flightbook/flightbook_flight.h>
#include <gui/tasks/menu/flightbook/flightbook.h>
#include "gui/gui_list.h"
#include "gui/map/tile.h"
#include "gui/map/map_obj.h"
#include "gui/map/map_thread.h"
#include "fc/fc.h"
#include "fc/agl.h"
#include "fc/logger/igc.h"
#include "drivers/tft/tft.h"
#include "etc/format.h"
#include "etc/geo_calc.h"
#include "gui/dialog.h"
#include "gui/tasks/filemanager.h"
#include "cloud_img.h"
#include "gui/polygon.h"

// Unable to include "widget.h" here, as this messes up with "local" variable.
// So write down external declaration here:
extern lv_color_t get_vario_color(int gain);

// How many points should we display? This is used for the track on the map and also for the altitude chart.
// Please make sure, that ALT_POINT_MULT is an integer (without fraction). To ensure this, TRACK_NUM_POINTS
// must be a divisior of TFT_WIDTH.
#define TRACK_NUM_POINTS 60

// If we have TRACK_NUM_POINTS in altitude chart, then each point has a distance of ALT_POINT_MULT
#define ALT_POINT_MULT (TFT_WIDTH/TRACK_NUM_POINTS)

// How many clouds do we want?
#define CLOUD_NUM 3

REGISTER_TASK_ILS(flightbook_flight_map,

		// This is data for the map
		map_obj_data_t data;
		int32_t lat_center, lon_center;
		uint8_t zoom;

		// The number of points used for the track.
		uint16_t points_num;

		// This is the track on the map. As we use vario colors,
		// we have multiple lines between two points in different colors.
		lv_point_t points_track[TRACK_NUM_POINTS];
		lv_obj_t *lines_track[TRACK_NUM_POINTS - 1];
		// This is the black line inside the colored track to clearly show the track.
		lv_obj_t *line_track;

		// This is the lv_obj to display the altitude graph below map
		lv_obj_t *graph_canvas;
		void * canvas_buf;

		// This is the line in the altitude graph to show the track
		lv_point_t points_alt[TRACK_NUM_POINTS];
		lv_obj_t *line_alt;

		// This is the position of the arrow/line if the user uses left/right
		// to navigate through the track. 0 <= arrow_pos <= points_num
		uint8_t arrow_pos;

		// The image of the arrow on the map showing current position
		lv_obj_t *arrow;

		// The vertical red line in the altitude graph showing current position
		lv_point_t points_arrow_line[2];
		lv_obj_t *arrow_line;

		// This stored the altitude at the track points to show in label
		int16_t altitude[TRACK_NUM_POINTS];

		// The label showing the current position statistics.
		lv_obj_t *label;

		// The clouds in the sky
		lv_obj_t *clouds[CLOUD_NUM];

		// Flag to indicate, that track is read and graphs are ready to be shown.
		bool initialized;

		// The previos position in filemanager to restore
		char file_path[PATH_LEN];
		uint8_t fm_return_level;
);

void flightbook_flight_map_cb(lv_obj_t * obj, lv_event_t event)
{
    switch (event)
    {
        case LV_EVENT_CANCEL:
            gui_switch_task(&gui_flightbook_flight, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
            //filemanager_get_current_level is only valid during filemanager callbacks
            //flightbook_flight_open(flightbook_flight_map_path, filemanager_get_current_level());
            flightbook_flight_open(local->file_path, local->fm_return_level);
        break;
        case LV_EVENT_KEY:
        {
            uint32_t key = *((uint32_t*) lv_event_get_data());
            if ((key == LV_KEY_RIGHT) && local->arrow_pos < local->points_num - 1)
            {
                local->arrow_pos++;
            }
            else if ((key == LV_KEY_LEFT) && local->arrow_pos > 0)
            {
                local->arrow_pos--;
            }
            break;
        }
    }
}

#define CLOUD_TIME_MIN     15000
#define CLOUD_TIME_DELTA   10000

void cloud_anim_done_cb(lv_anim_t * old)
{
    lv_obj_t * cloud = old->var;

    lv_coord_t x = -cloud_img.header.w;
    lv_coord_t y = (lv_coord_t) (random() % 20);

    lv_obj_set_pos(cloud, x, y);

    lv_anim_t a;
    lv_anim_init(&a);

    lv_anim_set_var(&a, cloud);
    lv_anim_set_values(&a, x, TFT_WIDTH + cloud_img.header.w);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_ready_cb(&a, cloud_anim_done_cb);
    lv_anim_set_time(&a, CLOUD_TIME_MIN + random() % CLOUD_TIME_DELTA);
    lv_anim_start(&a);
}

void flightbook_flight_map_load_task(void * param)
{
	int32_t fp;
    flight_stats_t f_stat;
    flight_pos_t pos;
    int16_t w, h;
    uint32_t pos_num = 0;
    uint32_t approx_pos_num;
    int16_t x, y;
    REDSTAT pStat;
    lv_obj_t *par;
    lv_coord_t graph_height;
    lv_coord_t graph_width;
    int16_t min_alt = INT16_MAX, max_alt = INT16_MIN;

    local->arrow_pos = 0;

    // Map is 3/4 of the vertical size and altitude graph is 1/4 below.
    // Create objects and set sizes/position:
	gui_lock_acquire();
    par = lv_obj_get_parent(local->data.map);
	lv_obj_set_size(local->data.map,  lv_obj_get_width(par), lv_obj_get_height(par) * 3 / 4);
	local->graph_canvas = lv_canvas_create(par, NULL);
	graph_width = lv_obj_get_width(par);
	graph_height = lv_obj_get_height(par) - lv_obj_get_height(local->data.map);
	lv_obj_set_size(local->graph_canvas,  graph_width, graph_height);
	lv_obj_set_pos(local->graph_canvas, 0, lv_obj_get_height(local->data.map));

	local->canvas_buf = ps_malloc(graph_width * graph_height * sizeof(lv_color_t));
	lv_canvas_set_buffer(local->graph_canvas, local->canvas_buf, graph_width, graph_height, LV_IMG_CF_TRUE_COLOR);
	lv_canvas_fill_bg(local->graph_canvas, lv_color_make(150, 150, 255), LV_OPA_COVER);
	gui_lock_release();

    logger_read_flight_stats(local->file_path, &f_stat);

    local->lat_center = ((int64_t)f_stat.min_lat + f_stat.max_lat) / 2;
    local->lon_center = ((int64_t)f_stat.min_lon + f_stat.max_lon) / 2;

    DBG("lat_max %ld lon_max %ld", f_stat.max_lat, f_stat.max_lon);
    DBG("lat_min %ld lon_min %ld", f_stat.min_lat, f_stat.min_lon);
    DBG("lat_c %ld lon_c %ld", local->lat_center, local->lon_center);

    // find right scaling:
    w = lv_obj_get_width(local->data.map);
    h = lv_obj_get_height(local->data.map);
    for (local->zoom = 0; local->zoom < 10; local->zoom++)
    {
    	geo_to_pix_w_h(local->lon_center, local->lat_center, local->zoom, f_stat.max_lon, f_stat.max_lat, &x, &y, w, h);
    	if ( x < 0 || x >= w || y < 0 || y >= h) continue;
    	geo_to_pix_w_h(local->lon_center, local->lat_center, local->zoom, f_stat.min_lon, f_stat.min_lat, &x, &y, w, h);
    	if ( x < 0 || x >= w || y < 0 || y >= h) continue;
    	break;
    }

    fp = red_open(local->file_path, RED_O_RDONLY);

    // Use filesize to approximately get number of "B" records:
    red_fstat(fp, &pStat);
    // 450 is header/footer size and 36 is size of 1 B Record:
    approx_pos_num = (pStat.st_size - 450) / 36;
    if ( approx_pos_num < 0 ) approx_pos_num = 1;

    float step = 0;
    if ( approx_pos_num < TRACK_NUM_POINTS )
    	step = 1.0;
    else
    	step = (float)approx_pos_num / TRACK_NUM_POINTS;

    int i = 0;
    pos_num = 0;
    float next_pos = step;
    int16_t previous_baro_alt;
    int16_t groundlevel;
    int16_t percent;
    int16_t last_percent = 0;

    lv_point_t * points_ground = (lv_point_t *) malloc(sizeof(lv_point_t) * (TRACK_NUM_POINTS + 3));

    while ( igc_read_next_pos(fp, &pos) )
    {
    	pos_num++;
    	percent = pos_num * 100 / approx_pos_num;
    	if (percent > last_percent + 5)
    	{
    		last_percent = percent;
    		dialog_progress_set_progress((uint8_t)percent);
    	}

    	if ( pos_num >= next_pos )
    	{
    		next_pos += step;
    		geo_to_pix_w_h(local->lon_center, local->lat_center, local->zoom, pos.lon, pos.lat, &x, &y, w, h);
    		local->points_track[i].x = x;
    		local->points_track[i].y = y;

    		groundlevel = agl_get_alt(pos.lat, pos.lon, true);
    		min_alt = min(min_alt, pos.baro_alt); min_alt = min(min_alt, groundlevel);
    		max_alt = max(max_alt, pos.baro_alt); max_alt = max(max_alt, groundlevel);

    		local->points_alt[i].x = i * ALT_POINT_MULT;
    		local->points_alt[i].y = pos.baro_alt;

    		points_ground[i].x = local->points_alt[i].x;
    		points_ground[i].y = groundlevel;

    		local->altitude[i] = pos.baro_alt;

    		if ( i > 0 )
    		{
    			lv_obj_t *line;
    			int16_t alt_diff;

    			alt_diff = pos.baro_alt - previous_baro_alt;

    			//calling anything with lv_ prefix outside GUI thread require GUI lock, otherwise we can corrupt LVGL memory
    			gui_lock_acquire();
    			line = lv_line_create(local->data.map, NULL);
    			lv_line_set_points(line, &local->points_track[i-1], 2);
    			lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, get_vario_color((int)(alt_diff / step * 100)));
    			lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 11);
    			gui_lock_release();

    			local->lines_track[i-1] = line;
    		}
    		previous_baro_alt = pos.baro_alt;
    		i++;
    	}
    }
    local->points_num = i;

    red_close(fp);

	gui_lock_acquire();

	// create black line showing track on map:
	local->line_track = lv_line_create(local->data.map, NULL);
	lv_line_set_points(local->line_track, &local->points_track[0], local->points_num);
	lv_obj_set_style_local_line_color(local->line_track, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_obj_set_style_local_line_width(local->line_track, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);

	// Create arrow on map:
	local->arrow = lv_img_create(local->data.map, NULL);
	lv_img_set_src(local->arrow, &img_map_arrow);
	lv_img_set_antialias(local->arrow, true);

	gui_lock_release();

	for ( i = 0; i < local->points_num; i++ )
	{
		// scale altitude lines according to size of graph:
		local->points_alt[i].y = graph_height - (lv_coord_t)(local->points_alt[i].y - min_alt) * (int32_t)graph_height / (max_alt - min_alt );               // int32_t is against integer overflow
		points_ground[i].y = graph_height - (lv_coord_t)(points_ground[i].y - min_alt) * (int32_t)graph_height / (max_alt - min_alt ); // int32_t is against integer overflow;

		// create vertical ground line
//		local->lines_ground[i] = lv_line_create(local->graph_canvas, NULL);
//		lv_line_set_points(local->lines_ground[i], &local->points_ground[i][0], 2);
//		lv_obj_set_style_local_line_color(local->lines_ground[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
//		lv_obj_set_style_local_line_width(local->lines_ground[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, ALT_POINT_MULT);
	}

    points_ground[local->points_num + 0].y = graph_height;
    points_ground[local->points_num + 0].x = graph_width;
    points_ground[local->points_num + 1].y = graph_height;
    points_ground[local->points_num + 1].x = 0;
    points_ground[local->points_num + 2].x = points_ground[0].x;
    points_ground[local->points_num + 2].y = points_ground[0].y;

    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.width = 1;
    dsc.color = LV_COLOR_GREEN;

    //polygon is locking gui by itself
	draw_polygon(local->graph_canvas, points_ground, local->points_num + 3, &dsc, graph_height);

	free(points_ground);



	gui_lock_acquire();
    srandom((int)HAL_GetTick());
    for (i = 0; i < CLOUD_NUM; i++)
    {
        local->clouds[i] = lv_img_create(local->graph_canvas, NULL);
        lv_coord_t x = (lv_coord_t) (TFT_WIDTH / 3) * i;
        lv_coord_t y = (lv_coord_t) (random() % 20);

        lv_img_set_src(local->clouds[i], &cloud_img);
        lv_obj_set_pos(local->clouds[i], x, y);
        lv_obj_set_style_local_image_recolor_opa(local->clouds[i], LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
        lv_obj_set_style_local_image_recolor(local->clouds[i], LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

        lv_anim_t a;
        lv_anim_init(&a);

        lv_anim_set_var(&a, local->clouds[i]);
        lv_anim_set_values(&a, x, TFT_WIDTH + cloud_img.header.w);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_set_ready_cb(&a, cloud_anim_done_cb);
        lv_anim_set_time(&a, CLOUD_TIME_MIN + random() % CLOUD_TIME_DELTA);
        lv_anim_start(&a);
    }

	// create track line in altitude graph:
	local->line_alt = lv_line_create(local->graph_canvas, NULL);
	lv_line_set_points(local->line_alt, &local->points_alt[0], local->points_num);
	lv_obj_set_style_local_line_color(local->line_alt, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_obj_set_style_local_line_width(local->line_alt, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 2);

	// Create vertical red line on altitude graph:
	local->arrow_line = lv_line_create(local->graph_canvas, NULL);
	local->points_arrow_line[0].x = local->points_arrow_line[0].y = local->points_arrow_line[1].x = local->points_arrow_line[1].y = 0;
	lv_line_set_points(local->arrow_line, &local->points_arrow_line[0], 2);
	lv_obj_set_style_local_line_color(local->arrow_line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
	lv_obj_set_style_local_line_width(local->arrow_line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_line_dash_gap(local->arrow_line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 5);
	lv_obj_set_style_local_line_dash_width(local->arrow_line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 10);

	// Create label:
	local->label = lv_label_create(local->graph_canvas, NULL);
	lv_obj_align(local->label, local->graph_canvas, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
	lv_obj_set_style_local_text_color(local->label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
	lv_label_set_align(local->label, LV_LABEL_ALIGN_RIGHT);

	gui_lock_release();

	local->initialized = true;

    dialog_close();
    gui_low_priority(false);

	vTaskDelete(NULL);
}

//we need to pass the return level for the filemanager that will be opened later during back operations
void flightbook_flight_map_load(char * path, uint8_t fm_return_level)
{
	strcpy(local->file_path, path);
	local->fm_return_level = fm_return_level;

	char file[REDCONF_NAME_MAX];
	filemanager_get_filename(file, path);
	dialog_show("Reading...", file, dialog_progress, NULL);
	dialog_progress_spin();
	gui_low_priority(true);

	//create function that will process the data in separate task, so GUI won't became unresponsive
	xTaskCreate((TaskFunction_t)flightbook_flight_map_load_task, "fb_map_task", 1024 * 2, NULL, osPriorityIdle + 1, NULL);
}

lv_obj_t * flightbook_flight_map_init(lv_obj_t * par)
{
	lv_obj_t *map;

	local->initialized = false;
	local->canvas_buf = NULL;

	map = map_obj_init(par, &local->data);

    gui_set_dummy_event_cb(par, flightbook_flight_map_cb);

    return map;
}

void flightbook_flight_map_loop()
{
    double dy, dx;

    map_set_static_pos(local->lat_center, local->lon_center, local->zoom);
    map_obj_loop(&local->data, local->lat_center, local->lon_center);

	if (!local->initialized ) return;

	// Set arrow and rotate into right direction
    lv_obj_set_pos(local->arrow, local->points_track[local->arrow_pos].x - img_map_arrow.header.w/2, local->points_track[local->arrow_pos].y - img_map_arrow.header.h/2);
    dx = local->points_track[local->arrow_pos].x - local->points_track[local->arrow_pos + 1].x;
    dy = local->points_track[local->arrow_pos].y - local->points_track[local->arrow_pos + 1].y;
    int16_t angle = (int16_t)(atan2(dy, dx) * (180/M_PI) + 270) % 360;
    lv_img_set_angle(local->arrow, (int16_t)(angle * 10.0));

    // set vertical line in altitude graph
    local->points_arrow_line[0].x = local->points_alt[local->arrow_pos].x;
    local->points_arrow_line[0].y = 0;
    local->points_arrow_line[1].x = local->points_alt[local->arrow_pos].x;
    local->points_arrow_line[1].y = lv_obj_get_height(local->graph_canvas);
    lv_line_set_points(local->arrow_line, &local->points_arrow_line[0], 2);

    // display label
    char label_text[50];
    format_altitude_with_units(label_text, (float)local->altitude[local->arrow_pos]);
    strcat(label_text, "\nAMSL");
    lv_label_set_text(local->label, label_text);
    lv_obj_align(local->label, local->graph_canvas, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

}

void flightbook_flight_map_stop()
{
    map_set_automatic_pos();

    if (local->canvas_buf != NULL)
        ps_free(local->canvas_buf);
}
