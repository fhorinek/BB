/*
 * page_settings.h
 *
 *  Created on: 14. 5. 2020
 *      Author: horinek
 */

#ifndef GUI_WIDGET_LIST_H_
#define GUI_WIDGET_LIST_H_

#include "gui/gui.h"
#include "gui/widgets/widgets.h"

DECLARE_TASK(widget_list);

void widget_list_set_page_name(char * name, uint8_t page_index);
void widget_list_select_widget(widget_t * w, uint8_t widget_index);


#endif /* GUI_WIDGET_LIST_H_ */
