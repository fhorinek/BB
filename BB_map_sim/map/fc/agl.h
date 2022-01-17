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

#endif /* GUI_MAP_AGL_H_ */
