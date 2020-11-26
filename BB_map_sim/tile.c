/*
 * tile.c
 *
 *  Created on: 10. 11. 2020
 *      Author: horinek
 */

#include "tile.h"
#include "linked_list.h"

#include <stdio.h>

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

#define INFO(...)	do{ printf(__VA_ARGS__); printf("\n"); } while(0);
#define WARN(...)	do{ printf(__VA_ARGS__); printf("\n"); } while(0);
#define DGB(...)	do{ printf(__VA_ARGS__); printf("\n"); } while(0);
#define ERR(...)	do{ printf(__VA_ARGS__); printf("\n"); } while(0);

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

typedef union
{
    uint32_t uint32;
    int32_t int32;
    uint8_t uint8[4];
} byte4;

typedef union
{
    uint16_t uint16;
    int16_t int16;
    uint8_t uint8[2];
} byte2;

typedef struct
{
	uint8_t flags;
	int8_t lat;
	int16_t lon;
} hagl_pos_t;

hagl_pos_t agl_get_fpos(int32_t lat, int32_t lon)
{
	hagl_pos_t tmp;

	tmp.flags = 0x00;
    tmp.lat = lat / GPS_COORD_MUL;
    tmp.lon = lon / GPS_COORD_MUL;

    return tmp;
}



void agl_get_filename(char * fn, hagl_pos_t pos)
{
    char lat_c, lon_c;

    if (pos.lat >= 0)
    {
        lat_c = 'n';
    }
    else
    {
        lat_c = 's';
        pos.lat = abs(pos.lat) + 1;
    }

    if (pos.lon >= 0)
    {
        lon_c = 'e';
    }
    else
    {
        lon_c = 'w';
        pos.lon = abs(pos.lon) + 1;
    }

    sprintf(fn, "%c%02u%c%03u", lat_c, pos.lat, lon_c, pos.lon);
}


