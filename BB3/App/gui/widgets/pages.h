/*
 * pages.h
 *
 *  Created on: Aug 27, 2020
 *      Author: horinek
 */

#ifndef GUI_WIDGETS_PAGES_H_
#define GUI_WIDGETS_PAGES_H_

#include "../../common.h"

uint8_t pages_get_count();
void pages_defragment();
char * pages_get_name(uint8_t index);

bool page_rename(char * old_name, char * new_name);
bool page_create(char * new_name);
void page_delete(char * name);

#endif /* GUI_WIDGETS_PAGES_H_ */
