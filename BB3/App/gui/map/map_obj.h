/*
 * map_obj.h
 *
 *  Created on: 20.09.2022
 *      Author: bubeck
 */

#ifndef GUI_MAP_MAP_OBJ_H_
#define GUI_MAP_MAP_OBJ_H_

#include <stdint.h>

#include "gui/gui_list.h"
#include "fc/fc.h"

typedef struct map_obj_data {
    lv_obj_t * map;
    lv_obj_t * image[9];
    lv_obj_t * dot;
    lv_obj_t * arrow;

    lv_obj_t * poi[NUMBER_OF_POI];
    uint8_t poi_magic[NUMBER_OF_POI];

    lv_point_t offsets[9];
    uint8_t master_tile;

    lv_obj_t * fanet_icons[NB_NUMBER_IN_MEMORY];
    lv_obj_t * fanet_labels[NB_NUMBER_IN_MEMORY];
    uint8_t fanet_obj_count;
    uint8_t fanet_magic;

    uint8_t magic;
} map_obj_data_t;

lv_obj_t * map_obj_init(lv_obj_t * par, map_obj_data_t *map_data);
void map_obj_loop(map_obj_data_t *map_data, int32_t disp_lat, int32_t disp_lon);
void map_obj_glider_loop(map_obj_data_t *map_data);
void map_obj_fanet_loop(map_obj_data_t *map_data, int32_t disp_lat, int32_t disp_lon, uint16_t zoom);
void map_get_master_tile_xy(map_obj_data_t *map_data, lv_point_t *p);
#endif /* GUI_MAP_MAP_OBJ_H_ */
