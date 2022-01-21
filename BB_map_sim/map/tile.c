/*
 * tile.c
 *
 *  Created on: 10. 11. 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG

#include "tile.h"
#include "linked_list.h"
#include "map_thread.h"

#include "fc/fc.h"
#include "fc/agl.h"
#include "etc/geo_calc.h"

#define MAP_BUFFER_SIZE	(MAP_W * MAP_H * sizeof(lv_color_t))


// Some HGT files contain 1201 x 1201 points (3 arc/90m resolution)
#define HGT_DATA_WIDTH_3		1201ul

// Some HGT files contain 3601 x 3601 points (1 arc/30m resolution)
#define HGT_DATA_WIDTH_1		3601ul

// Some HGT files contain 3601 x 1801 points (1 arc/30m resolution)
#define HGT_DATA_WIDTH_1_HALF	1801ul

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
    uint8_t magic;
    uint8_t grid_w;
    uint8_t grid_h;
    int32_t longitude;
    int32_t latitude;
} map_header_t;

typedef struct
{
    uint32_t index_addr;
    uint32_t feature_cnt;
} map_info_entry_t;

#define CACHE_START_WORD	0x55AA
#define CACHE_VERSION		34
#define CACHE_HAVE_AGL		0b10000000
#define CACHE_HAVE_MAP_MASK	0b01111111
#define CACHE_NO_FILE		0

typedef struct
{
	uint16_t start_word;
	uint16_t version;

	uint8_t src_files_magic[4]; //11 12 22 21
} cache_header_t;

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
	            gui_lock_acquire();
                lv_canvas_draw_line(canvas, line_points, 2, draw_desc);
	            gui_lock_release();
			}

			edges[active[i]].x_val += edges[active[i]].slope;
		}
	}

	free(edges);
}

#define MAP_SHADE_MAG   64

void draw_topo(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2, int32_t step_x, int32_t step_y)
{
	//create static dest buffer for tile in psram

	//load 4 agl files to psram and place values to the tile buffer

	//transfer tile buffer to canvas buffer

//
//    #define PALETE_SIZE 121
//    lv_color_t palete[PALETE_SIZE];
//    for (uint8_t i = 0; i < PALETE_SIZE; i++)
//    {
//        palete[i] = lv_color_hsv_to_rgb(120-i, 100, 100);
//    }
//
//    lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);
//    uint16_t * image_buff = (uint16_t *)ext->dsc.data;
//
//    int16_t step_x_m, step_y_m;
//    geo_get_topo_steps(lat, step_x, step_y, &step_x_m, &step_y_m);
//
//    for (uint16_t y = 0; y < MAP_H; y++)
//    {
//        for (uint16_t x = 0; x < MAP_W; x++)
//        {
//        	uint32_t index = get_index(x, y);
//
//            int16_t val = buffer[index];
//
//            uint8_t ci = ((val - val_min) * PALETE_SIZE) / delta;
//
//            uint32_t img_index = y * MAP_W + x;
//            lv_color_t color =  palete[ci];
//
//            //apply shade
//			int16_t val_left = buffer[get_index(x - 1, y)];
//			int16_t val_right = buffer[get_index(x + 1, y)];
//			int16_t val_top = buffer[get_index(x, y - 1)];
//			int16_t val_bottom = buffer[get_index(x, y + 1)];
//
//			int32_t delta_hor = ((val - val_left) + (val_right - val)) / 2;
//			int32_t delta_ver = ((val - val_top) + (val_bottom - val)) / 2;
//
//			int16_t ilum = 0;
//			ilum += (delta_hor * MAP_SHADE_MAG) / (step_x_m);
//			ilum += (delta_ver * MAP_SHADE_MAG) / (step_y_m);
//
//			//clamp
//			ilum = min(ilum, LV_OPA_COVER);
//			ilum = max(ilum, -LV_OPA_COVER);
//
//			if (ilum > 0)
//				color = lv_color_lighten(color, ilum);
//			else
//				color = lv_color_darken(color, -ilum);
//
//            image_buff[img_index] = color.full;
//        }
//
//    }
}

void tile_get_filename(char * fn, int32_t lat, int32_t lon)
{
    char lat_c, lon_c;
    lat = lat / GNSS_MUL;
    lon = lon / GNSS_MUL;

    if (lat >= 0)
    {
        lat_c = 'N';
    }
    else
    {
        lat_c = 'S';
        lat = abs(lat) + 1;
    }

    if (lon >= 0)
    {
        lon_c = 'E';
    }
    else
    {
        lon_c = 'W';
        lon = abs(lon) + 1;
    }

    sprintf(fn, "%c%02lu%c%03lu", lat_c, lat, lon_c, lon);
}


uint8_t tile_find_inside(int32_t lon, int32_t lat, uint8_t zoom)
{
    for (uint8_t i = 0; i < 9; i++)
    {
    	if (!gui.map.chunks[i].ready)
    	{
    		continue;
    	}

    	if (gui.map.chunks[i].zoom != zoom)
    	{
    		continue;
    	}

		int16_t x, y;
		tile_geo_to_pix(i, lon, lat, &x, &y);

		if (x >= 0 && x < MAP_W && y >= 0 && y < MAP_H)
			return i;
    }

    return 0xFF;
}

void tile_geo_to_pix(uint8_t index, int32_t g_lon, int32_t g_lat, int16_t * x, int16_t * y)
{
    int32_t lon = gui.map.chunks[index].center_lon;
    int32_t lat = gui.map.chunks[index].center_lat;
    uint8_t zoom = gui.map.chunks[index].zoom;

    geo_to_pix(lon, lat, zoom, g_lon, g_lat, x, y);
}

void tile_align_to_cache_grid(int32_t lon, int32_t lat, uint8_t zoom, int32_t * c_lon, int32_t * c_lat)
{
	int32_t step_x;
	int32_t step_y;
	geo_get_steps(lat, zoom, &step_x, &step_y);

	//get bbox
	uint32_t map_w = (MAP_W * step_x) / 1;
	uint32_t map_h = (MAP_H * step_y) / 1;

	int64_t steps = geo_get_steps_from_equator(lat, zoom);
	int64_t step_rounded = (steps / MAP_H) * MAP_H;
	int64_t delta = steps - steps_rounded_rounded;
	int64_t rest = delta + ((lat % GNSS_MUL) / (float)GNSS_MUL) * map_h;
	rest = (delta / MAP_H) * MAP_H;
	int64_t dist = rest - delta;

	*c_lat = (lat / GNSS_MUL) * GNSS_MUL + dist * step_y;

	*c_lon = ((lon + (map_w / 2)) / map_w) * map_w;
}

void tile_get_cache(int32_t lon, int32_t lat, uint8_t zoom, int32_t * c_lon, int32_t * c_lat, char * path)
{
	tile_align_to_cache_grid(lon, lat, zoom, c_lon, c_lat);

	sprintf(path, PATH_MAP_CACHE_DIR "/%u/%08lX%08lX", zoom, *c_lon, *c_lat);
}

static uint8_t * load_map_file(int32_t lon, int32_t lat, uint8_t index)
{
	//buffer for map data
	static uint8_t * map_cache = NULL;
	//name of the file in buffer
	static char map_cache_name[16];

	//names used to generate tile
	static char name[4][16];

	tile_get_filename(name[index], lat, lon);

	for (uint8_t i = index; i > 0; i--)
	{
		//was this file allready processed?
		if (strcmp(name[i-1], name[index]) == 0)
			return NULL;
	}



	bool loaded = false;

	if (map_cache != NULL)
	{
		if (strcmp(map_cache_name, name[index]) == 0)
		{
			loaded = true;
		}
	}

	if (!loaded)
	{
		FIL map_data;

		char path[PATH_LEN];
		snprintf(path, sizeof(path), "%s/%s.map", PATH_MAP_DIR, name[index]);
		if (f_open(&map_data, path, FA_READ) != FR_OK)
		{
			ERR("map file %s not found", name[index]);
			db_insert(PATH_MAP_INDEX, name, "W"); //set want flag
			return NULL;
		}

		if (map_cache != NULL)
		{
			ps_free(map_cache);
			map_cache = NULL;
		}

		UINT br;
		map_cache = ps_malloc(f_size(&map_data));

		f_read(&map_data, map_cache, f_size(&map_data), &br);
		f_close(&map_data);

		//mark name to cache
		strcpy(map_cache_name, name);
	}

	//check if this file was already used on this tile
	return map_cache;
}

uint16_t draw_map(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2, int32_t step_x, int32_t step_y, uint8_t * map_cache)
{
	if (map_cache == NULL)
		return 0;

	map_header_t * mh = (map_header_t *)map_cache;

	DBG("file grid is %u x %u", mh->grid_w, mh->grid_h);

	int32_t flon = (mh->longitude / GNSS_MUL) * GNSS_MUL;
	int32_t flat = (mh->latitude / GNSS_MUL) * GNSS_MUL;

	uint32_t grid_start_addr = sizeof(map_header_t);

	//search the grid and locate the features that are visible
	int32_t gstep_x = GNSS_MUL / mh->grid_w;
	int32_t gstep_y = GNSS_MUL / mh->grid_h;

	INFO("rectangle name=disp bbox=%f,%f,%f,%f", lon1 / (float)GNSS_MUL, lat1 / (float)GNSS_MUL, lon2 / (float)GNSS_MUL, lat2 / (float)GNSS_MUL);

	ll_item_t * start = NULL;
	ll_item_t * end = NULL;

	for (uint8_t y = 0; y < mh->grid_h; y++)
	{
		int32_t glat1 = flat + gstep_y * y;
		int32_t glat2 = glat1 + gstep_y;

		if ((glat1 <= lat1 && lat1 <= glat2)
				|| (glat1 <= lat2 && lat2 <= glat2)
				|| (lat1 >= glat1 && glat2 >= lat2))
		{
			for (uint8_t x = 0; x < mh->grid_w; x++)
			{
				int32_t glon1 = flon + gstep_x * x;
				int32_t glon2 = glon1 + gstep_x;

				if ((glon1 <= lon1 && lon1 <= glon2)
						|| (glon1 <= lon2 && lon2 <= glon2)
						|| (lon1 <= glon1 && glon2 <= lon2))
				{
					INFO("rectangle name=x%uy%u bbox=%f,%f,%f,%f", x, y,
							glon1 / (float)GNSS_MUL, glat1 / (float)GNSS_MUL,
							glon2 / (float)GNSS_MUL, glat2 / (float)GNSS_MUL);

				    //load features
				    uint32_t grid_addr = grid_start_addr + (y * mh->grid_h + x) * sizeof(map_info_entry_t);

				    map_info_entry_t * in = (map_info_entry_t * )(map_cache + grid_addr);

					for (uint16_t i = 0; i < in->feature_cnt; i++)
					{
						uint32_t feature_addr = *((uint32_t *)(map_cache + in->index_addr + 4 * i));

						list_add_sorted_unique(feature_addr, &start, &end);
					}
				}
			}
		}
	}

	lv_draw_rect_dsc_t poly_draw;
	lv_draw_rect_dsc_init(&poly_draw);

	lv_draw_line_dsc_t line_draw;
	lv_draw_line_dsc_init(&line_draw);
	lv_draw_line_dsc_t warn_line_draw;
	lv_draw_line_dsc_init(&warn_line_draw);

	warn_line_draw.width = 3;
	warn_line_draw.color = LV_COLOR_RED;
	warn_line_draw.round_end = 1;
	warn_line_draw.round_start = 1;

	lv_draw_label_dsc_t text_draw;
	lv_draw_label_dsc_init(&text_draw);
	text_draw.font = &lv_font_montserrat_12;
	text_draw.color = LV_COLOR_BLACK;

	ll_item_t * actual = start;
//	list_dbg(start);


	line_draw.width = 1;
	line_draw.color = LV_COLOR_MAGENTA;

	lv_point_t * points = (lv_point_t *) malloc(sizeof(lv_point_t) * 5);

	points[0].x = 0;
	points[0].y = 0;
	points[1].x = MAP_W - 1;
	points[1].y = 0;
	points[2].x = MAP_W - 1;
	points[2].y = MAP_H - 1;
	points[3].x = 0;
	points[3].y = MAP_H - 1;
	points[4].x = 0;
	points[4].y = 0;

	lv_canvas_draw_line(gui.map.canvas, points, 5, &line_draw);
	free(points);

	//draw features
	while(actual != NULL)
	{
		uint8_t type = *((uint8_t *)(map_cache + actual->feature_addr));
//		DBG("feature %u type %u", index++, type);

//		if (type <= 13 && type >= 10) //place
//		{
//			uint8_t name_len = *((uint8_t *)(map_cache + actual->feature_addr + 1));
//
//			int32_t plon, plat;
//
//			plon = *((int32_t *)(map_cache + actual->feature_addr + 4));
//			plat = *((int32_t *)(map_cache + actual->feature_addr + 8));
//
//			int64_t px = (int64_t)(plon - lon1) / step_x;
//			int64_t py = (int64_t)(lat1 - plat) / step_y;
//
//			char name[name_len + 1];
//			strncpy(name, map_cache + actual->feature_addr + 12, name_len);
//			name[name_len] = 0;
//
//	        gui_lock_acquire();
//			lv_canvas_draw_text(gui.map.canvas, px, py, 100, &text_draw, name, LV_LABEL_ALIGN_CENTER);
//	        gui_lock_release();
//		}

		bool draw_warning = false;

		if (type / 100 == 1) //lines
		{
			switch (type)
			{
			//road
			case(100):
				line_draw.width = 2;
				line_draw.color = LV_COLOR_MAROON;
				break;
			case(101):
				line_draw.width = 2;
				line_draw.color = LV_COLOR_BLACK;
				break;
			case(102):
				line_draw.width = 1;
				line_draw.color = LV_COLOR_BLACK;
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

			//power or airline
			default:
				line_draw.width = 1;
				line_draw.color = LV_COLOR_BLACK;
				draw_warning = true;
				break;

			}

			uint16_t number_of_points = *((uint16_t *)(map_cache + actual->feature_addr + 2));

			lv_point_t * points = (lv_point_t *) malloc(sizeof(lv_point_t) * number_of_points);
			for (uint16_t j = 0; j < number_of_points; j++)
			{
				int32_t plon, plat;

				plon = *((int32_t *)(map_cache + actual->feature_addr + 4 + 0 + 8 * j));
				plat = *((int32_t *)(map_cache + actual->feature_addr + 4 + 4 + 8 * j));

				int64_t px = (int64_t)(plon - lon1) / step_x;
				int64_t py = (int64_t)(lat1 - plat) / step_y;

                if (px > INT16_MAX) px = INT16_MAX;
                if (px < INT16_MIN) px = INT16_MIN;
                if (py > INT16_MAX) py = INT16_MAX;
                if (py < INT16_MIN) py = INT16_MIN;

				points[j].x = px;
				points[j].y = py;
			}

	        gui_lock_acquire();
			if (draw_warning)
				lv_canvas_draw_line(gui.map.canvas, points, number_of_points, &warn_line_draw);

	        lv_canvas_draw_line(gui.map.canvas, points, number_of_points, &line_draw);
	        gui_lock_release();

			free(points);
		}

		if (type == 200 || type == 201) //water or resident
		{

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
				line_draw.opa = LV_OPA_50;
			}

			uint16_t number_of_points = *((uint16_t *)(map_cache + actual->feature_addr + 2));

			lv_point_t * points = (lv_point_t *) malloc(sizeof(lv_point_t) * number_of_points);
			for (uint16_t j = 0; j < number_of_points; j++)
			{
				int32_t plon, plat;

				plon = *((int32_t *)(map_cache + actual->feature_addr + 4 + 0 + 8 * j));
				plat = *((int32_t *)(map_cache + actual->feature_addr + 4 + 4 + 8 * j));

				int16_t px = ((int64_t)plon - (int64_t)lon1) / step_x;
				int16_t py = ((int64_t)lat1 - (int64_t)plat) / step_y;

				//multipolygon separator
				if (plat == 0x7FFFFFFF && plon == 0x7FFFFFFF)
				{
					px = 0x7FFF;
					py = 0x7FFF;
				}

				points[j].x = px;
				points[j].y = py;
			}

			draw_polygon(gui.map.canvas, points, number_of_points, &line_draw);

			free(points);
		}

		actual = actual->next;
	}
	list_free(start, end);

	return mh->magic & CACHE_HAVE_MAP_MASK;
}

void tile_create(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2, int32_t step_x, int32_t step_y, uint8_t * magic)
{
    lv_canvas_fill_bg(gui.map.canvas, LV_COLOR_GREEN, LV_OPA_COVER);
	//draw_topo(lon1, lat1, lon2, lat2, step_x, step_y, magic);

    //draw map map
    magic[0] |= draw_map(lon1, lat1, lon2, lat2, step_x, step_y, load_map_file(lon1, lat1, 0));
    magic[1] |= draw_map(lon1, lat1, lon2, lat2, step_x, step_y, load_map_file(lon1, lat2, 1));
    magic[2] |= draw_map(lon1, lat1, lon2, lat2, step_x, step_y, load_map_file(lon2, lat2, 2));
    magic[3] |= draw_map(lon1, lat1, lon2, lat2, step_x, step_y, load_map_file(lon2, lat1, 3));
}

//get map source files on device
bool tile_validate_sources(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2, uint8_t * magic)
{
	char name[4][16];
	char path[PATH_LEN];

	tile_get_filename(name[0], lat1, lon1);
	tile_get_filename(name[1], lat1, lon2);
	tile_get_filename(name[2], lat2, lon2);
	tile_get_filename(name[3], lat1, lon2);

	for (uint8_t i = 0; i < 4; i++)
	{
		uint8_t tmp_magic = 0;

		snprintf(path, sizeof(path), "%s/%s.hgt", PATH_MAP_DIR, name[i]);
		if (file_exists(path))
			tmp_magic = CACHE_HAVE_AGL;

		snprintf(path, sizeof(path), "%s/%s.map", PATH_MAP_DIR, name[i]);
		FIL f;
		if (f_open(&f, path, FA_READ) == FR_OK)
		{
			map_header_t mh;
			UINT rb;
			f_read(&f, &mh, sizeof(mh), &rb);
			tmp_magic |= mh.magic & CACHE_HAVE_MAP_MASK;
			f_close(&f);
		}

		if (tmp_magic != magic[i])
			return false;
	}

	return true;
}


bool tile_load_cache(uint8_t index, int32_t lon, int32_t lat, uint8_t zoom)
{
	gui.map.chunks[index].ready = false;
	gui.map.magic++;

    char tile_path[PATH_LEN];
    int32_t c_lon, c_lat;

    bool pass = true;
    tile_get_cache(lon, lat, zoom, &c_lon, &c_lat, tile_path);

	int32_t step_x;
	int32_t step_y;
	geo_get_steps(c_lat, zoom, &step_x, &step_y);

	//get bbox
	uint32_t map_w = MAP_W * step_x;
	uint32_t map_h = (MAP_H * step_y);
	int32_t lon1 = c_lon - map_w / 2;
	int32_t lon2 = c_lon + map_w / 2;
	int32_t lat1 = c_lat + map_h / 2;
	int32_t lat2 = c_lat - map_h / 2;

    if (file_exists(tile_path))
    {
    	DBG("Trying to load from cache");

    	//load from cache
        FIL f;
        UINT br;
        cache_header_t ch;
        FRESULT res;


        res = f_open(&f, tile_path, FA_READ);
        if (res == FR_OK)
        {
			res = f_read(&f, &ch, sizeof(cache_header_t), &br);
			if (res == FR_OK && br == sizeof(cache_header_t))
			{
				if (ch.start_word != CACHE_START_WORD || ch.version != CACHE_VERSION)
				{
					WARN("Cache header invalid version");
					pass = false;
				}

				if (tile_validate_sources(lon1, lat1, lon2, lat2, ch.src_files_magic))
				{
					WARN("Cache was made with different files");
					pass = false;
				}
			}

			if (pass)
			{
				res = f_read(&f, gui.map.chunks[index].buffer, MAP_BUFFER_SIZE, &br);
				if (res != FR_OK || br != MAP_BUFFER_SIZE)
				{
					WARN("Cache body size not valid");
					pass = false;
				}
			}

			f_close(&f);
        }
    }
    else
    {
    	pass = false;
    }

    if (pass)
    {
		gui.map.chunks[index].center_lon = c_lon;
		gui.map.chunks[index].center_lat = c_lat;
		gui.map.chunks[index].zoom = zoom;
		gui.map.chunks[index].ready = true;

		gui.map.magic++;

		return true;
    }

    return false;
}


bool tile_generate(uint8_t index, int32_t lon, int32_t lat, uint8_t zoom)
{
	//invalidate
	gui.map.chunks[index].ready = false;
	gui.map.magic++;

    char tile_path[PATH_LEN];
    int32_t c_lon, c_lat;

    tile_get_cache(lon, lat, zoom, &c_lon, &c_lat, tile_path);

	uint32_t start = HAL_GetTick();
	DBG("Geneating start");

	int32_t step_x;
	int32_t step_y;
	geo_get_steps(c_lat, zoom, &step_x, &step_y);

	//get bbox
	uint32_t map_w = MAP_W * step_x;
	uint32_t map_h = (MAP_H * step_y);
	int32_t lon1 = c_lon - map_w / 2;
	int32_t lon2 = c_lon + map_w / 2;
	int32_t lat1 = c_lat + map_h / 2;
	int32_t lat2 = c_lat - map_h / 2;

	//create tile
	cache_header_t ch = {0};

	ch.start_word = CACHE_START_WORD;
	ch.version = CACHE_VERSION;

    lv_canvas_set_buffer(gui.map.canvas, gui.map.chunks[index].buffer, MAP_W, MAP_H, LV_IMG_CF_TRUE_COLOR);

	tile_create(lon1, lat1, lon2, lat2, step_x, step_y, ch.src_files_magic);

	DBG("Saving tile to storage");

	FIL f;
	UINT bw;

	//create dir
	char dir_path[PATH_LEN];
	sprintf(dir_path, PATH_MAP_CACHE_DIR "/%u", zoom);
	f_mkdir(dir_path);

	f_open(&f, tile_path, FA_WRITE | FA_CREATE_ALWAYS);
	f_write(&f, &ch, sizeof(cache_header_t), &bw);
	f_write(&f, gui.map.chunks[index].buffer, MAP_BUFFER_SIZE, &bw);
	f_close(&f);

	DBG("duration %0.2fs", (HAL_GetTick() - start) / 1000.0);

    gui.map.chunks[index].center_lon = c_lon;
    gui.map.chunks[index].center_lat = c_lat;
    gui.map.chunks[index].zoom = zoom;
    gui.map.chunks[index].ready = true;

    gui.map.magic++;

    return true;
}
