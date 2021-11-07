/*
 * ctx.h
 *
 *  Created on: Jul 13, 2021
 *      Author: horinek
 */

#ifndef GUI_CTX_H_
#define GUI_CTX_H_

#include "gui.h"

void ctx_close();
void ctx_open(uint8_t index);
void ctx_init();

void ctx_set_cb(gui_ctx_cb_t cb);
void ctx_clear();
void ctx_add_option(char * option);
void ctx_show();
void ctx_hide();

#define CTX_CANCEL	0xFF

#endif /* GUI_CTX_H_ */