int16_t agl_get_alt(int32_t lat, int32_t lon, bool use_bilinear)
{
    uint16_t num_points_x;
    uint16_t num_points_y;
    int16_t alt;

    #define CACHE_SIZE	4

    static FILE * files_cache[CACHE_SIZE] = {NULL, NULL, NULL, NULL};
    static FILE * files_cache2[CACHE_SIZE] = {NULL, NULL, NULL, NULL};
    static hagl_pos_t files_fpos[CACHE_SIZE] = {POS_INVALID, POS_INVALID, POS_INVALID, POS_INVALID};
    static uint8_t file_index = 0;

    hagl_pos_t fpos = agl_get_fpos(lat, lon);

    uint8_t i = 0;
    while (1)
    {
    	if(files_fpos[file_index].lat == fpos.lat && files_fpos[file_index].lon == fpos.lon)
    	{
    		if (files_fpos[file_index].flags & POS_FLAG_NOT_FOUND)
    			return AGL_INVALID;

    		break;
    	}
    	else
    	{
			i++;
			if (i == CACHE_SIZE)
			{
				char filename[9];
				char path[64];

				files_fpos[file_index] = fpos;

				agl_get_filename(filename, fpos);
				snprintf(path, sizeof(path), "/media/horinek/topo_data/HGT1/step2/%s.hgt", filename);

				if (files_cache[file_index])
					fclose(files_cache[file_index]);

				if (files_cache2[file_index])
					fclose(files_cache2[file_index]);

				INFO("opening file '%s' cache[%u]", path, file_index);
				files_cache[file_index] = fopen(path, "rb");
				if (files_cache[file_index] == NULL)
				{
					files_fpos[file_index].flags |= POS_FLAG_NOT_FOUND;
					WARN("not found!");

					return AGL_INVALID;
				}

//				int16_t vmin, vmax;
//
//				uint32_t t_start = HAL_GetTick();
//				agl_get_file_min_max(path, &vmin, &vmax);
//				DBG("time %lu ms", HAL_GetTick() - t_start);

				if (use_bilinear)
					files_cache2[file_index] = fopen(path, "rb");

				break;
			}
			file_index = (file_index + 1) % CACHE_SIZE;
    	}

    };


    if (lon < 0)
    {
        // we do not care above degree, only minutes are important
        // reverse the value, because file goes in opposite direction.
        lon = (GNSS_MUL - 1) + (lon % GNSS_MUL);   // lon is negative!
    }

    if (lat < 0)
    {
        // we do not care above degree, only minutes are important
        // reverse the value, because file goes in opposite direction.
        lat = (GNSS_MUL - 1) + (lat % GNSS_MUL);   // lat is negative!
    }

    // Check, if we have a 1201x1201 or 3601x3601 tile:
    fseek(files_cache[file_index], 0, SEEK_END);
    switch (ftell(files_cache[file_index]))
    {
        case HGT_DATA_WIDTH_3 * HGT_DATA_WIDTH_3 * 2:
            num_points_x = num_points_y = HGT_DATA_WIDTH_3;
        break;
        case HGT_DATA_WIDTH_1 * HGT_DATA_WIDTH_1 * 2:
            num_points_x = num_points_y = HGT_DATA_WIDTH_1;
        break;
        case HGT_DATA_WIDTH_1 * HGT_DATA_WIDTH_1_HALF * 2:
            num_points_x = HGT_DATA_WIDTH_1_HALF;
            num_points_y = HGT_DATA_WIDTH_1;
        break;
        default:
            return AGL_INVALID;
    }

    // "-2" is, because a file has a overlap of 1 point to the next file.
    uint32_t coord_div_x = GNSS_MUL / (num_points_x - 2);
    uint32_t coord_div_y = GNSS_MUL / (num_points_y - 2);
    uint16_t y = (lat % GNSS_MUL) / coord_div_y;
    uint16_t x = (lon % GNSS_MUL) / coord_div_x;


    uint8_t tmp[4];
    byte2 alt11, alt12, alt21, alt22;

    //seek to position
    uint32_t pos = ((uint32_t) x + num_points_x * (uint32_t) ((num_points_y - y) - 1)) * 2;
//    DBG("agl_get_alt: lat=%ld, lon=%ld; x=%d, y=%d; pos=%ld", lat, lon, x, y, pos);
    fseek(files_cache[file_index], pos, SEEK_SET);
    fread(tmp, sizeof(tmp), 1, files_cache[file_index]);

    //switch big endian to little
    alt11.uint8[0] = tmp[1];
    alt11.uint8[1] = tmp[0];


	if (!use_bilinear)
		return alt11.int16;


    alt21.uint8[0] = tmp[3];
    alt21.uint8[1] = tmp[2];

    //seek to opposite position
    pos -= num_points_x * 2;
    fseek(files_cache2[file_index], pos, SEEK_SET);
    fread(tmp, sizeof(tmp), 1, files_cache2[file_index]);

    //switch big endian to little
    alt12.uint8[0] = tmp[1];
    alt12.uint8[1] = tmp[0];

    alt22.uint8[0] = tmp[3];
    alt22.uint8[1] = tmp[2];

    //get point displacement
    float lat_dr = ((lat % GNSS_MUL) % coord_div_y) / (float)(coord_div_y);
    float lon_dr = ((lon % GNSS_MUL) % coord_div_x) / (float)(coord_div_x);

    //compute height by using bilinear interpolation
    float alt1 = alt11.int16 + (float)(alt12.int16 - alt11.int16) * lat_dr;
    float alt2 = alt21.int16 + (float)(alt22.int16 - alt21.int16) * lat_dr;

    alt = alt1 + (float)(alt2 - alt1) * lon_dr;
    //DEB("alt11=%d, alt21=%d, alt12=%d, alt22=%d, alt=%d\n", alt11.int16, alt21.int16, alt12.int16, alt22.int16, alt);

    return alt;
}


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
				lv_canvas_draw_line(canvas, line_points, 2, draw_desc);

			edges[active[i]].x_val += edges[active[i]].slope;
		}
	}

	free(edges);
	//			lv_canvas_draw_line(canvas, points, number_of_points, &line_draw);
}

#define MAP_DIV_CONST	80000

void pix_to_point(lv_point_t point, int32_t map_lon, int32_t map_lat, uint8_t zoom, int32_t * lon, int32_t * lat, lv_obj_t * canvas)
{
	uint16_t w = lv_obj_get_width(canvas);
	uint16_t h = lv_obj_get_height(canvas);

	zoom += 1;
	uint32_t step_x = (zoom * GPS_COORD_MUL) / MAP_DIV_CONST;
	uint32_t step_y = (zoom * GPS_COORD_MUL / lat_mult[map_lat / GPS_COORD_MUL]) / MAP_DIV_CONST;

	uint32_t map_w = w * step_x;
	uint32_t map_h = h * step_y;
	int32_t lon1 = map_lon - map_w / 2;
	int32_t lat1 = map_lat + map_h / 2;

	*lon = lon1 + point.x * step_x;
	*lat = lat1 - point.y * step_y;

}

