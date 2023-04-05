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

/* If you change this, then please look at airspace.c/airspace_class_names! */
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
} airspace_class_t;

extern lv_color_t airspace_class_brushes[];

typedef struct
{
    int32_t latitude;
    int32_t longitude;
} gnss_pos_t;

typedef struct
{
    int32_t latitude1;     // max
    int32_t longitude1;    // min
    int32_t latitude2;     // min
    int32_t longitude2;    // max
} gnss_bbox_t;

#define AIRSPACE_NAME_LEN  64

#define BRUSH_TRANSPARENT_FLAG	0b10000000
#define PEN_WIDTH_MASK			0b01111111

typedef union
{
    uint32_t len;
    char *ptr;
} char_len_u;

typedef union
{
    uint32_t pos;
    gnss_pos_t *ptr;
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

// This is the maximum number of near airspaces, that we can handle.
#define AIRSPACE_NEAR_DISTANCE_NUM 10

/**
 * This describes a single "near" airspaces.
 */
typedef struct airspace_near
{
    airspace_record_t *as;       // the corresponding airspace
    int32_t distances[AIRSPACE_NEAR_DISTANCE_NUM]; // the distance in flight direction in cm
    bool inside;              // is the pilot inside this airspace?
// Here we can also compute and store the exit path to leave the airspace, if inside
} airspace_near_t;

/**
 * This is a pmalloc'ed array of airspace_near_t's.
 */
typedef struct airspaces_near
{
    airspace_near_t * asn;   // pmalloc'ed to hold all near airspaces
    int size;                  // number of entries pmalloc'ed.
    int num;                   // number of entries being used
    bool valid;                // Are the entries valid?
    uint32_t last_updated;  // last HAL_GetTick where the airspaces_near changed
    int16_t used_heading;     // The heading, that we used to compute everything
    gnss_pos_t used_pilot_pos;
} airspaces_near_t;

char* airspace_class_name(airspace_class_t class);
lv_color_t airspace_class_brush(airspace_class_t class);

void airspace_create_lock();
void airspace_init_buffer();

bool airspace_load(char *name, bool use_dialog);
void airspace_unload();
void airspace_clear_cache(char *name);

void airspace_load_parallel();
void airspace_step();

//maximum number of points in one airspace
#define AIRSPACE_MAX_POINTS     (1024 * 4)
//maximum name for airspace
#define AIRSPACE_MAX_NAME_LEN   128
//cache version, increment when changing format or handling
#define AIRSPACE_CACHE_VERSION  18

//maximum number of airspaces loaded in any moment
#define AIRSPACE_INDEX_ALLOC    (512)
//maximum memory to allocated for airspace points
#define AIRSPACE_DATA_ALLOC     (512 * 1024)

#endif /* FC_AIRSPACES_AIRSPACE_H_ */
