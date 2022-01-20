/*
 * dbg_overlay.h
 *
 *  Created on: 23. 11. 2021
 *      Author: horinek
 */

#ifndef GUI_DBG_OVERLAY_H_
#define GUI_DBG_OVERLAY_H_

#include "common.h"

void dbg_overlay_init();
void dbg_overlay_step();

void dbg_overlay_tasks_create();
void dbg_overlay_tasks_remove();
void dbg_overlay_tasks_update(uint8_t * packet);

#endif /* GUI_DBG_OVERLAY_H_ */