void create_tile(uint32_t lon, uint32_t lat, uint8_t zoom, lv_obj_t * canvas)
{
	uint16_t w = lv_obj_get_width(canvas);
	uint16_t h = lv_obj_get_height(canvas);

	lv_canvas_fill_bg(canvas, LV_COLOR_RED, LV_OPA_COVER);

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

	int16_t amin = 10000;
	int16_t amax = -10000;

	int16_t vmin = 100;
	int16_t vmax = 445;

//	for (uint16_t y = 0; y < h; y++)
//	{
//		for (uint16_t x = 0; x < w; x++)
//		{
//			int16_t alt = agl_get_alt(lat1 - step_y * y, lon1 + step_x * x, false);
//			//120 - 0 green - red
//			//100
//
//			if (amin > alt) amin = alt;
//			if (amax < alt) amax = alt;
//
//			float a = 1.0 - ((alt - vmin) / (float)(vmax - vmin));
//			if (a < 0) a = 0;
//			if (a > 1) a = 1;
//			uint8_t h = 120 * a;
//			lv_color_t color = lv_color_hsv_to_rgb(h, 100, 100);
//			lv_canvas_set_px(canvas, x, y, color);
//		}
//	}
	INFO("min %d", amin);
	INFO("max %d", amax);

	int32_t flon = 17 * GPS_COORD_MUL;
	int32_t flat = 48 * GPS_COORD_MUL;

	FILE * map_data = fopen("/media/horinek/topo_data/OSM/data/step4/N48E017.MAP", "rb");

	uint8_t id, version, grid_w, grid_h;
	fread(&id, 1, 1, map_data);
	fread(&version, 1, 1, map_data);
	fread(&grid_w, 1, 1, map_data);
	fread(&grid_h, 1, 1, map_data);

	INFO("file grid is %u x %u", grid_w, grid_h);

	uint32_t number_of_features;
	fseek(map_data, 8, SEEK_SET);
	fread(&number_of_features, 4, 1, map_data);

	uint32_t grid_start_addr = 12;

	int32_t glon1;
	int32_t glat1;
	int32_t glon2;
	int32_t glat2;

	int32_t gstep_x = GPS_COORD_MUL / grid_w;
	int32_t gstep_y = GPS_COORD_MUL / grid_h;

//	INFO("mapshaper -rectangle name=disp bbox=%f,%f,%f,%f \\", lon1 / (float)GPS_COORD_MUL, lat1 / (float)GPS_COORD_MUL, lon2 / (float)GPS_COORD_MUL, lat2 / (float)GPS_COORD_MUL);

	ll_item_t * start = NULL;
	ll_item_t * end = NULL;

	for (uint8_t y = 0; y < grid_h; y++)
	{
		glat1 = flat + gstep_y * y;
		glat2 = glat1 + gstep_y;

		if ((glat1 <= lat1 && lat1 <= glat2) || (glat1 <= lat2 && lat2 <= glat2) || (lat1 >= glat1 && glat2 >= lat2))
		{
			for (uint8_t x = 0; x < grid_w; x++)
			{
				glon1 = flon + gstep_x * x;
				glon2 = glon1 + gstep_x;


				if ((glon1 <= lon1 && lon1 <= glon2) || (glon1 <= lon2 && lon2 <= glon2) || (lon1 <= glon1 && glon2 <= lon2))
				{
//					INFO("-rectangle bbox=%f,%f,%f,%f \\", glon1 / (float)GPS_COORD_MUL, glat1 / (float)GPS_COORD_MUL, glon2 / (float)GPS_COORD_MUL, glat2 / (float)GPS_COORD_MUL);
					uint32_t grid_addr = grid_start_addr + (y * grid_h + x) * 8;
					fseek(map_data, grid_addr, SEEK_SET);
					uint32_t index_addr;
					uint32_t feature_cnt;
					fread(&index_addr, 4, 1, map_data);
					fread(&feature_cnt, 4, 1, map_data);

					//INFO("grid %u x %u: %u", x, y, feature_cnt);

					for (uint16_t i = 0; i < feature_cnt; i++)
					{
						fseek(map_data, index_addr + 4 * i, SEEK_SET);
						uint32_t feature_addr;
						fread(&feature_addr, 4, 1, map_data);

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
		fseek(map_data, actual->feature_addr, SEEK_SET);
		actual = actual->next;

		uint8_t type;
		fread(&type, 1, 1, map_data);

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
			fread(&number_of_points, 2, 1, map_data);

			lv_point_t * points = (lv_point_t *) malloc(sizeof(lv_point_t) * number_of_points);
			for (uint16_t j = 0; j < number_of_points; j++)
			{
				int32_t plon, plat;

				fread(&plon, 4, 1, map_data);
				fread(&plat, 4, 1, map_data);

				int64_t px = (int64_t)(plon - lon1) / step_x;
				int64_t py = (int64_t)(lat1 - plat) / step_y;

				if (px > 32767) px = 32767;
				if (px < -32768) px = -32768;
				if (py > 32767) py = 32767;
				if (py < -32768) py = -32768;

				points[j].x = px;
				points[j].y = py;
			}

			lv_canvas_draw_line(canvas, points, number_of_points, &line_draw);
			free(points);
		}

		if (type == 200 || type == 201) //water or resident
		{
			uint16_t number_of_points;
			fread(&number_of_points, 2, 1, map_data);

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

			lv_point_t * points = (uint16_t *) malloc(sizeof(lv_point_t) * number_of_points);
			for (uint16_t j = 0; j < number_of_points; j++)
			{
				int32_t plon, plat;

				fread(&plon, 4, 1, map_data);
				fread(&plat, 4, 1, map_data);


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
