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


#define GPS_COORD_MUL 10000000

void create_tile(uint32_t lon, uint32_t lat, uint8_t zoom, lv_obj_t * canvas,  bool skip_topo);
void pix_to_point(lv_point_t point, int32_t map_lon, int32_t map_lat, uint8_t zoom, int32_t * lon, int32_t * lat, lv_obj_t * canvas);

#endif /* TILE_H_ */
