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

uint8_t tile_find_inside(int32_t lon, int32_t lat, uint16_t zoom);
void geo_get_steps(int32_t lat, uint16_t zoom, int32_t * step_x, int32_t * step_y);

void tile_geo_to_pix(uint8_t index, int32_t lon, int32_t lat, int16_t * x, int16_t * y);
void tile_get_cache(int32_t lon, int32_t lat, uint16_t zoom, int32_t * c_lon, int32_t * c_lat, char * path);
//void tile_align_to_cache_grid(int32_t lon, int32_t lat, uint16_t zoom, int32_t * c_lon, int32_t * c_lat);
void tile_align_to_cache_grid(int32_t lon, int32_t lat, uint16_t zoom, int32_t * c_lon, int32_t * c_lat, int32_t * col_div, int32_t * row_div);

bool tile_load_cache(uint8_t index, int32_t lon, int32_t lat, uint16_t zoom);
bool tile_generate(uint8_t index, int32_t lon, int32_t lat, uint16_t zoom);
bool tile_airspace(uint8_t index, int32_t lon, int32_t lat, uint16_t zoom);

void tile_unload_pois(uint8_t index);

#endif /* TILE_H_ */
