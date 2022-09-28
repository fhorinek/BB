/*
 * map.h
 *
 *  Created on: Mar 4, 2021
 *      Author: horinek
 */

#ifndef GUI_MAP_MAP_H_
#define GUI_MAP_MAP_H_

#include "gui/gui.h"

void map_set_static_pos(int32_t latitude, int32_t longitude, uint8_t zoom);
void map_set_automatic_pos();

void thread_map_start(void *argument);

#endif /* GUI_MAP_MAP_H_ */
