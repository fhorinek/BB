/*
 * gfx.h
 *
 *  Created on: 18. 6. 2020
 *      Author: horinek
 */

#ifndef GFX_GFX_H_
#define GFX_GFX_H_

#include "common.h"
								//icon
#define GFX_STATUS_CHARGE_NONE  0   //4 _
#define GFX_STATUS_CHARGE_DATA  1   //4 0
#define GFX_STATUS_NONE_DATA    2   //_ 0
#define GFX_STATUS_NONE_CHARGE  3   //0 4
#define GFX_STATUS_UPDATE	    4	//3
#define GFX_STATUS_SUCCESS	    5  	//1
#define GFX_STATUS_ERROR        6   //2
#define GFX_STATUS_TORCH        7   //5

void gfx_clear();
void gfx_draw_status(uint8_t status, const char * sub_text);
void gfx_draw_progress(float val);

#endif /* GFX_GFX_H_ */
