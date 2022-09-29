/*
 * flightbook_flight_map.c
 *
 *  Created on: Feb 27, 2022
 *      Author: tilmann@bubecks.de
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
#include "fc/logger/igc.h"
#include "etc/format.h"
#include "etc/geo_calc.h"

lv_color_t get_vario_color(int gain);
lv_color_t get_vario_color2(int gain);

#define TRACK_NUM_POINTS 50

REGISTER_TASK_ILS(flightbook_flight_map,
		map_obj_data_t data;
		int32_t lat_center, lon_center;
		uint8_t zoom;
		lv_point_t points[TRACK_NUM_POINTS];
		uint16_t points_num;
		lv_obj_t *lines[TRACK_NUM_POINTS];
);

char flightbook_flight_map_path[PATH_LEN];

void flightbook_flight_map_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CANCEL) {
		gui_switch_task(&gui_flightbook_flight, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
		flightbook_flight_open(flightbook_flight_map_path, filemanager_get_current_level());
	}
}

lv_obj_t * flightbook_flight_map_init(lv_obj_t * par)
{
	int32_t fp;
	lv_obj_t *map;
    flight_stats_t f_stat;
    lv_obj_t *line;
    flight_pos_t pos;
    int w, h;
    int pos_num = 0;
    int16_t x, y;
    REDSTAT pStat;

	map = map_obj_init(par, &local->data);

    logger_read_flight_stats(flightbook_flight_map_path, &f_stat);

    local->lat_center = (f_stat.min_lat + f_stat.max_lat) / 2;
    local->lon_center = (f_stat.min_lon + f_stat.max_lon) / 2;

    DBG("lat_max %ld lon_max %ld", f_stat.max_lat, f_stat.max_lon);
    DBG("lat_min %ld lon_min %ld", f_stat.min_lat, f_stat.min_lon);
    DBG("lat_c %ld lon_c %ld", local->lat_center, local->lon_center);

    // find right scaling:
    w = lv_obj_get_width(par);
    h = lv_obj_get_height(par);
    for ( local->zoom = 0; local->zoom < 10; local->zoom++ )
    {
    	geo_to_pix_w_h(local->lon_center, local->lat_center, local->zoom, f_stat.max_lon, f_stat.max_lat, &x, &y, w, h);
    	if ( x < 0 || x >= w || y < 0 || y >= h) continue;
    	geo_to_pix_w_h(local->lon_center, local->lat_center, local->zoom, f_stat.min_lon, f_stat.min_lat, &x, &y, w, h);
    	if ( x < 0 || x >= w || y < 0 || y >= h) continue;
    	break;
    }

    fp = red_open(flightbook_flight_map_path, RED_O_RDONLY);

    // Use filesize to approximately get number of "B" records:
    red_fstat(fp, &pStat);
    // 450 is header/footer size and 36 is size of 1 B Record:
    pos_num = (pStat.st_size - 450) / 36;
    if ( pos_num < 0 ) pos_num = 1;

    float step = 0;
    if ( pos_num < TRACK_NUM_POINTS )
    	step = 1.0;
    else
    	step = (float)pos_num / TRACK_NUM_POINTS;

    int i = 0;
    pos_num = 0;
    float next_pos = step;
    int16_t previous_gnss_alt;

    while ( igc_read_next_pos(fp, &pos) )
    {
    	pos_num++;
    	if ( pos_num >= next_pos )
    	{
    		next_pos += step;
    		geo_to_pix_w_h(local->lon_center, local->lat_center, local->zoom, pos.lon, pos.lat, &x, &y, w, h);
    		local->points[i].x = x;
    		local->points[i].y = y;
    		if ( i > 0 )
    		{
    			lv_obj_t *line;
    			int16_t alt_diff;

    			alt_diff = pos.gnss_alt - previous_gnss_alt;

    			line = lv_line_create(map, NULL);
    			lv_line_set_points(line, &local->points[i-1], 2);
    			lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, get_vario_color((int)(alt_diff / step * 100)));
    			lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 11);

    			local->lines[i-1] = line;
    		}
    		previous_gnss_alt = pos.gnss_alt;
    		i++;
    	}
    }
    local->points_num = i;


    line = lv_line_create(map, NULL);
    lv_line_set_points(line, &local->points[0], local->points_num);
    lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);

    red_close(fp);

    gui_set_dummy_event_cb(par, flightbook_flight_map_cb);

	return map;
}

void flightbook_flight_map_loop()
{
    // disp_lat = 449996718; disp_lon = 130009586;   // Bassano
    //disp_lat=  485547480; disp_lon =  93919890;   // Hohenneuffen
	//           479964285 480000036                      90035779
    map_set_static_pos(local->lat_center, local->lon_center, local->zoom);
    map_obj_loop(&local->data, local->lat_center, local->lon_center);
}

void flightbook_flight_map_stop()
{
    map_set_automatic_pos();
}


