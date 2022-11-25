/*
 * geo_calc.h
 *
 *  Created on: May 29, 2020
 *      Author: horinek
 */

#ifndef ETC_GEO_CALC_H_
#define ETC_GEO_CALC_H_

#include "../common.h"

#define MAP_DIV_CONST   53280
//#define MAP_DIV_CONST   50000

//single tile 3500
//sum 30sec
#define MAP_W       128
#define MAP_H       128
#define MAP_ROWS    5
#define MAP_COLS    3

//single tile 1889
//sum 2:15min
//#define MAP_W       64
//#define MAP_H       64
//#define MAP_ROWS    8
//#define MAP_COLS    6


#define MAP_CHUNKS  (MAP_ROWS * MAP_COLS)

void align_to_cache_grid(int32_t lon, int32_t lat, uint16_t zoom, int32_t * c_lon, int32_t * c_lat);

void geo_get_steps(int32_t lat, uint16_t zoom, int32_t * step_x, int32_t * step_y);

void geo_to_pix(int32_t lon, int32_t lat, uint8_t zoom, int32_t g_lon, int32_t g_lat, int16_t * x, int16_t * y);
void geo_to_pix_w_h(int32_t lon, int32_t lat, uint8_t zoom, int32_t g_lon, int32_t g_lat, int16_t * x, int16_t * y, int16_t w, int16_t h);
void geo_get_topo_steps(int32_t lat, int32_t step_x, int32_t step_y, int16_t * step_x_m, int16_t * step_y_m);

void geo_destination_f(float lat1, float lon1, float angle, float distance_km, float * lat2, float * lon2);
void geo_destination(int32_t lat1, int32_t lon1, float angle, float distance_km, int32_t * lat2, int32_t * lon2);

uint32_t geo_distance(int32_t lat1, int32_t lon1, int32_t lat2, int32_t lon2, bool FAI, int16_t * bearing);

#endif /* ETC_GEO_CALC_H_ */
