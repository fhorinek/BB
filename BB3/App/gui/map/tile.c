/*
 * tile.c
 *
 *  Created on: 10. 11. 2020
 *      Author: horinek
 */

#include "tile.h"
#include "linked_list.h"
#include "map_thread.h"

#include "fc/agl.h"

float lat_mult[] = {
	1.00001269263410,
	1.00016503034732,
	1.00062227564598,
	1.00138512587137,
	1.00245474614468,
	1.00383277371586,
	1.00552132409524,
	1.00752299900955,
	1.00984089623881,
	1.01247862140355,
	1.01544030178698,
	1.01873060229028,
	1.02235474363937,
	1.02631852297526,
	1.03062833698348,
	1.03529120773669,
	1.04031481145009,
	1.04570751037551,
	1.05147838808628,
	1.05763728844289,
	1.06419485855898,
	1.07116259613477,
	1.07855290156473,
	1.08637913528301,
	1.09465568086611,
	1.10339801447867,
	1.11262278132588,
	1.12234787986046,
	1.13259255459192,
	1.14337749845791,
	1.15472496584871,
	1.16665889752463,
	1.17920505883836,
	1.19239119287568,
	1.20624719035817,
	1.22080527842164,
	1.23610023069989,
	1.25216960150919,
	1.26905398736196,
	1.28679731954458,
	1.30544719209465,
	1.32505523022013,
	1.34567750504505,
	1.36737500157145,
	1.39021414794172,
	1.41426741553005,
	1.43961400112117,
	1.46634060453179,
	1.49454231757694,
	1.52432364338688,
	1.55579966888401,
	1.58909741791146,
	1.62435741829468,
	1.66173552331931,
	1.70140503710806,
	1.74355920469978,
	1.78841414194345,
	1.83621229854950,
	1.88722657098226,
	1.94176521201939,
	2.00017772298141,
};

// Some HGT files contain 1201 x 1201 points (3 arc/90m resolution)
#define HGT_DATA_WIDTH_3		1201ul

// Some HGT files contain 3601 x 3601 points (1 arc/30m resolution)
#define HGT_DATA_WIDTH_1		3601ul

// Some HGT files contain 3601 x 1801 points (1 arc/30m resolution)
#define HGT_DATA_WIDTH_1_HALF	1801ul

#define GPS_COORD_MUL 10000000
#define GNSS_MUL	GPS_COORD_MUL

#define POS_FLAG_NOT_FOUND	0b00000001
#define POS_INVALID	0x00, -128, -32768

#define	AGL_INVALID -32768

typedef struct
{
	int16_t y_min;
	int16_t y_max;
	float x_val;
	float slope;
} polygon_edge_t;

typedef struct
{
    uint8_t id;
    uint8_t version;
    uint8_t grid_w;
    uint8_t grid_h;
    uint32_t timestamp;
    uint32_t number_of_features;
} map_header_t;

typedef struct
{
    uint32_t index_addr;
    uint32_t feature_cnt;
} map_info_entry_t;

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

				if (edge_value > tmp_value || edge_cnt == j)
				{
					polygon_edge_t move;
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

//	for (uint16_t i = 0; i < edge_cnt; i++)
//	{
//		INFO("%u: %d %d %0.1f %0.3f", i, edges[i].y_min, edges[i].y_max, edges[i].x_val, edges[i].slope);
//	}

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
	            osSemaphoreAcquire(gui.lock, WAIT_INF);
                lv_canvas_draw_line(canvas, line_points, 2, draw_desc);
	            osSemaphoreRelease(gui.lock);
			}

			edges[active[i]].x_val += edges[active[i]].slope;
		}
	}

	free(edges);
}

#define MAP_SHADE_MAG   64

