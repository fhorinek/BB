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

#define GFX_STATUS_UPDATE	    0	//3
#define GFX_STATUS_SUCCESS	    1  	//1
#define GFX_STATUS_ERROR        2   //2
#define GFX_STATUS_TORCH        3   //5

#define GFX_STATUS_ANIMATED     10
#define GFX_STATUS_CHARGE_NONE  10   //4 _
#define GFX_STATUS_CHARGE_PASS  11   //4 4
#define GFX_STATUS_NONE_BOOST   12   //_ 4
#define GFX_STATUS_CHARGE_DATA  13   //4 0
#define GFX_STATUS_NONE_DATA    14   //_ 40
#define GFX_STATUS_NONE_CHARGE  15   //0 4
#define GFX_STATUS_NONE_NONE    16   //_ _

void gfx_clear();
void gfx_draw_status(uint8_t status, const char * sub_text);
void gfx_draw_progress(float val);

void gfx_draw_dot(int16_t x, int16_t y);
bool gfx_draw_anim();

void gfx_anim_init();
bool gfx_anim_step(uint8_t gfx_status);
void gfx_anim_wait();

#define GFX_ANIM_TOP       265
#define GFX_ANIM_BOTTOM    400

#define GFX_PROGRESS_TOP    380

#endif /* GFX_GFX_H_ */
