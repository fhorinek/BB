/*
 * widget_list.c
 *
 *  Created on: 11. 8. 2020
 *      Author: horinek
 */

#include "widgets.h"

#include "widget_dummy.h"

widget_t * widgets[] =
{
		&widget_dummy
};

uint8_t number_of_widgets()
{
	return sizeof(widgets) / sizeof(widget_t *);
}

