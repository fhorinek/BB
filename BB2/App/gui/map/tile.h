/*
 * tile.h
 *
 *  Created on: 24. 7. 2020
 *      Author: horinek
 */

#ifndef GUI_MAP_TILE_H_
#define GUI_MAP_TILE_H_

#include "../../common.h"

#define MAP_TILE_SIZE	240

void tile_create(uint16_t * buffer, uint8_t w, int32_t lat, int32_t lon, int16_t angle, float zoom);

#endif /* GUI_MAP_TILE_H_ */
