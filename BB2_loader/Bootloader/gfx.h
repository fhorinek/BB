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
#define GFX_STATUS_CHARGE 	0	//4
#define GFX_STATUS_USB 		1	//0
#define GFX_STATUS_UPDATE	2	//3
#define GFX_STATUS_SUCCESS	3	//1
#define GFX_STATUS_ERROR    4   //2
#define GFX_STATUS_TORCH    5   //5

void gfx_clear();
void gfx_draw_status(uint8_t status, const char * sub_text);
void gfx_draw_progress(float val);

#endif /* GFX_GFX_H_ */