void draw_topo(int32_t lon1, int32_t lat1, int32_t lat, int32_t step_x, int32_t step_y, lv_obj_t * canvas)
{
    static int32_t buffer_lon1 = 0;
    static int32_t buffer_lat1 = 0;
    static int32_t buffer_offset_x = 0;
    static int32_t buffer_offset_y = 0;
    static int16_t * buffer = NULL;

    uint32_t get_index(uint16_t x, uint16_t y)
    {
        uint16_t array_y = (y + buffer_offset_y + MAP_H) % MAP_H;
        uint16_t array_x = (x + buffer_offset_x + MAP_W) % MAP_W;
        return array_y * MAP_W + array_x;
    }

    if (buffer == NULL)
    {
        buffer = (int16_t *)ps_malloc(MAP_W * MAP_H * sizeof(int16_t));
    }

    //calc offset
    int16_t offset_x = (buffer_lon1 - lon1) / step_x;
    int16_t offset_y = (lat1 - buffer_lat1) / step_y;

    INFO("Offset %dx%d", offset_x, offset_y);

    buffer_lon1 = lon1;
    buffer_lat1 = lat1;

    uint16_t x1 = 0;
    uint16_t x2 = MAP_W;
    uint16_t y1 = 0;
    uint16_t y2 = MAP_H;

    //TODO check steps for redraw

    if (abs(offset_x) > MAP_W || abs(offset_y) > MAP_H)
    {
        //redraw all
        offset_x = 0;
        offset_y = 0;
        buffer_offset_x = 0;
        buffer_offset_y = 0;
        x1 = MAP_W;
        y2 = MAP_H;
    }
    else
    {
        if (offset_x >= 0)
            x1 += offset_x;
        else
            x2 += offset_x;

        if (offset_y >= 0)
            y1 += offset_y;
        else
            y2 += offset_y;
    }

    buffer_offset_x = (buffer_offset_x - offset_x + MAP_W) % MAP_W;
    buffer_offset_y = (buffer_offset_y - offset_y + MAP_H) % MAP_H;

    int16_t val_min = INT16_MAX;
    int16_t val_max = INT16_MIN;


    for (uint16_t y = 0; y < MAP_H; y++)
    {
        uint16_t array_y = (y + buffer_offset_y + MAP_H) % MAP_H;
        for (uint16_t x = 0; x < MAP_W; x++)
        {
            uint16_t array_x = (x + buffer_offset_x + MAP_W) % MAP_W;
            uint32_t index = array_y * MAP_W + array_x;

            if (x < x1 || x >= x2 || y < y1 || y >= y2)
            {
                buffer[index] = agl_get_alt(lat1 - step_y * y, lon1 + step_x * x, true);
            }

            if (buffer[index] > val_max)
                val_max = buffer[index];
            if (buffer[index] < val_min)
                val_min = buffer[index];
        }
    }

    int16_t delta = val_max - val_min;
    if (delta == 0)
        delta = 1;

    #define PALETE_SIZE 121
    lv_color_t palete[PALETE_SIZE];
    for (uint8_t i = 0; i < PALETE_SIZE; i++)
    {
        palete[i] = lv_color_hsv_to_rgb(120-i, 100, 100);
    }

    lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);
    uint16_t * image_buff = (uint16_t *)ext->dsc.data;

    int16_t step_x_m = step_x * 111000 / GPS_COORD_MUL / lat_mult[lat / GPS_COORD_MUL];
    int16_t step_y_m = step_y * 111000 / GPS_COORD_MUL;

    for (uint16_t y = 0; y < MAP_H; y++)
    {
        uint16_t array_y = (y + buffer_offset_y + MAP_H) % MAP_H;
        for (uint16_t x = 0; x < MAP_W; x++)
        {
            uint16_t array_x = (x + buffer_offset_x + MAP_W) % MAP_W;
            uint32_t index = array_y * MAP_W + array_x;

            int16_t val = buffer[index];

            uint8_t ci = ((val - val_min) * PALETE_SIZE) / delta;

            uint32_t img_index = y * MAP_W + x;
            lv_color_t color =  palete[ci];

            //apply shade
            if (x > 0 && y > 0 && x < MAP_W - 1 && y < MAP_H - 1)
            {
                int16_t val_left = buffer[get_index(x - 1, y)];
                int16_t val_right = buffer[get_index(x + 1, y)];
                int16_t val_top = buffer[get_index(x, y - 1)];
                int16_t val_bottom = buffer[get_index(x, y + 1)];

                int32_t delta_hor = ((val - val_left) + (val_right - val)) / 2;
                int32_t delta_ver = ((val - val_top) + (val_bottom - val)) / 2;


                int16_t ilum = 0;
                ilum += (delta_hor * MAP_SHADE_MAG) / (step_x_m);
                ilum += (delta_ver * MAP_SHADE_MAG) / (step_y_m);

                //clamp
                ilum = min(ilum, LV_OPA_COVER);
                ilum = max(ilum, -LV_OPA_COVER);

                if (ilum > 0)
                    color = lv_color_lighten(color, ilum);
                else
                    color = lv_color_darken(color, -ilum);
            }
            else
            {
                color = LV_COLOR_BLACK;
            }

            image_buff[img_index] = color.full;
        }

    }
}
#define MAP_DIV_CONST	80000

