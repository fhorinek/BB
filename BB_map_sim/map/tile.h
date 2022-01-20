/*
 * tile.h
 *
 *  Created on: 10. 11. 2020
 *      Author: horinek
 */

#ifndef TILE_H_
#define TILE_H_

#include <stdint.h>

#include "lvgl/lvgl.h"


void tile_create(uint8_t index, int32_t lon, int32_t lat, uint8_t zoom,  bool skip_topo);
void tile_geo_to_pix(uint8_t index, int32_t lon, int32_t lat, int16_t * x, int16_t * y);
void tile_get_cache(int32_t lon, int32_t lat, uint8_t zoom, int32_t * c_lon, int32_t * c_lat, char * path);
void tile_align_to_cache_grid(int32_t lon, int32_t lat, uint8_t zoom, int32_t * c_lon, int32_t * c_lat);
bool tile_load_cache(uint8_t index, int32_t lon, int32_t lat, uint8_t zoom);
uint8_t tile_find_inside(int32_t lon, int32_t lat, uint8_t zoom);
void geo_get_steps(int32_t lat, uint8_t zoom, int32_t * step_x, int32_t * step_y);
bool tile_generate(uint8_t index, int32_t lon, int32_t lat, uint8_t zoom);

bool draw_map(int32_t lon, int32_t lat, int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2, int32_t step_x, int32_t step_y);

#endif /* TILE_H_ */