/*
 * settings.h
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */

#ifndef GUI_PAGE_EDIT_H_
#define GUI_PAGE_EDIT_H_

#include "gui/gui.h"
#include "gui/widgets/widgets.h"

DECLARE_TASK(page_edit);

void page_edit_set_page_name(char * filename, uint8_t index);
void page_edit_modify_widget(widget_t * w, uint8_t widget_index);

#endif /* GUI_PAGE_EDIT_H_ */