void create_tile(uint32_t lon, uint32_t lat, uint8_t zoom, lv_obj_t * canvas, bool skip_topo)
{
	uint16_t w = lv_obj_get_width(canvas);
	uint16_t h = lv_obj_get_height(canvas);

//	lv_canvas_fill_bg(canvas, LV_COLOR_RED, LV_OPA_COVER);

	//biggest zoom 1px = ~11,1m
	zoom += 1;
	uint32_t step_x = (zoom * GPS_COORD_MUL) / MAP_DIV_CONST;
	uint32_t step_y = (zoom * GPS_COORD_MUL / lat_mult[lat / GPS_COORD_MUL]) / MAP_DIV_CONST;

	//get bbox
	uint32_t map_w = w * step_x;
	uint32_t map_h = h * step_y;
	int32_t lon1 = lon - map_w / 2;
	int32_t lon2 = lon + map_w / 2;
	int32_t lat1 = lat + map_h / 2;
	int32_t lat2 = lat - map_h / 2;

	if (skip_topo)
	    lv_canvas_fill_bg(canvas, LV_COLOR_GREEN, LV_OPA_COVER);
	else
	    draw_topo(lon1, lat1, lat, step_x, step_y, canvas);


	int32_t flon = 13 * GPS_COORD_MUL;
	int32_t flat = 46 * GPS_COORD_MUL;

	FIL map_data;

	if (f_open(&map_data, "/DATA/MAPS/N46E013.MAP", FA_READ) != FR_OK)
	{
	    ERR("map file not found");
	    return;
	}

	map_header_t mh;

	UINT br;
	f_read(&map_data, &mh, sizeof(map_header_t), &br);

	INFO("file grid is %u x %u", mh.grid_w, mh.grid_h);

	uint32_t grid_start_addr = sizeof(map_header_t);

	int32_t glon1;
	int32_t glat1;
	int32_t glon2;
	int32_t glat2;

	int32_t gstep_x = GPS_COORD_MUL / mh.grid_w;
	int32_t gstep_y = GPS_COORD_MUL / mh.grid_h;

//	INFO("mapshaper -rectangle name=disp bbox=%f,%f,%f,%f \\", lon1 / (float)GPS_COORD_MUL, lat1 / (float)GPS_COORD_MUL, lon2 / (float)GPS_COORD_MUL, lat2 / (float)GPS_COORD_MUL);

	ll_item_t * start = NULL;
	ll_item_t * end = NULL;

	for (uint8_t y = 0; y < mh.grid_h; y++)
	{
		glat1 = flat + gstep_y * y;
		glat2 = glat1 + gstep_y;

		if ((glat1 <= lat1 && lat1 <= glat2) || (glat1 <= lat2 && lat2 <= glat2) || (lat1 >= glat1 && glat2 >= lat2))
		{
			for (uint8_t x = 0; x < mh.grid_w; x++)
			{
				glon1 = flon + gstep_x * x;
				glon2 = glon1 + gstep_x;

				if ((glon1 <= lon1 && lon1 <= glon2) || (glon1 <= lon2 && lon2 <= glon2) || (lon1 <= glon1 && glon2 <= lon2))
				{
//					INFO("-rectangle bbox=%f,%f,%f,%f \\", glon1 / (float)GPS_COORD_MUL, glat1 / (float)GPS_COORD_MUL, glon2 / (float)GPS_COORD_MUL, glat2 / (float)GPS_COORD_MUL);

				    //TODO: select only boxes in grid
				    uint32_t grid_addr = grid_start_addr + (y * mh.grid_h + x) * 8;
				    f_lseek(&map_data, grid_addr);

					map_info_entry_t in;
					f_read(&map_data, &in, sizeof(map_info_entry_t), &br);

					//INFO("grid %u x %u: %u", x, y, feature_cnt);

					for (uint16_t i = 0; i < in.feature_cnt; i++)
					{
						f_lseek(&map_data, in.index_addr + 4 * i);
						uint32_t feature_addr;
						f_read(&map_data, &feature_addr, sizeof(uint32_t), &br);

						list_add_sorted_unique(feature_addr, &start, &end);
					}
				}
			}
		}
	}
//	INFO("-o test.json format=geojson combine-layers");

	lv_draw_rect_dsc_t poly_draw;
	lv_draw_rect_dsc_init(&poly_draw);

	lv_draw_line_dsc_t line_draw;
	lv_draw_line_dsc_init(&line_draw);

	ll_item_t * actual = start;
//	list_dbg(start);

	while(actual != NULL)
	{
		f_lseek(&map_data, actual->feature_addr);
		actual = actual->next;

		uint8_t type;
		f_read(&map_data, &type, sizeof(uint8_t), &br);

		if (type / 100 == 1) //lines
		{
			switch (type)
			{
			//road
			case(100):
				line_draw.width = 2;
				line_draw.color = LV_COLOR_BLACK;
				break;
			case(101):
				line_draw.width = 1;
				line_draw.color = LV_COLOR_BLACK;
				break;
			case(102):
				line_draw.width = 1;
				line_draw.color = LV_COLOR_GRAY;
				break;

			//rail
			case(110):
				line_draw.width = 1;
				line_draw.color = LV_COLOR_ORANGE;
				break;

			//river
			case(120):
				line_draw.width = 2;
				line_draw.color = LV_COLOR_BLUE;
				break;

			//power
			default:
				line_draw.width = 1;
				line_draw.color = LV_COLOR_RED;
				break;

			}

			uint16_t number_of_points;
			f_read(&map_data, &number_of_points, sizeof(uint16_t), &br);

			lv_point_t * points = (lv_point_t *) malloc(sizeof(lv_point_t) * number_of_points);
			for (uint16_t j = 0; j < number_of_points; j++)
			{
				int32_t plon, plat;

                f_read(&map_data, &plon, sizeof(int32_t), &br);
                f_read(&map_data, &plat, sizeof(int32_t), &br);

				int64_t px = (int64_t)(plon - lon1) / step_x;
				int64_t py = (int64_t)(lat1 - plat) / step_y;

                if (px > INT16_MAX) px = INT16_MAX;
                if (px < INT16_MIN) px = INT16_MIN;
                if (py > INT16_MAX) py = INT16_MAX;
                if (py < INT16_MIN) py = INT16_MIN;

				points[j].x = px;
				points[j].y = py;
			}

	        osSemaphoreAcquire(gui.lock, WAIT_INF);
			lv_canvas_draw_line(canvas, points, number_of_points, &line_draw);
	        osSemaphoreRelease(gui.lock);

			free(points);
		}

		if (type == 200 || type == 201) //water or resident
		{
			uint16_t number_of_points;
			f_read(&map_data, &number_of_points, sizeof(uint16_t), &br);

			line_draw.width = 1;
			if (type == 200)
			{
				//water
				line_draw.color = LV_COLOR_BLUE;
				line_draw.opa = LV_OPA_COVER;
			}
			else
			{
				//resident
				line_draw.color = LV_COLOR_WHITE;
				line_draw.opa = LV_OPA_80;
			}

			lv_point_t * points = (lv_point_t *) malloc(sizeof(lv_point_t) * number_of_points);
			for (uint16_t j = 0; j < number_of_points; j++)
			{
				int32_t plon, plat;

                f_read(&map_data, &plon, sizeof(int32_t), &br);
                f_read(&map_data, &plat, sizeof(int32_t), &br);

				int16_t px = ((int64_t)plon - (int64_t)lon1) / step_x;
				int16_t py = ((int64_t)lat1 - (int64_t)plat) / step_y;

				//multipolygon separator
				if (plat == 0x7FFFFFFF && plon == 0x7FFFFFFF)
				{
					px = 0x7FFF;
					py = 0x7FFF;
				}

				//INFO("points.append([%d, %d])", px, py);

				points[j].x = px;
				points[j].y = py;
			}

			draw_polygon(canvas, points, number_of_points, &line_draw);

			free(points);
		}
	}
	list_free(start, end);
}
