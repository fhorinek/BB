/*
 * agl.h
 *
 *  Created on: Jul 30, 2020
 *      Author: horinek
 */

#ifndef GUI_MAP_AGL_H_
#define GUI_MAP_AGL_H_

#include "../common.h"

#define	AGL_INVALID -32768

void agl_step();

int16_t agl_get_alt(int32_t lat, int32_t lon, bool use_bilinear);

typedef struct
{
    uint8_t flags;
    int8_t lat;
    int16_t lon;
} hagl_pos_t;

#define POS_FLAG_NOT_FOUND  0b00000001
#define POS_FLAG_DUPLICATE  0b00000010
#define POS_FLAG_DONE       0b00000100

#define POS_INVALID 0x00, -128, -32768


hagl_pos_t agl_get_fpos(int32_t lon, int32_t lat);
bool agl_pos_cmp(hagl_pos_t * a, hagl_pos_t * b);
void agl_get_filename(char * fn, hagl_pos_t pos);

#endif /* GUI_MAP_AGL_H_ */
