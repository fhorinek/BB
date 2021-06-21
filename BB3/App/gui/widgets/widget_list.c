/*
 * widget_list.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "widgets.h"

DECLARE_WIDGET(Vario);
DECLARE_WIDGET(Avg);
DECLARE_WIDGET(Bar);
DECLARE_WIDGET(FTime);
DECLARE_WIDGET(Alt1);
DECLARE_WIDGET(CompArrow);
DECLARE_WIDGET(Map);
DECLARE_WIDGET(GSpeed);
DECLARE_WIDGET(GAlt);
DECLARE_WIDGET(GHdg);

widget_t * widgets[] =
{
    LIST_WIDGET(Vario),
    LIST_WIDGET(Avg),
    LIST_WIDGET(Bar),
    LIST_WIDGET(Alt1),
    LIST_WIDGET(FTime),
    LIST_WIDGET(CompArrow),
    LIST_WIDGET(GSpeed),
    LIST_WIDGET(GAlt),
	LIST_WIDGET(GHdg)
//    LIST_WIDGET(Map),
};


uint8_t number_of_widgets()
{
	return sizeof(widgets) / sizeof(widget_t *);
}

