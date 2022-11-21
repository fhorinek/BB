/*
 * airspace.h
 *
 *  Created on: 27. 9. 2022
 *      Author: horinek
 */

#ifndef FC_AIRSPACES_AIRSPACE_H_
#define FC_AIRSPACES_AIRSPACE_H_

#include "common.h"
#include "lvgl/lvgl.h"

typedef enum
{
    ac_restricted,
    ac_danger,
    ac_prohibited,
    ac_class_A,
    ac_class_B,
    ac_class_C,
    ac_class_D,
    ac_class_E,
    ac_class_F,
    ac_class_G,
    ac_glider_prohibited,
    ac_ctr,
    ac_tmz,
    ac_rmz,
    ac_wave_window,
    ac_undefined,
    ac_hidden,
} airspace_class_t;

typedef struct
{
    int32_t latitude;
    int32_t longitude;
} gnss_pos_t;

typedef struct
{
    int32_t latitude1;
    int32_t longitude1;
    int32_t latitude2;
    int32_t longitude2;
} gnss_bbox_t;

#define AIRSPACE_NAME_LEN  64

#define BRUSH_TRANSPARENT_FLAG	0b10000000
#define PEN_WIDTH_MASK			0b01111111

typedef union
{
    uint32_t len;
    char * ptr;
} char_len_u;

typedef union
{
    uint32_t pos;
    gnss_pos_t * ptr;
} point_pos_u;

typedef struct __airspace_record_t
{
    char_len_u name;

    uint16_t floor;
    uint16_t ceil;

    bool floor_gnd; //above GND or MSL
    bool ceil_gnd; //above GND or MSL
    airspace_class_t airspace_class;
    uint8_t pen_width;

    lv_color_t pen;
    lv_color_t brush;

    point_pos_u points;
    uint32_t number_of_points;

    gnss_bbox_t bbox;
} airspace_record_t;

void airspace_create_lock();
void airspace_init_buffer();
bool airspace_load(char * name, bool use_dialog);
void airspace_free(airspace_record_t * as);
void airspace_reload_parallel_task();

void airspace_unload();

#endif /* FC_AIRSPACES_AIRSPACE_H_ */
